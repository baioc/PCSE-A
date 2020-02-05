/*
 * crt0.S
 *
 * Copyright (C) 2003-2007 by Simon Nieuviarts
 *
 * Kernel entry point.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "start.h"

	.text
	.comm   multiboot_magic,4,4
	.comm   multiboot_info,4,4

	/* The entry point must be before the multiboot header. */
.global entry
entry:
	/* Setup the code segment. */
	jmp	entry2

	/* The multiboot header. */
	.p2align 2
0:	.long	0x1BADB002		/* magic number. */
	.long	0x00010002		/* flags. */
	.long	-0x1BADB002-0x00010002	/* checksum. */
	.long	0b
	.long	_start
	.long	_data_end
	.long	_bss_end
	.long	entry

	/*
	 * This global label is here to realign the debugger on the beginning
	 * of the code
	 */
entry2:
	movl	%eax,multiboot_magic	/* It can be verified later. */
	movl	%ebx,multiboot_info	/* It can be used later. */

	/* We have to set up a stack. */
	leal	first_stack,%esp
	addl	$FIRST_STACK_SIZE,%esp
	xorl	%ebp,%ebp

	/* Clear the flags. */
	pushl	$0
	popfl
	/* Note that the Interrupt Flag is cleared : interrupts are disabled. */

	call    boot

	/* This function should never exit. If it happens, reboot. */
	call	reboot

.global enable_paging
enable_paging:
	/* Enable paging */
	leal	pgdir,%eax
	/* CR3 = Base Address of the Page Directory */
	/* Cf. 3.6.3 Intel Architecture Software Developer's Manual Volume 3 */
	movl	%eax,%cr3
	/* Task State Segment, CR3 field must be initialized to Base Address of the Page Directory */
	/* Cf. 3.6.3 Intel Architecture Software Developer's Manual Volume 3 */
	movl	%eax,tss+28
	/* PG (paging) flag, bit 31 of CR0 : must to be set to active paging */
	/* Cf. 6.2.1 Intel Architecture Software Developer's Manual Volume 3 */
        /* Also sets WP, bit 16 of CR0 so kernel writes into read only pages produce page faults */
	movl	%cr0,%eax  
	orl	$0x80010000,%eax
	movl	%eax,%cr0

	jmp	0f
0:	movl	$0f,%eax
	jmp	*%eax
0:	ret

/* Unimplemented functions.
You have to implement them it in ".c" files. As these symbols are marked as
"weak", they will no more be used as soon as you write your implementations.
*/
	.weak console_putbytes
console_putbytes:
	ret

	.weak keyboard_data
keyboard_data:
	ret

/* Implement this one only if you want to light LEDs in sync with keyboard's
state. */
	.weak kbd_leds
kbd_leds:
	ret

    .org    0x1000
    /* This address will be mapped at kernel_base(0x10000) + 0x1000=0x101000 */
    /* page directory */
    /* Cf. 3.6.2 Intel Architecture Software Developer's Manual Volume 3 */
    .global pgdir
pgdir:
    /* Fill with zeros the page directory */
    .fill   1024,4,0

    /* Page table: will contain all mappings for the kernel space */
    .org    0x2000
    .global pgtab
pgtab:
    .org    0x42000
