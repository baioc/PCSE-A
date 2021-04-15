#ifndef __MEM_H__
#define __MEM_H__

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "stdint.h"
#include "stdbool.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define PAGE_SIZE 4096 /* 4 KiB */

#define PAGE_TABLE_LENGTH 1024

/// Page flags.
#define PAGE_PRESENT 0x01
#define PAGE_WRITE   0x02
#define PAGE_USER    0x04

#define PAGE_FLAGS_USER_RW (PAGE_USER | PAGE_WRITE | PAGE_PRESENT)

/// Userspace virtual mappings.
#define MMAP_USER_START 0x40000000 /* 1 GiB */
#define MMAP_STACK_END  0xC0000000 /* 3 GiB */

/*******************************************************************************
 * Types
 ******************************************************************************/

struct page {
  unsigned     frame; // page frame index
  struct page *next;  // while the page is allocated, this can be used freely
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*
 * Memory allocator. These two functions will be satisfied at link time as
 * they are provided by the allocator contained in the shared library.
 */
void *mem_alloc(unsigned long length);
void  mem_free(void *zone, unsigned long length);

/**
 * Initializes the physical page allocator system.
 * Considers kernel-reserved frames as being in use.
 */
void mem_init(void);

/**
 * Acquires an unused page frame of exactly PAGE_SIZE zeroed-out bytes.
 * On success, returns a pointer to a page struct, otherwise NULL.
 */
struct page *page_alloc(void);

/// Releases a page frame and scrubs its contents.
void page_free(struct page *p);

/**
 * Creates a virtual-physical address mapping in the PGDIR page directory.
 * Addresses VIRT and REAL must be page-aligned and the given FLAGS will apply.
 * NOTE: PAGE_PRESENT flag is needed to actually create the mapping, and in case
 * a mapping of VIRT already exists, it will be overwritten.
 *
 * If the page table for the requested virtual address is not marked as present
 * in the given page directory, this returns a non-zero value and has no further
 * effect. When this happens, you must first call ptab_map().
 */
int page_map(uint32_t *pgdir, uint32_t virt, uint32_t real, unsigned flags);

/**
 * Registers the physical page of index FRAME as the page table that would be
 * needed for the VIRT address in PGDIR, also applying FLAGS (use PAGE_PRESENT).
 * XXX: just like with page_map(), make sure the TLB is flushed afterwards.
 */
void ptab_map(uint32_t *pgdir, uint32_t virt, unsigned frame, unsigned flags);

/**
 * Manually translates a virtual address to a physical one mapped in the given
 * page directory.
 *
 * In case there's no such page mapping, returns NULL, otherwise this gives
 * the physical address the virtual one would map to.
 */
void *translate(const uint32_t *pgdir, uint32_t virt);

/**
 * Check whether a userspace memory range [ADDR ... ADDR+SIZE) is valid.
 * Returns false if it definitely isn't safe to use, but note that it could
 * still lead to page faults, for instance.
 */
bool access_ok(uint32_t addr, unsigned long size);

#endif
