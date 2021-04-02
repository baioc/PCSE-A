/*
 * syscall.h
 *
 *  Created on: 01/04/2021
 *      Author: baioc
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "syscall.h"

#include "stdint.h"
#include "interrupts.h"

#include "console.h"
#include "clock.h"
#include "process.h"

extern void syscall_handler(void);

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define SYSCALL_ENTRY(FN) [NR_##FN] = FN

#define TOTAL_SYSCALLS (sizeof(syscalls) / sizeof(syscalls[0]))

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static const void *syscalls[] = {
    SYSCALL_ENTRY(console_putbytes),
    SYSCALL_ENTRY(chprio),
    SYSCALL_ENTRY(clock_settings),
    SYSCALL_ENTRY(current_clock),
    SYSCALL_ENTRY(exit),
    SYSCALL_ENTRY(getpid),
    SYSCALL_ENTRY(getprio),
    SYSCALL_ENTRY(kill),
    SYSCALL_ENTRY(start),
    SYSCALL_ENTRY(wait_clock),
    SYSCALL_ENTRY(waitpid),
};

/*******************************************************************************
 * Public function
 ******************************************************************************/

void syscall_init(void)
{
  set_interrupt_handler(NR_SYSCALL, syscall_handler, PL_USER);
}

int do_syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx,
               uint32_t esi, uint32_t edi)
{
  if (eax >= TOTAL_SYSCALLS) return -1;
  typedef int (*syscall_fn_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
  return ((syscall_fn_t)syscalls[eax])(ebx, ecx, edx, esi, edi);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/
