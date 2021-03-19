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
static struct page  g_page_pool[POOL_SIZE];
static struct page *g_free_list;

/*******************************************************************************
 * Public function
 ******************************************************************************/

void page_init(void)
{
  // add all pages in the pool (w/ kernel frames excluded) to the free list
  g_free_list = NULL;
  for (unsigned i = 0; i < POOL_SIZE; ++i) {
    g_page_pool[i].next = g_free_list;
    g_free_list = &g_page_pool[i];
  }
}

int page_alloc(void)
{
  // pop free list head and use its address to compute index and frame number
  if (g_free_list == NULL) return -1;
  const unsigned i = g_free_list - g_page_pool;
  g_free_list = g_free_list->next;
  return KERNEL_PAGES + i;
}

void page_free(int frame)
{
  // get pool index from frame number
  const int i = frame - KERNEL_PAGES;
  assert(i >= 0 && i < POOL_SIZE);
  // add page to free list
  g_page_pool[i].next = g_free_list;
  g_free_list = &g_page_pool[i];
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/
