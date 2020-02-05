#include "processor_structs.h"
#include "start.h"
#include "string.h"
#include "debug.h"
#include "cpu.h"
#include "gdb_serial_support.h"
#include "early_mm.h"

#define SZ_32           0x4                     /* 32-bit segment */
#define SZ_16           0x0                     /* 16-bit segment */
#define SZ_G            0x8                     /* 4K limit field */

#define ACC_A           0x01                    /* accessed */
#define ACC_TYPE        0x1e                    /* type field: */

#define ACC_TYPE_SYSTEM 0x00                    /* system descriptors: */

#define ACC_LDT         0x02                        /* LDT */
#define ACC_CALL_GATE_16 0x04                       /* 16-bit call gate */
#define ACC_TASK_GATE   0x05                        /* task gate */
#define ACC_TSS         0x09                        /* task segment */
#define ACC_CALL_GATE   0x0c                        /* call gate */
#define ACC_INTR_GATE   0x0e                        /* interrupt gate */
#define ACC_TRAP_GATE   0x0f                        /* trap gate */

#define ACC_TSS_BUSY    0x02                        /* task busy */

#define ACC_TYPE_USER   0x10                    /* user descriptors */

#define ACC_DATA        0x10                        /* data */
#define ACC_DATA_W      0x12                        /* data, writable */
#define ACC_DATA_E      0x14                        /* data, expand-down */
#define ACC_DATA_EW     0x16                        /* data, expand-down,
                                                             writable */
#define ACC_CODE        0x18                        /* code */
#define ACC_CODE_R      0x1a                        /* code, readable */
#define ACC_CODE_C      0x1c                        /* code, conforming */
#define ACC_CODE_CR     0x1e                        /* code, conforming,
                                                       readable */
#define ACC_PL          0x60                    /* access rights: */
#define ACC_PL_K        0x00                    /* kernel access only */
#define ACC_PL_U        0x60                    /* user access */
#define ACC_P           0x80                    /* segment present */

/*
 * Components of a selector
 */
#define SEL_LDT         0x04                    /* local selector */
#define SEL_PL          0x03                    /* privilege level: */
#define SEL_PL_K        0x00                        /* kernel selector */
#define SEL_PL_U        0x03                        /* user selector */


static void fill_descriptor(unsigned long long *entry, void *addr,
	unsigned limit, unsigned access, unsigned sizebits)
{
	unsigned long p0, p1;
	unsigned long base = (unsigned long) addr;

	if (limit > 0xfffff) {
		limit >>= 12;
		sizebits |= SZ_G;
	}
	p0 = (limit & 0xffff) + ((base & 0xffff) << 16);
	p1 = (base >> 16) & 0xff;
	p1 |= ((access | ACC_P) & 0xff) << 8;
	p1 |= limit & 0xf0000;
	p1 |= (sizebits & 0xf) << 20;
	p1 |= base & 0xff000000;
	*entry = (((unsigned long long) p1) << 32) | p0;
}

static void fill_gate(unsigned long long *entry, unsigned offset,
	unsigned short selector, unsigned char access, unsigned char word_count)
{
	unsigned long p0, p1;

	p0 = offset & 0xffff;
	p0 |= selector << 16;
	p1 = word_count & 0xf;
	p1 |= (access | ACC_P) << 8;
	p1 |= offset & 0xffff0000;
	*entry = (((unsigned long long) p1) << 32) | p0;
}


struct pseudo_descriptor {
        short pad;
        unsigned short limit;
        unsigned long linear_base;
};

#define HANDLER_ENTRIES 32
static struct x86_tss trap_tss[HANDLER_ENTRIES];

