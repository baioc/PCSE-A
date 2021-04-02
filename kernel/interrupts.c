/*
 * interrupts.c
 *
 *  Created on: 11/02/2021
 *      Author: baioc
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "interrupts.h"

#include "stddef.h"
#include "stdint.h"
#include "debug.h"
#include "cpu.h"
#include "processor_structs.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

// see https://wiki.osdev.org/8259_PIC
#define PIC_MASTER_CMND_PORT 0x20
#define PIC_MASTER_DATA_PORT 0x21
#define PIC_SLAVE_CMND_PORT  0xA0
#define PIC_SLAVE_DATA_PORT  0xA1
#define PIC_CMND_EOI         0x20

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

void set_interrupt_handler(int num, void (*handler)(void), unsigned char pl)
{
  assert(num != 1 && num != 15 && (num < 20 || num > 31)); // intel-reserved
  assert((unsigned)num < (sizeof(idt) / sizeof(idt[0])));
  assert(handler != NULL);
  const uint32_t addr = (uint32_t)handler;

  // see https://wiki.osdev.org/Interrupt_Descriptor_Table
  const struct {
    uint16_t lower_addr;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  attributes;
    uint16_t upper_addr;
  } entry = {
      .upper_addr = (addr & 0xFFFF0000) >> 16,
      .lower_addr = addr & 0x0000FFFF,
      .selector = KERNEL_CS,
      .attributes = 0x80 | ((pl & 0b11) << 5) | 0x0E,
  };

  idt[num] = *((uint64_t *)&entry);
}

void mask_irq(int irq, bool mask)
{
  assert(irq >= 0);
  assert(irq <= 15);

  // check which port to use depending on irq line
  uint16_t port;
  if (irq <= 7) {
    port = PIC_MASTER_DATA_PORT;
  } else {
    port = PIC_SLAVE_DATA_PORT;
    irq -= 8;
  }

  // mask/unmask requested bit in the interrupt register
  const uint8_t current_mask = inb(port);
  outb(mask ? current_mask | (1 << irq) : current_mask & ~(1 << irq), port);
}

void acknowledge_interrupt(int irq)
{
  // send EOI to one or both PIC chips
  if (irq >= 8) outb(PIC_CMND_EOI, PIC_SLAVE_CMND_PORT);
  outb(PIC_CMND_EOI, PIC_MASTER_CMND_PORT);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/
