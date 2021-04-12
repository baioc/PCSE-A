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
#include "debug.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static hash_t shm_map; // "key" -> physical page pointer

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
  if (key == NULL) return NULL;
  if (hash_isset(&shm_map, (void *)key)) return NULL;

  // check whether the process has any free shm slots
  struct proc *proc = get_current_process();
  struct page *page = proc->shm_free;
  if (page == NULL) return NULL;

  // allocate a physical page to be shared
  struct page *frame = page_alloc();
  if (frame == NULL) return NULL;

  // mark the shm page (and the key) as being in use
  proc->shm_free = proc->shm_free->next;
  hash_set(&shm_map, (void *)key, frame);

  // in each process, we have to link a local copy with the shared frame index
  *page = (struct page){.frame = frame->frame};
  page->next = proc->pages;
  proc->pages = page;

  // map shared memory into the process' page dir
  const uint32_t virt = proc->shm_begin + (page - proc->shm_pages) * PAGE_SIZE;
  const uint32_t real = frame->frame * PAGE_SIZE;
  const int      pde =
      page_map((uint32_t *)proc->ctx.page_dir, virt, real, PAGE_FLAGS_USER_RW);
  assert(pde < 0);

  return (void *)virt;
}

void *shm_acquire(const char *key)
{
  struct page *frame = hash_get(&shm_map, (void *)key, NULL);
  if (frame == NULL) return NULL;

  struct proc *proc = get_current_process();
  struct page *page = proc->shm_free;
  if (page == NULL) return NULL;

  // add a reference count to the shared page
  proc->shm_free = proc->shm_free->next;
  frame->refcount++;

  // link copied frame index
  *page = (struct page){.frame = frame->frame};
  page->next = proc->pages;
  proc->pages = page;

  // map new virtual page into existing shared page
  const uint32_t virt = proc->shm_begin + (page - proc->shm_pages) * PAGE_SIZE;
  const uint32_t real = frame->frame * PAGE_SIZE;
  const int      pde =
      page_map((uint32_t *)proc->ctx.page_dir, virt, real, PAGE_FLAGS_USER_RW);
  assert(pde < 0);

  return (void *)virt;
}

void shm_release(const char *key)
{
  struct page *frame = hash_get(&shm_map, (void *)key, NULL);
  if (frame == NULL) return;

  // unlink process-local copy
  struct proc *proc = get_current_process();
  struct page *prev = NULL;
  struct page *curr = proc->pages;
  while (curr != NULL && curr->frame != frame->frame) {
    prev = curr;
    curr = curr->next;
  }
  if (curr == NULL)
    return; // => this process had never acquired shm[key]
  else if (curr == proc->pages)
    proc->pages = proc->pages->next;
  else
    prev->next = curr->next;

  // add this proc's local shm_page back to its free list
  curr->next = proc->shm_free;
  proc->shm_free = curr;

  // page_free checks refcount, but we must verify the need to clear the key
  const bool alive = frame->refcount > 1;
  page_free(frame);
  if (!alive) hash_del(&shm_map, (void *)key);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/
