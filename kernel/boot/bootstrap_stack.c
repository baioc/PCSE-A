/*
 * bootstrap_stack.c
 *
 * Ensimag - Projet Système
 * Copyright (C) 2014 by Damien Dejean <dam.dejean@gmail.com>
 */

#include "start.h"

unsigned char first_stack[FIRST_STACK_SIZE] __attribute__ ((section (".bootstrap_stack")));

