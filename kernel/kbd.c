/*
 * kbd.c
 *
 *  Created on: 09/04/2021
 *      Author: baioc
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "kbd.h"
#include "console.h"

#include "interrupts.h"
#include "cpu.h"
#include "stdbool.h"
#include "stdio.h"
#include "mqueue.h"
#include "debug.h"

extern void kbd_interrupt_handler(void);

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define PS2_CMND_PORT 0x64
#define PS2_DATA_PORT 0x60

#define STDIN_BUFFER_SIZE 21

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

static void do_echo(char c);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static bool echo;
static int  stdin; // global message queue

/*******************************************************************************
 * Public function
 ******************************************************************************/

void kbd_init(void)
{
  // create "stdin"
  echo = true;
  stdin = pcreate(STDIN_BUFFER_SIZE);
  assert(stdin >= 0);

  // setup intr handler and unmask IRQ
  set_interrupt_handler(INTR_VECTOR_OFFSET + 1, kbd_interrupt_handler, PL_USER);
  mask_irq(1, false);
}

// https://wiki.osdev.org/PS2_Keyboard
void kbd_interrupt(void)
{
  const unsigned code = inb(PS2_DATA_PORT);
  acknowledge_interrupt(1);
  do_scancode(code); // calls keyboard_data() and kbd_leds()
}

void keyboard_data(char *str)
{
  // XXX: as per the spec, we have to store kbd data even if no one is reading
  for (; *str != '\0'; str++) {
    // drop characters in case the buffer is full (we don't want to block)
    int count;
    int err = pcount(stdin, &count);
    assert(!err);
    if (count >= STDIN_BUFFER_SIZE) return;
    // otherwise send each individual character to stdin
    err = psend(stdin, (int)*str);
    assert(!err);
  }
}

void kbd_leds(unsigned char leds)
{
  outb(0xED, PS2_CMND_PORT);
  outb(leds, PS2_DATA_PORT);
}

unsigned long cons_read(char *string, unsigned long length)
{
  unsigned long idx = 0;
  while (idx < length) {
    int       tmp; // int because that's what mqueue uses
    const int err = preceive(stdin, &tmp);
    assert(!err);

    // echo if enabled
    const char c = (char)tmp;
    if (echo) do_echo(c);

    // check for special cases
    if (c == '\r')
      break;
    else if (c == 127)
      idx = idx > 0 ? idx - 1 : idx;
    else
      string[idx++] = c;
  }
  return idx;
}

void cons_echo(int on)
{
  echo = on != 0;
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

static void do_echo(char c)
{
  if (c == 9 || (c >= 32 && c <= 126))
    printf("%c", c);
  else if (c == '\r')
    printf("\n");
  else if (c < 32)
    printf("^%c", 64 + c);
  else if (c == 127)
    printf("\b \b"); // NOTE: this doesn't work across lines
  else
    return;
}
