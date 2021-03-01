/*
 * process.c
 *
 *  Created on: 11/02/2021
 *      Authors: Antoine Briançon, Thibault Cantori, baioc
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "process.h"

#include "debug.h"
#include "queue.h"

#include "malloc.c"

/*******************************************************************************
 * Macros
 ******************************************************************************/

// Total number of kenel processes.
#define NBPROC 1000

// Kernel stack size.
#define STACK_SIZE 512

/*******************************************************************************
 * Types
 ******************************************************************************/

// Describe different states of a process
enum proc_state {
  ACTIVE, // process currently running on processor
  READY,  // process waiting for his turn
  ZOMBIE, // terminated but still in use
  DEAD,   // marks a free process slot
};

// NOTE: must be kept in sync with ctx_sw
struct context {
  unsigned ebx;
  unsigned esp;
  unsigned ebp;
  unsigned esi;
  unsigned edi;
};

struct proc {
  int             pid;
  enum proc_state state;
  char *          name;
  struct context  ctx; // execution context registers
  unsigned *      kernel_stack;
  int             priority;
  union {
    link         queue; // doubly-linked list node used by queues
    struct proc *next;  // singly-linked list pointer
  } node;
  int retval;
};

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/// Changes context between two processes, defined in ctx_sw.S
extern void ctx_sw(struct context *old, struct context *new);

/// Defined in ctx_sw.S, this is called when a process implicitly exits.
extern void proc_exit(void);

// Main function of the idle process.
static int idle(void);

/**
 * Kills a process (even if it is a zombie) once and for all.
 * This means freeing any resources it owns and moving it to the free list, so
 * make sure it has already been deleted from the queue.
 */
static void proc_free(struct proc *proc);

// Adds process into priority queue.
static void proc_list_add(struct proc *proc);

// Removes process from the queue it's currently in.
static void proc_list_del(struct proc *proc);

// References the highest-priority process in a queue.
static struct proc *proc_list_top(void);

