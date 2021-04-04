/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "mem.h"

#include "stddef.h"
#include "debug.h"
#include "string.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define TOTAL_MEMORY 0x10000000 /* 256 MiB */
#define TOTAL_PAGES  (TOTAL_MEMORY / PAGE_SIZE)

#define KERNEL_MEMORY 0x04000000 /* 64 MiB */
#define KERNEL_PAGES  (KERNEL_MEMORY / PAGE_SIZE)

#define POOL_SIZE (TOTAL_PAGES - KERNEL_PAGES) /* page units */

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Heap boundaries */
extern char  mem_heap[];
extern char  mem_heap_end[];
static char *curptr = mem_heap;

extern char mem_end[]; // physical memory end

// Free-list -based pool allocator for physical memory pages.
static struct page  g_user_memory[POOL_SIZE];
static struct page *g_free_list = NULL;

/*******************************************************************************
 * Public function
 ******************************************************************************/

/* Trivial sbrk implementation */
void *sbrk(ptrdiff_t diff)
{
  char *s = curptr;
  char *c = s + diff;
  if ((c < curptr) || (c > mem_heap_end)) return ((void *)(-1));
  curptr = c;
  return s;
}

// Reference: https://wiki.osdev.org/Paging
int page_map(uint32_t *pgdir, uint32_t virt, uint32_t real, unsigned flags)
{
  // check for page alignment
  assert(virt % PAGE_SIZE == 0);
  assert(real % PAGE_SIZE == 0);

  // get the page dir entry and check if the associated page table is present
  const unsigned pd_index = (virt >> 22) & 0x3FF;
  const uint32_t pd_entry = pgdir[pd_index];
  if (!(pd_entry & PAGE_PRESENT)) return pd_index;

  // get the page table entry
  uint32_t *const pgtab = (uint32_t *)(pd_entry & 0xFFFFF000);
  const unsigned  pt_index = (virt >> 12) & 0x3FF;

  // setup address translation, applying flags and marking target as present
  pgtab[pt_index] = real | (flags & 0xFFF) | PAGE_PRESENT;
  // NOTE: no need to flush TLB, it will be done automatically on ctx_sw()

  return -1; // === ok
}

void *translate(const uint32_t *pgdir, uint32_t virt)
{
  // get the page dir entry and check if the associated page table is present
  const unsigned pd_index = (virt >> 22) & 0x3FF;
  const uint32_t pd_entry = pgdir[pd_index];
  if (!(pd_entry & PAGE_PRESENT)) return NULL;

  // get the page table entry
  uint32_t *const pgtab = (uint32_t *)(pd_entry & 0xFFFFF000);
  const unsigned  pt_index = (virt >> 12) & 0x3FF;
  const uint32_t  pt_entry = pgtab[pt_index];
  if (!(pt_entry & PAGE_PRESENT)) return NULL;

  // get the actual physical page address and add the in-page offset in
  const uint32_t page = (uint32_t)(pt_entry & 0xFFFFF000);
  return (void *)(page | (virt & 0xFFF));
}

void mem_init(void)
{
  // add all pages in the pool (w/ kernel frames excluded) to the free list
  g_free_list = NULL;
  for (unsigned i = 0; i < POOL_SIZE; ++i) {
    g_user_memory[i].next = g_free_list;
    g_free_list = &g_user_memory[i];
  }

  // make sure all pages start zeroed-out
  assert(TOTAL_MEMORY == (uint32_t)mem_end);
  assert(KERNEL_MEMORY == (uint32_t)mem_heap_end);
  memset(mem_heap_end, 0, mem_end - mem_heap_end);
}

struct page *page_alloc(void)
{
  // pop free list head and use its address to compute index and frame number
  if (g_free_list == NULL) return NULL;
  struct page *p = g_free_list;
  g_free_list = g_free_list->next;
  p->frame = KERNEL_PAGES + (p - g_user_memory);
  return p;
}

void page_free(struct page *p)
{
  // zero out physical memory page and return it to the free list
  assert((p->frame - KERNEL_PAGES) == (unsigned)(p - g_user_memory));
  memset((void *)(p->frame * PAGE_SIZE), 0, PAGE_SIZE);
  p->next = g_free_list;
  g_free_list = p;
}

bool access_ok(uint32_t addr, unsigned long size)
{
  return addr >= MMAP_USER_START && addr < MMAP_STACK_END &&
         size < MMAP_STACK_END - addr;
}
