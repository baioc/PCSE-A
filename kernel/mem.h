/*
 * Copyright (C) 2005 -- Simon Nieuviarts
 * Copyright (C) 2012 -- Damien Dejean <dam.dejean@gmail.com>
 *
 * Memory allocator. These two functions will be satisfied at link time as
 * they are provided by the allocator contained in the shared library.
 */
#ifndef __MEM_H__
#define __MEM_H__

void *mem_alloc(unsigned long length);
void mem_free(void *zone, unsigned long length);

#endif