// Pops the highest-priority process from the priority queue.
static struct proc *proc_list_out(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

// Table of ALL processes, indexed by their pid.
static struct proc process_table[NBPROC + 1] = {0};

// Current process running on processor.
static struct proc *current_process = NULL;

// Process priority queue.
static link process_queue;

// Process disjoint lists.
static struct proc *free_procs = NULL;
static struct proc *zombie_procs = NULL;

/*******************************************************************************
 * Public function
 ******************************************************************************/

void process_init(void)
{
  // set up initial kernel process, idle, which has pid 0
  struct proc *proc_idle = &process_table[0];
  *proc_idle = (struct proc){.name = "idle", .pid = 0, .priority = 0};

  // initialize process queue and lists
  process_queue = (link)LIST_HEAD_INIT(process_queue);
  zombie_procs = NULL;
  free_procs = NULL;
  for (int i = NBPROC; i >= 1; --i) { // all other procs begin dead
    struct proc *proc = &process_table[i];
    proc->state = DEAD;
    proc->node.next = free_procs;
    free_procs = proc;
  }

  // idle is the first process to run
  current_process = proc_idle;
  proc_idle->state = ACTIVE;
  idle();
}

void schedule(void)
{
  // process that's passing the cpu over
  struct proc *pass = current_process;
  assert(pass != NULL);

  // check if there are zombies to bury
  while (zombie_procs != NULL) {
    struct proc *zombie = zombie_procs;
    zombie_procs = zombie->node.next;
    proc_free(zombie);
  }

  switch (pass->state) {
  // put it back in the priority queue if it's still active
  case ACTIVE:
    pass->state = READY;
    proc_list_add(pass);
    break;

  // we'll only find a zombie proc here when it has just exited
  case ZOMBIE:
    // we cannot free the context we're in, so delay that operation to next time
    pass->node.next = zombie_procs;
    zombie_procs = pass;
    break;

  case READY:
  case DEAD: // unreachable
    assert(false);
  }

  // process that will take control of the execution
  assert(!queue_empty(&process_queue));
  struct proc *take = proc_list_out();
  current_process = take;
  take->state = ACTIVE;

  // hand the cpu over to the newly scheduled process
  ctx_sw(&pass->ctx, &take->ctx);
}

int start(int (*pt_func)(void *), unsigned long ssize, int prio,
          const char *name, void *arg)
{
  assert(pt_func != NULL);
  (void)ssize;
  if (prio < 1 || prio > MAXPRIO) return -1;
  assert(name != NULL);

  // bail when we can't find a free slot in the process table
  struct proc *new_proc = free_procs;
  if (new_proc == NULL) return -1;
  free_procs = new_proc->node.next;

  // otherwise set-up its pid (implicit) and priority (explicit)
  new_proc->pid = new_proc - process_table; // calculate index from pointer
  new_proc->priority = prio;

  // copy name
  new_proc->name = mem_alloc((strlen(name) + 1) * sizeof(char));
  if (new_proc->name == NULL) {
    free_procs = new_proc;
    return -1;
  }
  strcpy(new_proc->name, name);

  // allocate stack
  new_proc->kernel_stack = mem_alloc(STACK_SIZE * sizeof(int));
  if (new_proc->kernel_stack == NULL) {
    mem_free(new_proc->name, (strlen(new_proc->name) + 1) * sizeof(char));
    free_procs = new_proc;
    return -1;
  }

  // setup stack with the given arg, termination code and main function
  new_proc->kernel_stack[STACK_SIZE - 1] = (unsigned)(arg);
  new_proc->kernel_stack[STACK_SIZE - 2] = (unsigned)(proc_exit);
  new_proc->kernel_stack[STACK_SIZE - 3] = (unsigned)(pt_func);
  new_proc->ctx.esp = (unsigned)(&(new_proc->kernel_stack[STACK_SIZE - 3]));

  // this process is now ready to run
  new_proc->state = READY;
  proc_list_add(new_proc);

  // check if the new process should be run immediately or not
  if (current_process->pid != 0 &&
      new_proc->priority > current_process->priority) {
    schedule();
  }

  return new_proc->pid;
}

int chprio(int pid, int newprio)
{
  // process referenced by that pid doesn't exist, or newprio is invalid
  if (pid < 1 || pid > NBPROC || newprio < 1 || newprio > MAXPRIO) return -1;

  struct proc *proc = &process_table[pid];
  if (proc->state == DEAD) return -1;

  const int old_prio = proc->priority;
  if (newprio == old_prio) return old_prio; // change priority only if needed

  proc->priority = newprio;

  // Priority of an activable process has been changed
  if (proc != current_process) {
    assert(proc->state == READY);
    proc_list_del(proc); // Remove process from its current queue
    proc_list_add(proc); // Place it again with the updated priority
    if (proc->priority > current_process->priority) schedule();

    // Priority of running process changed and it shouldn't be running anymore
  } else if (current_process->priority < proc_list_top()->priority) {
    assert(proc->state == ACTIVE);
    schedule();
  }

  return old_prio;
}

int getprio(int pid)
{
  if (pid < 1 || pid > NBPROC) return -1;
  struct proc *proc = &process_table[pid];
  if (proc->state == DEAD) return -1;
  return proc->priority;
}

int getpid(void)
{
  return current_process->pid;
}

void exit(int retval)
{
  // store exit code for later
  current_process->retval = retval;

  // zombify current process and yield
  current_process->state = ZOMBIE;
  schedule();

  for (;;) { // ensures gcc marks exit as `noreturn`
  }
}

int kill(int pid)
{
  if (pid < 1 || pid > NBPROC) return -1;
  struct proc *proc = &process_table[pid];
  switch (proc->state) {
  case DEAD:
    return -1; // invalid pid
  case ZOMBIE:
    break; // can't kill what's already dead
  case ACTIVE:
    exit(-1); // current process just killed itself... lets just exit
    break;
  case READY:
    // when a proc isn't running, we can kill and bury it directly
    proc_list_del(proc);
    proc_free(proc);
    break;
  }
  return 0;
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

static inline void proc_free(struct proc *proc)
{
  // free resources
  mem_free(proc->kernel_stack, STACK_SIZE * sizeof(int));
  mem_free(proc->name, (strlen(proc->name) + 1) * sizeof(char));

  // add it to the free list
  proc->state = DEAD;
  proc->node.next = free_procs;
  free_procs = proc;
}

// TODO: clear up test procs created in idle()
static int test(void *arg)
{
  const char c = *(char *)arg;
  for (int i = 0; i < 5; ++i) {
    printf("%c", c);
    schedule();
  }
  return c;
}

static int idle(void)
{
  // Add two other processes
  if (start(test, 256, 2, "tstA", "A") < 0) {
    printf("Error creating process A\n");
  }
  if (start(test, 256, 2, "tstB", "B") < 0) {
    printf("Error creating process B\b");
  }
  if (start(test, 256, 2, "tstC", "C") < 0) {
    printf("Error creating process C\n");
  }

  while (1) {
    printf(".");
    schedule();
  }

  return 0;
}

static inline void proc_list_add(struct proc *proc)
{
  queue_add(proc, &process_queue, struct proc, node.queue, priority);
}

static inline struct proc *proc_list_out(void)
{
  return queue_out(&process_queue, struct proc, node.queue);
}

static inline void proc_list_del(struct proc *proc)
{
  queue_del(proc, node.queue);
}

static inline struct proc *proc_list_top(void)
{
  return queue_top(&process_queue, struct proc, node.queue);
}
