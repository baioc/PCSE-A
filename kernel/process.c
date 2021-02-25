/*
 * process.c
 *
 *  Created on: 11/02/2021
 *      Authors: Antoine Briançon, Thibault Cantori
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "process.h"
#include "queue.h"
#include "malloc.c"
#include "stdint.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

// Describe different states of a process
typedef enum _proc_state {
  CHOSEN, // process currently running on processor
  READY,  // process waiting for his turn
  BLOCKED_ON_SEMAHPORE,
  BLOCKED_ON_IO,
  WAITING_FOR_CHILD,
  SLEEPING,
  ZOMBIE
} proc_state;

// Describe a process
typedef struct _proc {
  uint32_t      pid;      // between 1 and NBPROC
  uint32_t      priority; // between 1 and MAXPRIO
  proc_state    state;
  uint32_t      save_zone[5]; // used to save context of process
  uint32_t      ssize;
  const char *  name;
  void *        arg;
  link          position; // useful for the list
  uint32_t *    stack;
  struct _proc *parent; // process which created this process (phase 3)
  struct _proc
      *children; // list of processes that this process created (phase 3)
} proc;

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

int idle();
int tstA();
int tstB();

/*
 * Changes context between two processes
 */
extern void ctx_sw(uint32_t save_zone1[5], uint32_t save_zone2[5]);

/*
 * Add process into activable processes list
 */
void proc_list_add(proc *proc_to_add);

/*
 * Remove process from activable processes list
 */
void proc_list_del(proc *proc_to_del);

/*
 * Return first process from activable processes list
 */
proc *proc_list_top();

/*
 * Remove first process from activable processes list
 */
proc *proc_list_out();

/*******************************************************************************
 * Variables
 ******************************************************************************/

// Current number of processes started on system
uint32_t nbr_proc = 0;

// Table of ALL processes. A process is referenced in this table by its pid - 1
// Because pids are numbered from 1 to NBPROC
proc process_table[NBPROC];

// List of activable processes
link list_proc = LIST_HEAD_INIT(list_proc);

// Current process running on processor
proc *chosen_process;

/*******************************************************************************
 * Public function
 ******************************************************************************/

/*
 * First draft of scheduling function
 */
void schedule()
{
  proc *pass = chosen_process;  // process who will give the execution
  proc *take = proc_list_out(); // process who will take the execution

  pass->state = READY;
  take->state = CHOSEN;
  chosen_process = take;

  proc_list_add(pass);
  ctx_sw(pass->save_zone, take->save_zone);
}

/*
 * Create a process
 * pt_func : main function of process
 * ssize : size of stack
 * prio : priority of process for execution
 * name : name of process
 * arg : arguments passed to the main function pt_func
 */
int start(int (*pt_func)(void *), unsigned long ssize, int prio,
          const char *name, void *arg)
{
  assert(prio > 0 && prio <= MAXPRIO);

  // Too many processes
  if (nbr_proc == NBPROC) {
    return -1;
  }

  // Add space for kernel use of stack (main function and args)
  ssize += 2;

  proc *new_proc = process_table + nbr_proc;
  new_proc->pid = ++nbr_proc;
  new_proc->priority = prio;
  new_proc->ssize = (uint32_t)ssize;

  // Allocate memory for stack
  new_proc->stack = mem_alloc(ssize * sizeof(uint32_t));
  // Place the main function of the process at the top of stack
  // Along with its arguments
  new_proc->stack[ssize - 2] = (uint32_t)(pt_func);
  new_proc->stack[ssize - 1] = (uint32_t)(arg);
  // Initialize stack pointer to the top of stack
  new_proc->save_zone[1] = (uint32_t)(&(new_proc->stack[ssize - 2]));

  new_proc->name = name;
  new_proc->state = READY;
  new_proc->arg = arg;

  // Add process to the queue
  proc_list_add(new_proc);

  return new_proc->pid;
}

/*
 * Initialize the system with the main process "idle"
 * For now, also creates two other processes A and B
 */
void process_init()
{
  // Add a first special process idle
  proc *proc_idle = mem_alloc(sizeof(proc));

  proc_idle->pid = ++nbr_proc;
  proc_idle->priority = 1;
  proc_idle->ssize = 0;

  proc_idle->stack = 0;

  proc_idle->name = "idle";
  proc_idle->state = CHOSEN;
  proc_idle->arg = 0;

  // Initialize chosen_process pointer
  chosen_process = proc_idle;

  // Add two other processes
  if (start(tstA, 256, 2, "tstA", 0) < 0) {
    printf("Error creating process A");
  }
  if (start(tstB, 256, 2, "tstB", 0) < 0) {
    printf("Error creating process B");
  }

  // Then starts main process idle
  idle();
}

/*
 * Change priority of process referenced by pid to the value newprio
 * If priority changed and the process was in a queue, it needs to be placed
 * again in that queue depending on its new priority.
 * If the value of newprio is invalid, return value must be < 0. Otherwise,
 * return value is the previous priority of process
 */
int chprio(uint32_t pid, uint32_t newprio)
{
  // process referenced by that pid doesn't exist, or newprio is invalid
  if (pid > nbr_proc || newprio < 1 || newprio > MAXPRIO) {
    return -1;
  }

  proc *   proc = process_table + (pid - 1);
  uint32_t old_prio = proc->priority;

  // Priority must be changed
  if (proc->priority != newprio) {
    // Change process priority
    proc->priority = newprio;

    // Priority of an activable process has been changed
    if (chosen_process != proc) {
      // Remove process from activables processes list
      proc_list_del(proc);
      // Place it again in processes list
      proc_list_add(proc);

      if (newprio > chosen_process->priority) schedule();
    }
    // Priority of running process changed and shouldn't be running anymore
    else if (proc->priority < proc_list_top()->priority)
    {
      schedule();
    }
  }

  return old_prio;
}

/*
 * If value of pid is invalid, return value must be < 0. Otherwise, return value
 * is the current priority of process referenced by pid
 */
int getprio(uint32_t pid)
{
  // process referenced by that pid doesn't exist
  if (pid > nbr_proc) {
    return -1;
  }

  return process_table[pid - 1].priority;
}

/*
 * Returns pid of calling process
 */
int getpid(void)
{
  return chosen_process->pid;
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

/*
 * Add process into activable processes list
 */
void proc_list_add(proc *proc_to_add)
{
  queue_add(proc_to_add, &list_proc, proc, position, priority);
}

/*
 * Remove process from activable processes list
 */
void proc_list_del(proc *proc_to_del)
{
  queue_del(proc_to_del, position);
}

/*
 * Return first process from activable processes list
 */
proc *proc_list_top()
{
  return queue_top(&list_proc, proc, position);
}

/*
 * Remove first process from activable processes list
 */
proc *proc_list_out()
{
  return queue_out(&list_proc, proc, position);
}

/*
 * Main function of idle process
 */
int idle()
{
  while (1) {
    printf("idle");
    schedule();
  }

  return 0;
}

/*
 * Main function of process A
 */
int tstA()
{
  while (1) {
    printf("A");
    for (uint32_t i = 0; i < 5000000; i++)
      ;
    schedule();
  }

  return 0;
}

/*
 * Main function of process B
 */
int tstB()
{
  while (1) {
    printf("B");
    for (uint32_t i = 0; i < 5000000; i++)
      ;
    schedule();
  }

  return 0;
}
