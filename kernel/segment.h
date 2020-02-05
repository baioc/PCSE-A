/*
 * segment.h
 *
 * Copyright (C) 2003 by Simon Nieuviarts
 *
 * Segment selectors.
 */

#ifndef __SEGMENT_H__
#define __SEGMENT_H__

#define BASE_TSS	0x08
#define KERNEL_CS	0x10	/* Kernel's PL0 code segment */
#define KERNEL_DS	0x18	/* Kernel's PL0 data segment */
#define USER_CS		0x43	/* User's code descriptor, RPL=3 */
#define USER_DS		0x4b	/* User's data descriptor, RPL=3 */
#define TRAP_TSS_BASE	0x50

#endif
