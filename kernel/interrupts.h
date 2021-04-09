/*
 * interrupts.h
 *
 *  Created on: 11/02/2021
 *      Author: baioc
 */

#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "stdbool.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/// Interrupt number offset that was setup in processor_structs.c:setup_pic.
#define INTR_VECTOR_OFFSET 0x20

/// Privilege levels.
#define PL_KERNEL 0
#define PL_USER   3

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/**
 * Initializes an IDT entry, configuring interrupt NUM to be treated by HANDLER
 * with PL privilege level (if a software interrupt can be triggered in
 * userspace, this should be PL_USER, otherwise use PL_KERNEL).
 */
void set_interrupt_handler(int num, void (*handler)(void), unsigned char pl);

/// Disables (MASK = true) or enables (MASK = false) the given IRQ channel.
void mask_irq(int irq, bool mask);

/// Acknowledges interrupt at IRQ, should be called at the very end of handlers.
void acknowledge_interrupt(int irq);

#endif // _INTERRUPTS_H_
