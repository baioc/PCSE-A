/*
 * process.h
 *
 *  Created on: 11/02/2021
 *      Authors: Antoine Briançon, Thibault Cantori, baioc
 */

#ifndef _process_H_
#define _process_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "stdint.h"
#include "stddef.h"
#include "debug.h"
#include "kernel_tests.h"
#include "cpu.h"
#include "queue.h"
#include "mem.h"
#include "string.h"
#include "stdbool.h"
#include "clock.h"
#include "console.h"
#include "sem.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/// Max scheduling priority (min is 1).
#define MAXPRIO 256

/// Scheduling frequency in Hz, meaning a quantum is 1/SCHEDFREQ seconds.
#define SCHEDFREQ 50

// Total number of kenel processes.
#define NBPROC 1000

// Kernel stack size, in words.
#define STACK_SIZE 512

/*******************************************************************************
 * Types
 ******************************************************************************/
 // Describe different states of a process
 typedef enum _proc_state {
   DEAD,           // marks a free process slot
   BLOCKED,        // process is blocked by a semaphore
   ZOMBIE,         // terminated but still in use
   SLEEPING,       // process is waiting on its alarm
   AWAITING_CHILD, // process is waiting for one of its children
   AWAITING_IO,    // process is waiting for and input/output
   READY,          // process waiting for his turn
   ACTIVE,         // process currently running on processor
 } proc_state;

 // NOTE: must be kept in sync with ctx_sw()
 struct _context {
   unsigned ebx;
   unsigned esp;
   unsigned ebp;
   unsigned esi;
   unsigned edi;
 };
 typedef struct _context context;

 struct _proc {
   int             pid;
   proc_state      state;
   char *          name;
   context         ctx; // execution context registers
   unsigned *      kernel_stack;
   int             priority; // scheduling priority
   union {
     unsigned long quantum; // remaining cpu time (in ticks) for this proc
     unsigned long alarm;   // timestamp (in ticks) to wake a sleeping process
   } time;
   link         node; // doubly-linked node used by lists
   struct _proc *parent;
   link         children;
   link         siblings;
   link         blocked; // if blocked by a semaphore
   int          retval;
   int          m_queue_fid; // fid in which the process if sending/receving messages
   int          m_queue_rd_send; // if blocked on a reseted or deleted message queue while sending
   int          m_queue_rd_receive; // if blocked on a reseted or deleted message queue while receiving
 };
 typedef struct _proc proc;
/*******************************************************************************
 * Variables
 ******************************************************************************/

 // Process disjoint lists.
 link free_procs;
 link sleeping_procs;
 link ready_procs;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/// Initializes the process management subsystem and moves to process "idle".
void process_init(void);

/// Ticks the current process time, may end up calling the scheduler.
void process_tick(void);

/**
 * Makes the current process yield the CPU to the scheduler.
 * NOTE: this routine supposes interrupts are disabled and will re-enable them
 * when switching context back to user space.
 */
void schedule(void);

/*
 * Create a process
 * pt_func : main function of process
 * ssize : size of stack
 * prio : priority of process for execution
 * name : name of process, must be null-terminated
 * arg : arguments passed to the main function pt_func
 */
int start(int (*pt_func)(void *), unsigned long ssize, int prio,
          const char *name, void *arg);

/// Terminates the current process with the given exit code.
void exit(int retval);

/// Kills the process with the given pid, returning 0 on success.
int kill(int pid);

/*
 * Change priority of process referenced by pid to the value newprio
 * If priority changed and the process was in a queue, it needs to be placed
 * again in that queue depending on its new priority.
 * If the value of newprio is invalid, return value must be < 0. Otherwise,
 * return value is the previous priority of process
 */
int chprio(int pid, int newprio);

/*
 * If value of pid is invalid, return value must be < 0. Otherwise, return value
 * is the current priority of process referenced by pid
 */
int getprio(int pid);

/*
 * Returns pid of the calling process.
 */
int getpid(void);

/// Makes the current process yield the CPU for at least TICKS jiffies.
void sleep(unsigned long ticks);

/**
 * Waits for a child process to finish and reads its return value.
 *
 * This procedure may block while waiting for a child process with the specified
 * PID (a negative value here means any child will do) or it may return directly
 * in case the given PID is invalid or there are no children to wait for.
 * In the first case the return value is the pid of the child that woke this
 * process up and in the second one it is a strictly negative value.
 *
 * When RETVALP is not null and a child process was successfully awaited on,
 * the value it points to will be overwritten with the child's exit code.
 */
int waitpid(int pid, int *retvalp);

proc* get_current_process();

#endif /* _process_H_ */
