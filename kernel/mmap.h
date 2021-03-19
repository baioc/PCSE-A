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

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define PAGE_SIZE 0x1000 /* 4 KiB */

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
 * Initializes the physical page allocator system.
 * Considers kernel-reserved frames as being in use.
 */
void page_init(void);

/**
 * Acquires an unused physical memory page of PAGE_SIZE bytes.
 * Returns a frame index on success and a negative value when out of memory.
 */
int page_alloc(void);

/// Releases a physical memory page with a certain frame index.
void page_free(int frame);

#endif // _MMAP_H_
