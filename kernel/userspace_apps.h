/*
 * Ensimag - Projet système
 * Copyright (C) 2012 - Damien Dejean <ddejean@vmware.com>
 *
 * Defines structure to find user space programs.
 */

#ifndef _USERSPACE_APPS_H
#define _USERSPACE_APPS_H

/**
 * Describes a userspace app. An app is provided as a chunk of data that
 * contains the code, and the data of the application.
 * <name>  the name of the application as defined in user directory.
 * <start> the address of the beginning of the binary in the physical
 *         memory.
 * <end>   the end address of the binary
 */
struct uapps {
    const char *name;
    void *start;
    void *end;
};

/**
 * A table of descriptor that reference all userspace binaries.
 */
extern const struct uapps symbols_table[];

#endif

