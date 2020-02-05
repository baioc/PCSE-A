/*
 * early_mm.c
 *
 * Ensimag - Projet Système
 * Copyright (C) 2014 by Damien Dejean <dam.dejean@gmail.com>
 *
 * Early pagination management.
 * On boot a simple pagination structure is set up by bootstrap code (see crt0.S
 * and processor_structs.c). The aim is to protect code and data from pointer
 * faults and detect NULL pointer dereferencement.
 *
 * Based on the memory mapping provided by linker script, the setup is done as
 * follow:
 *
 * 0x00000000  -  0x00001000:  not mapped to handle NULL pointer accesses;
 * 0x00001000  -  _start:      mapped read/write;
 * _start      -  _text_start: mapped r/w, contains crt0.S (bootstrap code +
 *                             paging structures);
 * _text_start -  _rodata_end: mapped ro, contains .text and .rodata sections
 *                             of the kernel;
 * _data_start -  _bss_end:    mapped r/w, contains .data and .bss sections
 * _bss_end    -  256MB:       mapped r/w
 */

#include "cpu.h"
#include "string.h"


/* Real mode memory region */
#define FIRST_PAGE      0x00000000u
#define RM_START        FIRST_PAGE + 0x1000u

/* Kernel start */
extern char _start[];

/* .text section, ctr0.S excluded */
extern char _text_start[];
extern char _text_end[];

/* .multiboot section */
extern char _multiboot_start[];
extern char _multiboot_end[];

/* .bootstrap_stack section */
extern char _bootstrap_stack_start[];
extern char _bootstrap_stack_end[];

/* .rodata section */
extern char _rodata_start[];
extern char _rodata_end[];

/* .data section */
extern char _data_start[];
extern char _data_end[];

/* .bss section */
extern char _bss_start[];
extern char _bss_end[];

/* End of kernel binary */
extern char _end[];

/* End of supported memory */
extern char mem_end[];


/**
 * Asserts that an address is aligned on 4K.
 */
#define ASSERT_ALIGNED(symbol) do {                                             \
    if ((((unsigned)symbol) & 0xFFFFF000u) != ((unsigned)symbol)) {             \
       while (1) hlt();                                                         \
    }                                                                           \
} while (0)

/**
 * Asserts that a region is consistent.
 */
#define ASSERT_CONSISTENT(start, end) do {                  \
    if (((long int)end) - ((long int)start) < 4096) {       \
       while (1) hlt();                                     \
    }                                                       \
} while (0)


/**
 * Checks symbols consistency and alignment to ensure the kernel is able to boot
 * and map memory properly. Crash on bad result.
 */
void early_mm_check(void)
{
        ASSERT_ALIGNED(_start);

        /* .text section */
        ASSERT_ALIGNED(_text_start);
        ASSERT_ALIGNED(_text_end);
        ASSERT_CONSISTENT(_text_start, _text_end);

        /* .multiboot section */
        ASSERT_ALIGNED(_multiboot_start);
        ASSERT_ALIGNED(_multiboot_end);
        ASSERT_CONSISTENT(_multiboot_start, _multiboot_end);

        /* .bootstrap_stack section */
        ASSERT_ALIGNED(_bootstrap_stack_start);
        ASSERT_ALIGNED(_bootstrap_stack_end);
        ASSERT_CONSISTENT(_bootstrap_stack_start, _bootstrap_stack_end);

        /* .rodata section */
        ASSERT_ALIGNED(_rodata_start);
        ASSERT_ALIGNED(_rodata_end);
        ASSERT_CONSISTENT(_rodata_start, _rodata_end);

        /* .data section */
        ASSERT_ALIGNED(_data_start);
        ASSERT_ALIGNED(_data_end);
        ASSERT_CONSISTENT(_data_start, _data_end);

        /* .bss section */
        ASSERT_ALIGNED(_data_start);
        ASSERT_ALIGNED(_data_end);
        ASSERT_CONSISTENT(_data_start, _data_end);

        /* The whole kernel */
        ASSERT_CONSISTENT(_start, _end);
}


