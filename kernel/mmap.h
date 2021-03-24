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

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/**
 * Creates a virtual-physical address mapping in the PGD page directory.
 * Addresses VIRT and REAL must be page-aligned and the given FLAGS will apply.
 * NOTE: In case a mapping of VIRST already exists, it will be overwritten.
 *
 * If the page table for the requested virtual address is not marked as present
 * in the given page directory, this routine will return its index (a strictly
 * positive value) and have no further effect. When this happens, you must first
 * allocate a frame for the new page table and set it up in the page directory.
 * Otherwise, returns 0.
 */
int page_map(uint32_t *pgdir, const void *virt, const void *real, int flags);

/**
 * Initializes the physical page allocator system.
 * Considers kernel-reserved frames as being in use.
 */
void frame_init(void);

/**
 * Acquires an unused page frame of exactly PAGE_SIZE bytes.
 * Returns a frame index on success and a negative value when out of memory.
 */
int frame_alloc(void);

/// Releases a page frame with a certain index.
void frame_free(int frame);

#endif // _MMAP_H_
