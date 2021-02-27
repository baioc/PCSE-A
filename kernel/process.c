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
#include "cpu.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define STACK_SIZE 1024

/*******************************************************************************
 * Types
 ******************************************************************************/

// Describe different states of a process
typedef enum proc_state {
  CHOSEN, // process currently running on processor
  READY,  // process waiting for his turn
  BLOCKED_ON_SEMAHPORE,
  BLOCKED_ON_IO,
  WAITING_FOR_CHILD,
  SLEEPING,
  ZOMBIE
} proc_state;

// Used to save context of process
typedef struct context {
  int ebx;
  int esp;
  int ebp;
  int esi;
  int edi;
} context;

// Describe a process
typedef struct proc {
  int           pid;      // between 1 and NBPROC
  int           priority; // between 1 and MAXPRIO
  proc_state    state;
  context       ctx;
  int           ssize;
  const char *  name;
  void *        arg;
  link          position; // useful for the list
  int *         kernel_stack;
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
int tstC();

/*
 * Changes context between two processes
 */
extern void ctx_sw(context *ctx1, context *ctx2);

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
int nbr_proc = 0;

// Table of ALL processes. A process is referenced in this table by its pid - 1
// Because pids are numbered from 1 to NBPROC
proc process_table[NBPROC];

// List of activable processes
link list_proc = LIST_HEAD_INIT(list_proc);

// Current process running on processor
proc *chosen_process;

char *char_a = "A";
char *char_b = "B";

/*******************************************************************************
 * Public function
 ******************************************************************************/

/*
 * First draft of scheduling function
 */
void schedule()
{
  // necessary if we want to test the clock without initializing the processes
  if(chosen_process == NULL) return;

  if (proc_list_top()->priority < chosen_process->priority) return;

  proc* pass = chosen_process;  // process who will give the execution
  proc* take = proc_list_out(); // process who will take the execution

  pass->state = READY;
  take->state = CHOSEN;
  chosen_process = take;

  proc_list_add(pass);
  ctx_sw(&pass->ctx, &take->ctx);
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
  (void)ssize;
  assert(prio > 0 && prio <= MAXPRIO);

  // Too many processes
  if (nbr_proc == NBPROC) {
    return -1;
  }

  proc *new_proc = process_table + nbr_proc;
  new_proc->pid = ++nbr_proc;
  new_proc->priority = prio;

  // Allocate memory for stack
  new_proc->kernel_stack = mem_alloc(STACK_SIZE * sizeof(int));
  // Place the main function of the process at the top of kernel_stack
  // Along with its arguments
  new_proc->kernel_stack[STACK_SIZE - 3] = (int)(pt_func);
  // TODO return address must be positionned at STACK_SIZE - 2
  new_proc->kernel_stack[STACK_SIZE - 1] = (int)(arg);
  // Initialize kernel_stack pointer to the top of kernel_stack
  new_proc->ctx.esp = (int)(&(new_proc->kernel_stack[STACK_SIZE - 3]));

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

  proc_idle->kernel_stack = 0;

  proc_idle->name = "idle";
  proc_idle->state = CHOSEN;
  proc_idle->arg = 0;

  // Initialize chosen_process pointer
  chosen_process = proc_idle;

  // Add two other processes
  if (start(tstA, 256, 2, "tstA", char_a) < 0) {
    printf("Error creating process A");
  }
  if (start(tstB, 256, 2, "tstB", char_b) < 0) {
    printf("Error creating process B");
  }

  if (start(tstC, 256, 2, "tstC", 0) < 0) {
    printf("Error creating process C");
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
int chprio(int pid, int newprio)
{
  // process referenced by that pid doesn't exist, or newprio is invalid
  if (pid > nbr_proc || newprio < 1 || newprio > MAXPRIO) {
    return -1;
  }

  proc *proc = process_table + (pid - 1);
  int   old_prio = proc->priority;

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
int getprio(int pid)
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
    sti();
    hlt();
    cli();
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
    sti();
    hlt();
    cli();
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
    sti();
    hlt();
    cli();
  }

  return 0;
}

/*
 * Main function of process C
 */
int tstC()
{
  while (1) {
    printf("C");
    sti();
    hlt();
    cli();
  }

  return 0;
}
