/*
 * Ensimag - Projet système
 * Copyright (C) 2005 - Simon Nieuviarts
 * Copyright (C) 2012 - Damien Dejean <dam.dejean@gmail.com>
 *
 * Ld script that defines how a userspace application have to be linked.
 * Each app is located at the address 1GB with code (.text section) at the
 * beginning.
 */

FORCE_COMMON_ALLOCATION

/* An app is delivered as a raw binary */
OUTPUT_FORMAT(binary)

/*
 * Define application entry point: this symbol is available in lib/crt0.c
 * as a function that will call the application main.
 */
ENTRY(_start)

SECTIONS {
	/* Locate application at address 1GB */
	. = 0x40000000;

    /* Ensure that stdlib bootstrap code is the first */
    .text.init : {
        out/libstd.a:crt0.o(.text.init)
    }

    /* Application code first */
	.text : {
		*(.text)
	}

	.data ALIGN(0x1000) : {
		__CTOR_LIST__ = .;
		LONG((__CTOR_END__ - __CTOR_LIST__) / 4 - 2)
		*(.ctors)
		LONG(0)
		__CTOR_END__ = .;
		__DTOR_LIST__ = .;
		LONG((__DTOR_END__ - __DTOR_LIST__) / 4 - 2)
		*(.dtors)
		LONG(0)
		__DTOR_END__ = .;

		*(.fini)
		*(.anno)
		*(.rodata)
		*(.rodata.str1.1)
		*(.rodata.str1.4)
		*(.rodata.str1.32)
		*(__ex_table)
		*(.rodata.cst8)
		*(.data)
		*(.bss)
		*(COMMON)
	}

	/DISCARD/ : {
		*(.comment)
	}
}

