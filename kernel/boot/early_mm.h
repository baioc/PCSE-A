/*
 * early_mm.h
 *
 * Ensimag - Projet Système
 * Copyright (C) 2014 by Damien Dejean <dam.dejean@gmail.com>
 *
 * Kernel early paging facilities.
 */

/**
 * Checks symbols consistency and alignment to ensure the kernel is able to boot
 * and map memory properly. Crash on bad result.
 */
void early_mm_check(void);

/**
 * Create kernel initial memory mapping.
 */
void early_mm_map_kernel(void);
