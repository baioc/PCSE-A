/*
 * mmap.h
 *
 *  Created on: 18/03/2021
 *      Author: baioc
 */

#ifndef _MMAP_H_
#define _MMAP_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "stdint.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define PAGE_SIZE 0x1000 /* 4 KiB */

#define PAGE_TABLE_LENGTH 1024

/// Page flags.
#define PAGE_PRESENT 0x01
#define PAGE_WRITE   0x02
#define PAGE_USER    0x04

/*******************************************************************************
 * Types
 ******************************************************************************/

struct page {
  unsigned     frame; // page frame index
  struct page *next;  // while the page is allocated, this can be used freely
};

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/**
 * Initializes the physical page allocator system.
 * Considers kernel-reserved frames as being in use.
 */
void frame_init(void);

/**
 * Acquires an unused page frame of exactly PAGE_SIZE zeroed-out bytes.
 * Returns a pointer to a page struct on success, otherwise NULL.
 */
struct page *frame_alloc(void);

/// Releases a page frame and scrubs its contents.
void frame_free(struct page *p);

/**
 * Creates a virtual-physical address mapping in the PGD page directory.
 * Addresses VIRT and REAL must be page-aligned and the given FLAGS will apply.
 * NOTE: In case a mapping of VIRT already exists, it will be overwritten. Also,
 * this automatically adds PAGE_PRESENT to the given flags.
 *
 * If the page table for the requested virtual address is not marked as present
 * in the given page directory, this routine will return its index (a non
 * negative value) and have no further effect. When this happens, you must first
 * allocate a frame for the new page table and set it up in the page directory.
 * Otherwise, returns a negative value.
 */
int page_map(uint32_t *pgdir, uint32_t virt, uint32_t real, unsigned flags);

/**
 * Manually translates a virtual address to a physical one mapped in the given
 * page directory.
 *
 * In case there's no such page mappings, returns NULL, otherwise this gives
 * the physical address the virtual one would map to (which could also be NULL).
 */
void *translate(const uint32_t *pgdir, uint32_t virt);

#endif // _MMAP_H_
