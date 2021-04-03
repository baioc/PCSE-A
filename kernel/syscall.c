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
#include "mem.h"
#include "string.h"

#include "console.h"
#include "clock.h"
#include "process.h"

extern void syscall_handler(void);

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define SYSCALL_ENTRY(FN) [NR_##FN] = sys_##FN

#define TOTAL_SYSCALLS (sizeof(syscalls) / sizeof(syscalls[0]))

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

static int  sys_cons_write(const char *str, long size);
static void sys_clock_settings(unsigned long *quartz, unsigned long *ticks);
static unsigned long sys_current_clock(void);
static void          sys_wait_clock(unsigned long clock);
static int           sys_chprio(int pid, int newprio);
static void          sys_exit(int retval);
static int           sys_getpid(void);
static int           sys_getprio(int pid);
static int           sys_kill(int pid);
static int           sys_start(const char *name, unsigned long ssize, int prio,
                               void *arg);
static int           sys_waitpid(int pid, int *retvalp);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static const void *syscalls[] = {
    SYSCALL_ENTRY(cons_write),
    SYSCALL_ENTRY(clock_settings),
    SYSCALL_ENTRY(current_clock),
    SYSCALL_ENTRY(wait_clock),
    SYSCALL_ENTRY(chprio),
    SYSCALL_ENTRY(exit),
    SYSCALL_ENTRY(getpid),
    SYSCALL_ENTRY(getprio),
    SYSCALL_ENTRY(kill),
    SYSCALL_ENTRY(start),
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

// "Bubble-wrap" for user-originated calls, eg in order to sanitize addresses.

static int sys_cons_write(const char *str, long size)
{
  if (size == 0 || !access_ok((uint32_t)str, size - 1)) return 0;
  return cons_write(str, size);
}

static void sys_clock_settings(unsigned long *quartz, unsigned long *ticks)
{
  if (!access_ok((uint32_t)quartz, sizeof(unsigned long) - 1) ||
      !access_ok((uint32_t)ticks, sizeof(unsigned long) - 1))
  {
    return;
  }
  clock_settings(quartz, ticks);
}

static unsigned long sys_current_clock(void)
{
  return current_clock();
}

static void sys_wait_clock(unsigned long clock)
{
  wait_clock(clock);
}

static int sys_chprio(int pid, int newprio)
{
  return chprio(pid, newprio);
}

static void sys_exit(int retval)
{
  exit(retval);
}

static int sys_getpid(void)
{
  return getpid();
}

static int sys_getprio(int pid)
{
  return getprio(pid);
}

static int sys_kill(int pid)
{
  return kill(pid);
}

static int sys_start(const char *name, unsigned long ssize, int prio, void *arg)
{
  if (!access_ok((uint32_t)name, strlen(name))) return -13;
  return start(name, ssize, prio, arg);
}

static int sys_waitpid(int pid, int *retvalp)
{
  if (retvalp != NULL && !access_ok((uint32_t)retvalp, sizeof(int) - 1)) {
    return -13;
  }
  return waitpid(pid, retvalp);
}
