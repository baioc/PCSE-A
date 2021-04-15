/*
 * shm.c
 *
 *  Created on: 24/03/2021
 *      Author: Thibault Cantori
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "shm.h"

#include "pm.h"
#include "hash.h"
#include "stddef.h"
#include "mem.h"
#include "stdint.h"
#include "debug.h"
#include "string.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

// shared_ptr<struct page>
struct shm_page {
  struct page *shared;
  int          refcount;
  size_t       keylen; // as returned by strlen(key)
  char         key[/* keylen + 1 */];
};

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

// Negative when not found, valid index on success.
static int find_slot(struct shm_page **slots, struct shm_page *ref);

// Maps/unmaps a shared page in a process, returning virtual address.
static uint32_t shmmap(struct proc *proc, int slot, struct page *shared,
                       unsigned flags);

// Decreases a shared page's reference count;
static void unref(struct shm_page *shm);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static hash_t shm_map; // (char *) -> (struct shm_page *)

/*******************************************************************************
 * Public function
 ******************************************************************************/

void shm_init(void)
{
  hash_init_string(&shm_map);
}

void *shm_create(const char *key)
{
  // check if key is valid and unused
  if (key == NULL || hash_isset(&shm_map, (void *)key)) return NULL;

  // look for a free shm page slot
  struct proc *proc = get_current_process();
  const int    slot = find_slot(proc->shm_slots, NULL);
  if (slot < 0) return NULL;

  // allocate an actual memory page to share
  struct page *page = page_alloc();
  if (page == NULL) return NULL;

  // allocate "shared_ptr" and key copy on the heap
  const size_t keylen = strlen(key);
  proc->shm_slots[slot] = mem_alloc(sizeof(struct shm_page) + keylen + 1);
  if (proc->shm_slots[slot] == NULL) {
    page_free(page);
    return NULL;
  }
  proc->shm_slots[slot]->shared = page;
  proc->shm_slots[slot]->refcount = 1;
  proc->shm_slots[slot]->keylen = keylen;
  strncpy(proc->shm_slots[slot]->key, key, keylen + 1);

  // associate key with the shared ptr in the global map
  int err =
      hash_set(&shm_map, proc->shm_slots[slot]->key, proc->shm_slots[slot]);
  assert(!err);

  return (void *)shmmap(proc, slot, page, PAGE_FLAGS_USER_RW);
}

void *shm_acquire(const char *key)
{
  struct shm_page *shm = hash_get(&shm_map, (void *)key, NULL);
  if (shm == NULL) return NULL;

  // look for a free shm page slot
  struct proc *proc = get_current_process();
  const int    slot = find_slot(proc->shm_slots, NULL);
  if (slot < 0) return NULL;

  // mark slot and add a reference count to the shared page
  proc->shm_slots[slot] = shm;
  shm->refcount++;

  return (void *)shmmap(proc, slot, shm->shared, PAGE_FLAGS_USER_RW);
}

void shm_release(const char *key)
{
  struct shm_page *shm = hash_get(&shm_map, (void *)key, NULL);
  if (shm == NULL) return;

  // free local slot
  struct proc *proc = get_current_process();
  const int    slot = find_slot(proc->shm_slots, shm);
  if (slot < 0) return; // => never had acquired shared page with that key
  proc->shm_slots[slot] = NULL;

  // invalidate virtual page address mapping and remove shared reference
  shmmap(proc, slot, shm->shared, 0);
  unref(shm);
}

uint32_t shm_process_init(struct proc *proc, uint32_t shm_begin)
{
  // to ensure all shared pages go into one table, we want a 4MiB-aligned base
  assert(MAX_SHM_PAGES <= 1024);
  assert(shm_begin > 0);
  proc->shm_begin = shm_begin + PAGE_SIZE * PAGE_TABLE_LENGTH;
  if (proc->shm_begin < shm_begin) return 0;
  proc->shm_begin -= proc->shm_begin % (PAGE_SIZE * PAGE_TABLE_LENGTH);

  // NULLs mark free shared page slots
  memset(proc->shm_slots, 0, sizeof(proc->shm_slots));

  // preallocate a single non-shared pagetable for the shared pages
  struct page *pgtab = page_alloc();
  if (pgtab == NULL) return 0;
  pgtab->next = proc->pages;
  proc->pages = pgtab; // will be freed when process dies
  uint32_t *pgdir = (uint32_t *)proc->ctx.page_dir;
  ptab_map(pgdir, proc->shm_begin, pgtab->frame, PAGE_FLAGS_USER_RW);
  // no need to flush TLB, this is before process runs

  return proc->shm_begin + PAGE_SIZE * MAX_SHM_PAGES - 1;
}

extern void shm_process_destroy(struct proc *proc)
{
  for (int i = 0; i < MAX_SHM_PAGES; ++i) {
    if (proc->shm_slots[i] == NULL) continue;
    unref(proc->shm_slots[i]);
    proc->shm_slots[i] = NULL;
  }
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

static int find_slot(struct shm_page **slots, struct shm_page *ref)
{
  // we expect MAX_SHM_PAGES to be small so linear search is ok
  for (int i = 0; i < MAX_SHM_PAGES; ++i) {
    if (slots[i] == ref) return i;
  }
  return -1;
}

static uint32_t shmmap(struct proc *proc, int slot, struct page *shared,
                       unsigned flags)
{
  // map shared memory into the process' page dir
  const uint32_t virt = proc->shm_begin + slot * PAGE_SIZE;
  const uint32_t real = shared->frame * PAGE_SIZE;
  uint32_t *     pgdir = (uint32_t *)proc->ctx.page_dir;
  int            err = page_map(pgdir, virt, real, flags);
  assert(!err); // NOTE: we assume shared pages have their table already set up
  schedule();   // ensures TLB flush after page_map
  return virt;
}

static void unref(struct shm_page *shm)
{
  // decrease refcount and check whether shared page is still alive
  shm->refcount--;
  if (shm->refcount > 0) return;

  // remove global mapping and free resources allocated in shm_create
  const int err = hash_del(&shm_map, shm->key);
  assert(!err);
  page_free(shm->shared);
  mem_free(shm, sizeof(struct shm_page) + shm->keylen + 1);
}
