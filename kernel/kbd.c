/*
 * clock.c
 *
 *  Created on: 09/04/2021
 *      Author: baioc
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "kbd.h"

#include "interrupts.h"
#include "cpu.h"
#include "stdint.h"
#include "debug.h"
#include "stdio.h"

extern void kbd_interrupt_handler(void);

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define PS2_CMND_PORT 0x64
#define PS2_DATA_PORT 0x60

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

void kbd_init(void)
{
  // PS/2 driver initialization sequence
  // see https://wiki.osdev.org/"8042"_PS/2_Controller
  outb(0xAD, PS2_CMND_PORT); // disable devices
  outb(0xA7, PS2_CMND_PORT);
  inb(PS2_DATA_PORT); // flush buffer
  outb(0x20, PS2_CMND_PORT);
  uint8_t ccb = inb(PS2_DATA_PORT); // check controller configuration byte
  printf("initial PS/2 controller configuration: %#x\n", ccb);
  if (!(ccb & 0b00010000)) BUG();
  ccb &= 0b10111100;
  outb(0x60, PS2_CMND_PORT);
  outb(ccb, PS2_DATA_PORT);
  outb(0xAA, PS2_CMND_PORT); // perform self test
  uint8_t data = inb(PS2_DATA_PORT);
  if (data != 0x55) BUG();
  printf("controller test ok\n");
  outb(0x60, PS2_CMND_PORT); // restore ccb
  outb(ccb, PS2_DATA_PORT);
  outb(0xAB, PS2_CMND_PORT); // test first port
  data = inb(PS2_DATA_PORT);
  if (data != 0x00) BUG();
  printf("port test ok\n");
  outb(0xAE, PS2_CMND_PORT); // enable first device (keyboard)
  ccb |= 0b00000001;
  outb(0x60, PS2_CMND_PORT); // and enable its IRQ
  outb(ccb, PS2_DATA_PORT);

  outb(0x20, PS2_CMND_PORT);
  printf("final PS/2 controller configuration: %#x\n", inb(PS2_DATA_PORT));

  // setup intr handler and unmask IRQ
  set_interrupt_handler(INTR_VECTOR_OFFSET + 1, kbd_interrupt_handler, PL_USER);
  mask_irq(1, false);
}

// str is null-terminated
void keyboard_data(char *str)
{
  (void)str;
  // TODO
}

void kbd_leds(unsigned char leds)
{
  (void)leds;
  // TODO
}

// https://wiki.osdev.org/PS/2_Keyboard
void kbd_interrupt(void)
{
  const int scancode = inb(PS2_DATA_PORT);
  printf(" --[[ %#x ]]-- ", scancode);
  do_scancode(scancode);
  acknowledge_interrupt(1);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/
