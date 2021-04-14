/*
 * clock.c
 *
 *  Created on: 11/02/2021
 *      Author: baioc
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "clock.h"

#include "cpu.h"
#include "stdint.h"
#include "interrupts.h"
#include "stddef.h"
#include "debug.h"

/// Ticks the current process time, may end up calling the scheduler.
extern void process_tick(void);

/// Makes the current process yield the CPU for at least TICKS jiffies.
extern void sleep(unsigned long ticks);

extern void clock_tick_handler(void);

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define QUARTZ_FREQ   1193182
#define TIC_CMND_PORT 0x43
#define TIC_DATA_PORT 0x40

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static unsigned long g_jiffies = 0;

/*******************************************************************************
 * Public function
 ******************************************************************************/

// see https://wiki.osdev.org/Programmable_Interval_Timer
void clock_init(void)
{
  const uint16_t divisor = QUARTZ_FREQ / CLOCKFREQ;

  // Channel 0, Access lobyte/hibyte, Operate as rate generator, Binary format
  outb(0b00110100, TIC_CMND_PORT);
  outb(divisor & 0x00FF, TIC_DATA_PORT);
  outb((divisor & 0xFF00) >> 8, TIC_DATA_PORT);

  // reset jiffies
  g_jiffies = 0;

  // setup intr handler and unmask IRQ
  set_interrupt_handler(INTR_VECTOR_OFFSET + 0, clock_tick_handler, PL_KERNEL);
  mask_irq(0, false);
}

void clock_settings(unsigned long *quartz, unsigned long *ticks)
{
  assert(quartz != NULL);
  assert(ticks != NULL);
  *quartz = QUARTZ_FREQ;
  *ticks = QUARTZ_FREQ / CLOCKFREQ;
}

unsigned long current_clock(void)
{
  return g_jiffies;
}

void wait_clock(unsigned long ticks)
{
  sleep(ticks);
}

void clock_tick(void)
{
  g_jiffies += 1;
  acknowledge_interrupt(0);
  process_tick();
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/
