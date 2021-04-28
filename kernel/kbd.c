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
#include "console.h"
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

 bool echo = true;

/*******************************************************************************
 * Public function
 ******************************************************************************/

// récupère des caractères, est réveillé par keyboard_data, est appelé par utilisateur
unsigned long cons_read(char *string, unsigned long length){
  if(length <= 0){
    return 0;
  }
  char a = string[1];
  return (int) a;
}

void cons_echo(int on){
  if(on == 0 || on == 1){
    echo = (bool) on;
  }
}

void kbd_init(void)
{
  // creates the message-queue used to pass the caracters from the keyboard

  // setup intr handler and unmask IRQ
  set_interrupt_handler(INTR_VECTOR_OFFSET + 1, kbd_interrupt_handler, PL_USER);
  mask_irq(1, false);
}

// gère cons_read et cons_echo, lance aussi printf si cons_echo est actif
void keyboard_data(char *str)
{
  if(echo){
    int cpt = 0;
    char c = str[cpt];
    while(c != '\0'){
      printf("%c", c);
      cpt ++;
      c = str[cpt];
    }
  }
  /*} if(un processus en attente){
    rempli la file
  }*/
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
