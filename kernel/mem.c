/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "mem.h"

#include "stddef.h"
#include "debug.h"
#include "string.h"
#include "pm.h"

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

/// Gets a pointer to the page directory entry a virtual address would need.
static uint32_t *pgdir_ref(const uint32_t *pgdir, uint32_t virt)
{
  return (uint32_t *)pgdir + ((virt >> 22) & 0x3FF);
}

/**
 * Gets a pointer to the page table entry a virtual address would need.
 * This depends on a valid pgdir entry and returns NULL in case there isn't.
 */
static uint32_t *pgtab_ref(const uint32_t *pgdir, uint32_t virt)
{
  uint32_t pd_entry = *pgdir_ref(pgdir, virt);
  if (!(pd_entry & PAGE_PRESENT)) return NULL;
  uint32_t *pgtab = (uint32_t *)(pd_entry & 0xFFFFF000);
  return (uint32_t *)pgtab + ((virt >> 12) & 0x3FF);
}

void ptab_map(uint32_t *pgdir, uint32_t virt, unsigned frame, unsigned flags)
{
  assert(virt % PAGE_SIZE == 0);
  uint32_t *pd_ref = pgdir_ref(pgdir, virt);
  *pd_ref = (frame * PAGE_SIZE) | (flags & 0xFFF);
}

int page_map(uint32_t *pgdir, uint32_t virt, uint32_t real, unsigned flags)
{
  // check page alignment
  assert(virt % PAGE_SIZE == 0);
  assert(real % PAGE_SIZE == 0);

  // get page table entry reference and setup address + flags
  uint32_t *pt_ref = pgtab_ref(pgdir, virt);
  if (pt_ref == NULL) return -1;
  *pt_ref = (real & 0xFFFFF000) | (flags & 0xFFF);

  return 0;
}

void *translate(const uint32_t *pgdir, uint32_t virt)
{
  uint32_t *pt_ref = pgtab_ref(pgdir, virt);
  if (pt_ref == NULL) return NULL;
  uint32_t pt_entry = *pt_ref;
  if (!(pt_entry & PAGE_PRESENT)) return NULL;
  uint32_t page = pt_entry & 0xFFFFF000;
  return (void *)(page | (virt & 0xFFF));
}

void mem_init(void)
{
  // add all pages in the pool (kernel frames excluded) to the free list
  g_free_list = NULL;
  for (int i = POOL_SIZE - 1; i >= 0; --i) {
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
  p->next = NULL;
  return p;
}

void page_free(struct page *p)
{
  // check address is valid and index wasn't messed up
  assert((p->frame - KERNEL_PAGES) == (unsigned)(p - g_user_memory));
  if (p < &g_user_memory[0] || p >= &g_user_memory[POOL_SIZE]) BUG();

  // zero out and return memory back to global free list
  memset((void *)(p->frame * PAGE_SIZE), 0, PAGE_SIZE);
  p->next = g_free_list;
  g_free_list = p;
}

bool access_ok(uint32_t addr, unsigned long size)
{
  return addr >= MMAP_USER_START && addr < MMAP_STACK_END &&
         size < MMAP_STACK_END - addr;
}

void *_sbrk(ptrdiff_t increment)
{
  struct proc *  proc = get_current_process();
  const uint32_t s = proc->brk;

  // check for overflow or oom
  const uint32_t c = s + increment;
  if ((c < proc->brk) || (c > proc->brk_limit)) return (void *)-1;

  // forward-align to check whether we need more pages
  assert(s >= PAGE_SIZE);
  uint32_t prev = s % PAGE_SIZE == 0 ? s : s - (s % PAGE_SIZE) + PAGE_SIZE;
  uint32_t next = c % PAGE_SIZE == 0 ? c : c - (c % PAGE_SIZE) + PAGE_SIZE;
  if (next > prev) {
    const uint32_t curr =
        mmap_region(proc, prev, next - prev, NULL, PAGE_FLAGS_USER_RW);
    if (curr == 0) return (void *)-1;
    assert(curr + 1 == next);
  }

  proc->brk = c;
  if (next > prev) schedule(); // ensures TLB flush

  return (void *)s;
}
