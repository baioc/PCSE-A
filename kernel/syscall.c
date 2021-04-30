/*
 * syscall.c
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
#include "stddef.h"
#include "string.h"

#include "console.h"
#include "clock.h"
#include "process.h"
#include "mqueue.h"
#include "sem.h"
#include "shm.h"

extern void syscall_handler(void);

/*******************************************************************************
 * Internal function
 ******************************************************************************/

// "Bubble-wrap" for user-originated calls, eg in order to sanitize addresses.

static int sys_cons_write(const char *str, long size)
{
  if (!access_ok((uint32_t)str, size)) return 0;
  return cons_write(str, size);
}

static unsigned long sys_cons_read(char *string, unsigned long lenght)
{
  if (!access_ok((uint32_t)string, lenght)) return 0;
  return cons_read(string, lenght);
}

static void sys_cons_echo(int on)
{
  return cons_echo(on);
}

static void sys_clock_settings(unsigned long *quartz, unsigned long *ticks)
{
  if (!access_ok((uint32_t)quartz, sizeof(unsigned long)) ||
      !access_ok((uint32_t)ticks, sizeof(unsigned long)))
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
  if (!access_ok((uint32_t)name, strlen(name) + 1)) return -13;
  return start(name, ssize, prio, arg);
}

static int sys_waitpid(int pid, int *retvalp)
{
  if (retvalp != NULL && !access_ok((uint32_t)retvalp, sizeof(int))) return -13;
  return waitpid(pid, retvalp);
}

static int sys_pcount(int fid, int *count)
{
  if (count != NULL && !access_ok((uint32_t)count, sizeof(int))) return -13;
  return pcount(fid, count);
}

static int sys_pcreate(int count)
{
  return pcreate(count);
}

static int sys_pdelete(int fid)
{
  return pdelete(fid);
}

static int sys_preceive(int fid, int *message)
{
  if (message != NULL && !access_ok((uint32_t)message, sizeof(int))) return -13;
  return preceive(fid, message);
}

static int sys_preset(int fid)
{
  return preset(fid);
}

static int sys_psend(int fid, int message)
{
  return psend(fid, message);
}

static int sys_scount(int sem)
{
  return scount(sem);
}

static int sys_screate(short int count)
{
  return screate(count);
}

static int sys_sdelete(int sem)
{
  return sdelete(sem);
}

static int sys_sreset(int sem, short int count)
{
  return sreset(sem, count);
}

static int sys_signal(int sem)
{
  return signal(sem);
}

static int sys_signaln(int sem, short int count)
{
  return signaln(sem, count);
}

static int sys_try_wait(int sem)
{
  return try_wait(sem);
}

static int sys_wait(int sem)
{
  return wait(sem);
}

static void *sys_shm_create(const char *key)
{
  if (!access_ok((uint32_t)key, strlen(key) + 1)) return NULL;
  return shm_create(key);
}

static void *sys_shm_acquire(const char *key)
{
  if (!access_ok((uint32_t)key, strlen(key) + 1)) return NULL;
  return shm_acquire(key);
}

static void sys_shm_release(const char *key)
{
  if (!access_ok((uint32_t)key, strlen(key) + 1)) return;
  shm_release(key);
}

static void *sys_sbrk(ptrdiff_t increment)
{
  return _sbrk(increment);
}

static void sys_ps(void)
{
  ps();
}

static void sys_sinfo(void)
{
  sinfo();
}

static void sys_pinfo(void)
{
  pinfo();
}

static void sys_disown(int pid)
{
  disown(pid);
}

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define SYSCALL_ENTRY(FN) [NR_##FN] = sys_##FN

#define TOTAL_SYSCALLS (sizeof(syscalls) / sizeof(syscalls[0]))

/*******************************************************************************
 * Types
 ******************************************************************************/

typedef int (*syscall_fn_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static const void *syscalls[] = {
    SYSCALL_ENTRY(cons_write),
    SYSCALL_ENTRY(cons_echo),
    SYSCALL_ENTRY(cons_read),
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
    SYSCALL_ENTRY(pcount),
    SYSCALL_ENTRY(pcreate),
    SYSCALL_ENTRY(pdelete),
    SYSCALL_ENTRY(preceive),
    SYSCALL_ENTRY(preset),
    SYSCALL_ENTRY(psend),
    SYSCALL_ENTRY(scount),
    SYSCALL_ENTRY(screate),
    SYSCALL_ENTRY(sdelete),
    SYSCALL_ENTRY(sreset),
    SYSCALL_ENTRY(signal),
    SYSCALL_ENTRY(signaln),
    SYSCALL_ENTRY(try_wait),
    SYSCALL_ENTRY(wait),
    SYSCALL_ENTRY(shm_create),
    SYSCALL_ENTRY(shm_acquire),
    SYSCALL_ENTRY(shm_release),
    SYSCALL_ENTRY(sbrk),
    SYSCALL_ENTRY(disown),
    SYSCALL_ENTRY(ps),
    SYSCALL_ENTRY(sinfo),
    SYSCALL_ENTRY(pinfo),
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
  return ((syscall_fn_t)syscalls[eax])(ebx, ecx, edx, esi, edi);
}
