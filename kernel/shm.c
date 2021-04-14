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

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define MAX_SHM_PAGES 128 /* per process */

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/// Maps a shared page into a process and links a local to it.
static uint32_t page_share(struct proc *proc, struct page *restrict local,
                           const struct page *restrict shared);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static hash_t shm_map; // "key" -> pointer to physical page struct

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

  // check whether the process has any free shm slots
  struct proc *proc = get_current_process();
  struct page *page = proc->shm_free;
  if (page == NULL) return NULL;

  // allocate a physical page to be shared
  struct page *frame = page_alloc();
  if (frame == NULL) return NULL;

  // mark the shm page (and the key) as being in use
  frame->ref.count = 1;
  hash_set(&shm_map, (void *)key, frame);
  proc->shm_free = proc->shm_free->ref.next;

  return (void *)page_share(proc, page, frame);
}

void *shm_acquire(const char *key)
{
  struct page *frame = hash_get(&shm_map, (void *)key, NULL);
  if (frame == NULL) return NULL;

  struct proc *proc = get_current_process();
  struct page *page = proc->shm_free;
  if (page == NULL) return NULL;

  // add a reference count to the shared page
  frame->ref.count += 1;
  proc->shm_free = proc->shm_free->ref.next;

  return (void *)page_share(proc, page, frame);
}

void shm_release(const char *key)
{
  struct page *frame = hash_get(&shm_map, (void *)key, NULL);
  if (frame == NULL) return;

  // unlink process-local copy
  struct proc *proc = get_current_process();
  struct page *prev = NULL;
  struct page *page = proc->shm_used;
  while (page != NULL) {
    if (page->frame == frame->frame) break;
    prev = page;
    page = page->ref.next;
  }
  if (page == NULL) // => never had acquired page with that key
    return;
  else if (prev == NULL) // => page is list head
    proc->shm_used = proc->shm_used->ref.next;
  else // => unchain
    prev->ref.next = page->ref.next;

  // add local page back to free list
  page->ref.next = proc->shm_free;
  proc->shm_free = page;

  // only free when refcount reaches zero
  frame->ref.count -= 1;
  if (frame->ref.count <= 0) {
    hash_del(&shm_map, (void *)key);
    page_free(frame);
  }
}

uint32_t shm_process_init(struct proc *proc, uint32_t shm_begin)
{
  // to ensure all shared pages go into one table, we want a 4MiB-aligned base
  assert(MAX_SHM_PAGES <= 1024);
  proc->shm_begin = shm_begin + PAGE_SIZE * PAGE_TABLE_LENGTH;
  if (proc->shm_begin < shm_begin) return 0; // => overflow check
  proc->shm_begin -= proc->shm_begin % (PAGE_SIZE * PAGE_TABLE_LENGTH);

  // allocate memory for the local page struct storage
  proc->shm_pages = mem_alloc(MAX_SHM_PAGES * sizeof(struct page));
  if (proc->shm_pages == NULL) return 0;
  proc->shm_used = NULL;
  proc->shm_free = NULL;
  for (int i = MAX_SHM_PAGES - 1; i >= 0; --i) {
    proc->shm_pages[i].ref.next = proc->shm_free;
    proc->shm_free = &proc->shm_pages[i];
  }

  // preallocate a single non-shared pagetable for the shared pages
  struct page *shm_pgtab = page_alloc();
  if (shm_pgtab == NULL) {
    mem_free(proc->shm_pages, MAX_SHM_PAGES * sizeof(struct page));
    return 0;
  }
  shm_pgtab->ref.next = proc->pages;
  proc->pages = shm_pgtab;
  uint32_t *pgdir = (uint32_t*)proc->ctx.page_dir;
  ptab_map(pgdir, proc->shm_begin, shm_pgtab->frame, PAGE_FLAGS_USER_RW);

  return proc->shm_begin + PAGE_SIZE*MAX_SHM_PAGES - 1;
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

static uint32_t page_share(struct proc *proc, struct page *restrict local,
                           const struct page *restrict shared)
{
  // link a process-local copy of the shared frame
  local->frame = shared->frame; // frame number is later used to identify it
  local->ref.next = proc->shm_used;
  proc->shm_used = local;

  // map shared memory into the process' page dir
  const uint32_t virt = proc->shm_begin + (local - proc->shm_pages) * PAGE_SIZE;
  const uint32_t real = shared->frame * PAGE_SIZE;
  uint32_t *     pgdir = (uint32_t *)proc->ctx.page_dir;
  int            err = page_map(pgdir, virt, real, PAGE_FLAGS_USER_RW);
  assert(!err); // NOTE: we assume shared pages have their table already set up

  return virt;
}
