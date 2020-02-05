/*
 * Copyright (C) 2005 Simon Nieuviarts
 *
 * Memory allocator.
 */
#ifndef __MEM_H__
#define __MEM_H__

void *mem_alloc(unsigned long length);
void mem_free(void *zone, unsigned long length);

#endif