/* Page directory */
extern unsigned pgdir[];
#define PAGE_DIR_FLAGS     0x00000003u


/* Page tables */
extern unsigned pgtab[];
#define PAGE_TABLE_RO      0x000000001u
#define PAGE_TABLE_RW      0x000000003u

/**
 * Fill the provided pgdir with references on a big page table.
 * @param pgdir the page directory to fill.
 * @param pgtab the page table to reference.
 * @param count number of entry to fill.
 */
static void early_mm_fill_pgdir(unsigned pagedir[],
                                unsigned pagetab[],
                                unsigned count)
{
        unsigned i;
        unsigned pgdir_entry;

        pgdir_entry = (unsigned)pagetab;

        for (i = 0; i < count; i++) {
                pagedir[i] = (pgdir_entry + i * 0x1000) | PAGE_DIR_FLAGS;
        }
        for (i = count; i < 1024; i++) {
                pagedir[i] = 0;
        }
}

/**
 * Performs an identity mapping of the specified region with flags.
 * @param pagedir the page directory.
 * @param start start address of the region.
 * @param end end address of the region.
 * @param flags the flags to apply to the mapping.
 *
 * @pre start and end have to be aligned on 4K.
 *      pagedir must be initialized.
 */
static void early_mm_map_region(unsigned *pdir,
                                unsigned start,
                                unsigned end,
                                unsigned flags)
{
        unsigned address;

        for (address = start; address < end; address += 4096) {
                /* Page dir and table indexes */
                unsigned pd_index = address >> 22;
                unsigned pt_index = (address >> 12) & 0x3FFu;

                /* Get page table */
                unsigned *ptable = (unsigned*) (pdir[pd_index] & 0xFFFFF000);
                ptable[pt_index] = (address & 0xFFFFF000) | flags;
        }
}

/**
 * Create kernel initial memory mapping.
 */
void early_mm_map_kernel(void)
{
        /* Clear page tables */
        memset(pgtab, 0, 4096*64);

        /* Fill page directory for the first 256MB of memory */
        early_mm_fill_pgdir(pgdir, pgtab, 64);

        /*
         * Map all section independently, even if they are following each others
         * to prevent any inconsistency if the linker script is changed.
         */

        /* Zone 1: protect first page, no mapping. */
        pgtab[0] = 0;
        /* Zone 2: map read/write. */
        early_mm_map_region(pgdir, 4096, (unsigned)_start, PAGE_TABLE_RW);
        /* Zone 3: crt0.S mapped read/write for paging structures*/
        early_mm_map_region(pgdir, (unsigned)_start, (unsigned)_text_start, PAGE_TABLE_RW);
        /* Zone 4: .text, .rodata and .multiboot sections are obviously read only */
        early_mm_map_region(pgdir, (unsigned)_text_start, (unsigned)_text_end, PAGE_TABLE_RO);
        early_mm_map_region(pgdir, (unsigned)_multiboot_start, (unsigned)_multiboot_end, PAGE_TABLE_RO);
        early_mm_map_region(pgdir, (unsigned)_rodata_start, (unsigned)_rodata_end, PAGE_TABLE_RO);
        /* Except first stack which is RW */
        early_mm_map_region(pgdir, (unsigned)_bootstrap_stack_start, (unsigned)_bootstrap_stack_end, PAGE_TABLE_RW);
        /* Zone 5: .data and .bss are read/write */
        early_mm_map_region(pgdir, (unsigned)_data_start, (unsigned)_data_end, PAGE_TABLE_RW);
        early_mm_map_region(pgdir, (unsigned)_bss_start, (unsigned)_bss_end, PAGE_TABLE_RW);
        /* Zone 6: free memory is read/write */
        early_mm_map_region(pgdir, (unsigned)_end, (unsigned)mem_end, PAGE_TABLE_RW);
}

