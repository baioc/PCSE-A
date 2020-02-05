/*
 * assert.c
 *
 * Ensimag - Projet Système
 * Copyright 2014 - Damien Dejean <dam.dejean@gmail.com>
 *
 * Basic assertion mechanism.
 */

#include "cpu.h"

#define ASSERT(cond) do {               \
        if (!(cond)) while(1) hlt();    \
} while (0)
