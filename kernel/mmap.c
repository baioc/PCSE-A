/*
 * mmap.c
 *
 *  Created on: 18/03/2021
 *      Author: baioc
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "mmap.h"

#include "stddef.h"
#include "debug.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define TOTAL_MEMORY  0x10000000 /* 256 MiB */
#define TOTAL_PAGES   (TOTAL_MEMORY / PAGE_SIZE)

#define KERNEL_MEMORY 0x04000000 /* 64 MiB */
#define KERNEL_PAGES  (KERNEL_MEMORY / PAGE_SIZE)

#define POOL_SIZE     (TOTAL_PAGES - KERNEL_PAGES) /* page units */

/*******************************************************************************
 * Types
 ******************************************************************************/

struct page {
  struct page *next;
};

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

// Free-list -based pool allocator for physical memory pages.
static struct page  g_memory[POOL_SIZE];
static struct page *g_free_list;

/*******************************************************************************
 * Public function
 ******************************************************************************/

// Reference: https://wiki.osdev.org/Paging
void page_map(uint32_t *pgdir, const void *virt, const void *real, int flags)
{
  // check for page alignment
  assert(((uint32_t)virt & (PAGE_SIZE-1)) == 0);
  assert(((uint32_t)real & (PAGE_SIZE-1)) == 0);

  // get the page dir entry and check if the associated page table is present
  const unsigned pd_index = ((uint32_t)virt >> 22) & 0x3FF;
  const uint32_t pd_entry = pgdir[pd_index];
  if (!(pd_entry & PAGE_PRESENT)) {
    // TODO: allocate missing page table
  }

  // get the page table entry and see it there's already a mapping present
  uint32_t * const pgtab = (uint32_t *)(pd_entry & 0xFFFFF000);
  const unsigned pt_index = ((uint32_t)virt >> 12) & 0x3FF;
  const uint32_t pt_entry = pgtab[pt_index];
  if (pt_entry & PAGE_PRESENT) {
    // TODO: resolve page mapping conflict
  }

  // setup address translation, applying flags and marking target as present
  pgtab[pt_index] = (uint32_t)real | (flags & 0xFFF) | PAGE_PRESENT;
  // TODO: do we need to flush some TLB entry?
}

void frame_init(void)
{
  // add all pages in the pool (w/ kernel frames excluded) to the free list
  g_free_list = NULL;
  for (unsigned i = 0; i < POOL_SIZE; ++i) {
    g_memory[i].next = g_free_list;
    g_free_list = &g_memory[i];
  }
}

int frame_alloc(void)
{
  // pop free list head and use its address to compute index and frame number
  if (g_free_list == NULL) return -1;
  const unsigned i = g_free_list - g_memory;
  g_free_list = g_free_list->next;
  return KERNEL_PAGES + i;
}

void frame_free(int frame)
{
  // get pool index from frame number
  const int i = frame - KERNEL_PAGES;
  assert(i >= 0 && i < POOL_SIZE);
  // add page to free list
  g_memory[i].next = g_free_list;
  g_free_list = &g_memory[i];
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/
