#ifndef __PROCESSOR_STRUCTS_H__
#define __PROCESSOR_STRUCTS_H__

#define GDT_ENTRIES	8192
#include "segment.h"

#define IDT_ENTRIES	256


#ifndef ASSEMBLER

struct x86_tss {
	int             back_link;      /* segment number of previous task,
	                                   if nested */
	int             esp0;           /* initial stack pointer ... */
	int             ss0;            /* and segment for ring 0 */
	int             esp1;           /* initial stack pointer ... */
	int             ss1;            /* and segment for ring 1 */
	int             esp2;           /* initial stack pointer ... */
	int             ss2;            /* and segment for ring 2 */
	int             cr3;            /* CR3 - page table directory
	                                         physical address */
	int             eip;
	int             eflags;
	int             eax;
	int             ecx;
	int             edx;
	int             ebx;
	int             esp;            /* current stack pointer */
	int             ebp;
	int             esi;
	int             edi;
	int             es;
	int             cs;
	int             ss;             /* current stack segment */
	int             ds;
	int             fs;
	int             gs;
	int             ldt;            /* local descriptor table segment */
	unsigned short  trace_trap;     /* trap on switch to this task */
	unsigned short  io_bit_map_offset;
                                        /* offset to start of IO permission
                                           bit map */
};

extern unsigned long long gdt[GDT_ENTRIES];
extern unsigned long long idt[IDT_ENTRIES];
extern struct x86_tss tss;

void cpu_init(void);

void reboot(void);

#endif

#endif