static void setup_gdt(void)
{
        struct pseudo_descriptor pdesc;
        int i;

	memset(gdt, 0, sizeof(gdt));
	fill_descriptor(&gdt[BASE_TSS / 8], &tss, sizeof(tss) - 1,
		ACC_PL_K | ACC_TSS | ACC_P, 0);
	fill_descriptor(&gdt[KERNEL_CS / 8], 0, 0xffffffff,
		ACC_PL_K | ACC_CODE_R, SZ_32);
	fill_descriptor(&gdt[KERNEL_DS / 8], 0, 0xffffffff,
		ACC_PL_K | ACC_DATA_W, SZ_32);
	fill_descriptor(&gdt[USER_CS / 8], 0, 0xffffffff,
		ACC_PL_U | ACC_CODE_R, SZ_32);
	fill_descriptor(&gdt[USER_DS / 8], 0, 0xffffffff,
		ACC_PL_U | ACC_DATA_W, SZ_32);

	for (i=0; i<HANDLER_ENTRIES; i++) {
		fill_descriptor(&gdt[i + (TRAP_TSS_BASE / 8)], trap_tss + i,
			sizeof(struct x86_tss) - 1, ACC_PL_K | ACC_TSS | ACC_P, 0);
	}

        pdesc.limit = sizeof(gdt) - 1;
        pdesc.linear_base = (unsigned long) &gdt;

	__asm__ __volatile__("lgdt %0" :: "m" (pdesc.limit), "m" (pdesc.linear_base));
	__asm__ __volatile__("lldt %w0" :: "r" (0));

	/* Load segment selectors. */
        __asm__ __volatile__("ljmp %0,$0f\n\t0:" :: "i" (KERNEL_CS));
        __asm__ __volatile__("movw %w0,%%ds" :: "r" (KERNEL_DS));
        __asm__ __volatile__("movw %w0,%%es" :: "r" (KERNEL_DS));
        __asm__ __volatile__("movw %w0,%%fs" :: "r" (0));
        __asm__ __volatile__("movw %w0,%%gs" :: "r" (0));
        __asm__ __volatile__("movw %w0,%%ss" :: "r" (KERNEL_DS));
}

extern unsigned long exception_handler_tasks[HANDLER_ENTRIES];

static char trap_stack[16384];

extern char pgdir[];

static void setup_idt(void)
{
	struct pseudo_descriptor pdesc;
	unsigned int i;

	memset(idt, 0, sizeof(idt));

	for (i=0; i<sizeof(exception_handler_tasks)/sizeof(unsigned long); i++) {
		unsigned access;
		struct x86_tss *ts = trap_tss + i;
		ts->ss0 = KERNEL_DS;
		ts->esp0 = (unsigned) (trap_stack + sizeof(trap_stack));
		ts->ss = KERNEL_DS;
		ts->esp = ts->esp0;
		ts->io_bit_map_offset = sizeof(struct x86_tss);
		ts->cs = KERNEL_CS;
		ts->eip = exception_handler_tasks[i];
		ts->ds = KERNEL_DS;
		ts->es = KERNEL_DS;
		ts->fs = 0;
		ts->gs = 0;
		ts->eflags = 2;
		ts->trace_trap = 0;
		ts->cr3 = (int)pgdir;
		access = ACC_TASK_GATE;
		access += (i == 3) ? 0x60 : 0;
		fill_gate(idt + i, 0, TRAP_TSS_BASE + 8 * i, access, 0);
	}

        pdesc.limit = sizeof(idt) - 1;
        pdesc.linear_base = (unsigned long) &idt;

	__asm__ __volatile__("lidt %0" :: "m" (pdesc.limit), "m" (pdesc.linear_base));
}

static void setup_tss(void)
{
	memset(&tss, 0, sizeof(tss));
	tss.ss0 = KERNEL_DS;
	tss.esp0 = (int) (first_stack + FIRST_STACK_SIZE);
	tss.io_bit_map_offset = sizeof(tss);
	tss.cr3 = (int)pgdir;
	__asm__ __volatile__("ltr %0" : : "rm" ((unsigned short)(BASE_TSS)));
}

void reboot(void)
{
	struct pseudo_descriptor pdesc;

	gdb_serial_exit(0);
	pdesc.limit = 1;
	pdesc.linear_base = (unsigned long) &idt;
	__asm__ __volatile__("lidt %0" :: "m" (pdesc.limit));
	__asm__ __volatile__ ("int $3");
}

static void setup_pic()
{
	/* Initialize the master. */
	outb(0x11, 0x20);
	outb(0x20, 0x21);
	outb(0x4, 0x21);
	outb(0x1, 0x21);
	
	/* Initialize the slave. */
	outb(0x11, 0xa0);
	outb(0x28, 0xa1);
	outb(0x2, 0xa1);
	outb(0x1, 0xa1);
	
	/* Ack any bogus intrs by setting the End Of Interrupt bit. */
	outb(0x20, 0x20);
	outb(0x20, 0xa0);
	
	/* Disable all IRQs */
	outb(0xff, 0x21);
	outb(0xff, 0xa1);
}


void cpu_init(void)
{
        setup_gdt();
        setup_idt();
        setup_tss();
        setup_pic();
}
