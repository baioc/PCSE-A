/*
 * boot.c
 *
 * Ensimag - Projet Système
 * Copyright (C) 2014 - Damien Dejean <dam.dejean@gmail.com>
 *
 * Kernel boot process.
 */

#include "multiboot.h"
#include "start.h"
#include "processor_structs.h"
#include "early_mm.h"
#include "string.h"

/* Multiboot symbols from crt0.S */
extern unsigned multiboot_magic;
extern multiboot_info_t *multiboot_info;

/* Symbols for memory cleaning */
extern char _data_end[];
extern char mem_heap_end[];

/* Helpers */
extern void enable_paging(void);

/**
 * First boot function. Called from crt0.S after minimal environment setup.
 */
void boot(void)
{
        /* Save multiboot context */
        multiboot_save(multiboot_magic, multiboot_info);
        /* Blank all free memory */
        memset(_data_end, 0, (size_t)mem_heap_end - (size_t)_data_end);
        /* Initialize CPU structures */
        cpu_init();
        /* Setup paging */
        early_mm_check();       // Check binary linkage
        early_mm_map_kernel();  // Map kernel memory
        enable_paging();        // Enable CPU paging

        /*** To run C++ yout should call CTOR list now ***/

        /* Kernel main */
        kernel_start();

        /* Kernel never returns */
        reboot();
}
