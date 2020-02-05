/*
 * multiboot.c
 *
 * Ensimag - Projet Système
 * Copyright (C) 2014 - Damien Dejean <dam.dejean@gmail.com>
 *
 * Multiboot utils implementation.
 * @see http://www.gnu.org/software/grub/manual/multiboot/multiboot.html
 */

#include "multiboot.h"
#include "string.h"
#include "early_assert.h"

/*
 * Backup multiboot structure.
 */
static multiboot_info_t mb_info __attribute__ ((section (".multiboot")));

void multiboot_save(unsigned magic, multiboot_info_t *mb)
{
        ASSERT(magic == MULTIBOOT_BOOTLOADER_MAGIC);

        /* Save the multiboot structure */
        memcpy(&mb_info, mb, sizeof(mb_info));
}

unsigned multiboot_upper_mem(void)
{
        ASSERT((mb_info.flags & MULTIBOOT_INFO_MEMORY) == MULTIBOOT_INFO_MEMORY);

        /* Memory is provided in KB minus 1MB */
        return mb_info.mem_upper + 1024u;
}
