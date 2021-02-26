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

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/// Initializes IDT entry, configuring interrupt NUM to be treated by HANDLER.
void set_interrupt_handler(int num, void (*handler)(void));

/// Disables (MASK = true) or enables (MASK = false) the given IRQ channel.
void mask_irq(int irq, bool mask);

/// Acknowledges interrupt at IRQ, should be called at the very end of handlers.
void acknowledge_interrupt(int irq);

#endif // _INTERRUPTS_H_
