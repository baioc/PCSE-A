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
#include "cpu.h"
#include "clock.h"
#include "console.h"

#include "malloc.c"

/*******************************************************************************
 * Macros
 ******************************************************************************/

// Total number of kenel processes.
#define NBPROC 1000

// Kernel stack size.
#define STACK_SIZE 512

/// Scheduling quantum, in tick units.
#define QUANTUM (CLOCKFREQ / SCHEDFREQ)

// TODO: remove these when we actually have separated user and kernel spaces
#define COME_FROM_USERSPACE() \
  do {                        \
    cli();                    \
  } while (0)
#define RETURN_TO_USERSPACE(ret) \
  do {                           \
    sti();                       \
    return (ret);                \
  } while (0)

// Removes process pointed to by P from the priority queue it's currently in.
#define PROC_CLEAR_QUEUE(p) queue_del((p), node.queue)

// Adds process pointed to by P into the ready priority queue.
#define PROC_ENQUEUE_READY(p) \
  queue_add((p), &ready_procs, struct proc, node.queue, priority)

// Pops and returns the highest-priority process from the ready queue.
#define PROC_DEQUEUE_READY() (queue_out(&ready_procs, struct proc, node.queue))

// References the highest-priority process in a queue.
#define PROC_READY_TOP() (queue_top(&ready_procs, struct proc, node.queue))

// Adds process pointed to by P into the sleeping queue, sorted by its alarm.
#define PROC_ENQUEUE_SLEEPING(p) \
  queue_add((p), &sleeping_procs, struct proc, node.queue, time.alarm)

/*******************************************************************************
 * Types
 ******************************************************************************/

// Describe different states of a process
enum proc_state {
  ACTIVE,   // process currently running on processor
  READY,    // process waiting for his turn
  SLEEPING, // process is waiting on its alarm
  ZOMBIE,   // terminated but still in use
  DEAD,     // marks a free process slot
};

// NOTE: must be kept in sync with ctx_sw()
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
  int             priority; // scheduling priority
  union {
    unsigned long quantum; // remaining cpu time (in ticks) for this proc
    unsigned long alarm;   // timestamp (in ticks) to wake a sleeping process
  } time;
  union {
    link         queue; // doubly-linked list node used by priority queues
    struct proc *next;  // singly-linked list pointer used by zombie|free lists
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

/**
 * Makes the current process yield the CPU to the scheduler.
 * NOTE: this routine supposes interrupts are disabled and will re-enable them
 * when switching context back to user space.
 */
static void schedule(void);

// Enables interrupts and starts kernel idle process.
static void idle(void);

/**
 * Kills a process (even if it is a zombie) once and for all.
 * This means freeing any resources it owns and moving it to the free list, so
 * make sure it has already been deleted from the queue.
 */
static void proc_free(struct proc *proc);

/*******************************************************************************
 * Variables
 ******************************************************************************/

// Table of ALL processes, indexed by their pid.
static struct proc process_table[NBPROC + 1];

// Current process running on processor.
static struct proc *current_process = NULL;

// Process disjoint lists.
static link         ready_procs;
static struct proc *free_procs;
static struct proc *zombie_procs;
static link         sleeping_procs;

/*******************************************************************************
 * Public function
 ******************************************************************************/

void process_init(void) // only called from kernel space
{
  // set up initial kernel process, idle, which has pid 0
  struct proc *proc_idle = &process_table[0];
  *proc_idle = (struct proc){.name = "idle", .pid = 0, .priority = 0};

  // initialize process queue and lists
  ready_procs = (link)LIST_HEAD_INIT(ready_procs);
  sleeping_procs = (link)LIST_HEAD_INIT(sleeping_procs);
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
  proc_idle->time.quantum = 0;
  idle();
}

void process_tick(void)
{
  // assuming this is only called from ISRs, we're already in kernel space
  assert(current_process->state == ACTIVE);
  if (current_process->time.quantum == 0 ||
      --current_process->time.quantum == 0) {
    schedule();
  }
}

int start(int (*pt_func)(void *), unsigned long ssize, int prio,
          const char *name, void *arg)
{
  COME_FROM_USERSPACE();

  assert(pt_func != NULL);
  (void)ssize;
  if (prio < 1 || prio > MAXPRIO) RETURN_TO_USERSPACE(-1);
  assert(name != NULL);

  // bail when we can't find a free slot in the process table
  struct proc *new_proc = free_procs;
  if (new_proc == NULL) RETURN_TO_USERSPACE(-1);
  free_procs = new_proc->node.next;

  // otherwise set-up its pid (implicit) and priority (explicit)
  new_proc->pid = new_proc - process_table; // calculate index from pointer
  new_proc->priority = prio;

  // copy name
  new_proc->name = mem_alloc((strlen(name) + 1) * sizeof(char));
  if (new_proc->name == NULL) {
    free_procs = new_proc;
    RETURN_TO_USERSPACE(-1);
  }
  strcpy(new_proc->name, name);

  // allocate stack
  new_proc->kernel_stack = mem_alloc(STACK_SIZE * sizeof(int));
  if (new_proc->kernel_stack == NULL) {
    mem_free(new_proc->name, (strlen(new_proc->name) + 1) * sizeof(char));
    free_procs = new_proc;
    RETURN_TO_USERSPACE(-1);
  }

  // setup stack with the given arg, termination code and main function
  new_proc->kernel_stack[STACK_SIZE - 1] = (unsigned)(arg);
  new_proc->kernel_stack[STACK_SIZE - 2] = (unsigned)(proc_exit);
  new_proc->kernel_stack[STACK_SIZE - 3] = (unsigned)(pt_func);
  new_proc->ctx.esp = (unsigned)(&(new_proc->kernel_stack[STACK_SIZE - 3]));

  // this process is now ready to run
  new_proc->state = READY;
  new_proc->time.quantum = QUANTUM;
  PROC_ENQUEUE_READY(new_proc);

  // check if the new process should be run immediately or not
  if (current_process->pid != 0 &&
      new_proc->priority > current_process->priority) {
    schedule();
  }

  const int pid = new_proc->pid;
  RETURN_TO_USERSPACE(pid);
}

int chprio(int pid, int newprio)
{
  COME_FROM_USERSPACE();

  // process referenced by that pid doesn't exist, or newprio is invalid
  if (pid < 1 || pid > NBPROC || newprio < 1 || newprio > MAXPRIO) {
    RETURN_TO_USERSPACE(-1);
  }

  struct proc *proc = &process_table[pid];
  if (proc->state == DEAD) RETURN_TO_USERSPACE(-1);

  const int old_prio = proc->priority;
  if (newprio == old_prio)
    RETURN_TO_USERSPACE(old_prio); // change priority only if needed

  proc->priority = newprio;

  switch (proc->state) {
  case ACTIVE: // check whether current process shouldn't be running anymore
    if (PROC_READY_TOP() != NULL &&
        current_process->priority < PROC_READY_TOP()->priority)
    {
      schedule();
    }
    break;
  case READY:
    PROC_CLEAR_QUEUE(proc);   // remove process from the ready queue
    PROC_ENQUEUE_READY(proc); // place it again with the updated priority
    if (proc->priority > current_process->priority) schedule();
    break;
  case SLEEPING: // new priority will take effect after it wakes up
    break;
  case ZOMBIE: // this whole thing was useless
    break;
  case DEAD: // unreachable
    assert(false);
  }

  RETURN_TO_USERSPACE(old_prio);
}

int getprio(int pid)
{
  COME_FROM_USERSPACE();
  if (pid < 1 || pid > NBPROC) RETURN_TO_USERSPACE(-1);
  struct proc *proc = &process_table[pid];
  if (proc->state == DEAD) RETURN_TO_USERSPACE(-1);
  const int prio = proc->priority;
  RETURN_TO_USERSPACE(prio);
}

int getpid(void)
{
  COME_FROM_USERSPACE();
  const int pid = current_process->pid;
  RETURN_TO_USERSPACE(pid);
}

void exit(int retval)
{
  COME_FROM_USERSPACE();

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
  COME_FROM_USERSPACE();

  if (pid < 1 || pid > NBPROC) RETURN_TO_USERSPACE(-1);
  struct proc *proc = &process_table[pid];

  switch (proc->state) {
  case DEAD: // invalid pid
    RETURN_TO_USERSPACE(-1);
  case ZOMBIE: // can't kill what's already dead
    break;
  case ACTIVE: // current process just killed itself :'( lets just exit
    exit(-1);
    break;
  // when a proc isn't running, we can kill and bury it directly
  case READY:
  case SLEEPING:
    PROC_CLEAR_QUEUE(proc);
    proc_free(proc);
    break;
  }

  RETURN_TO_USERSPACE(0);
}

void sleep(unsigned long ticks)
{
  COME_FROM_USERSPACE();
  // setup alarm, go to sleep and yield
  current_process->time.alarm = current_clock() + ticks;
  current_process->state = SLEEPING;
  schedule();
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

static void schedule(void)
{
  // process that's passing the cpu over
  struct proc *pass = current_process;
  assert(pass != NULL);

  // check whether there are zombies to bury
  while (zombie_procs != NULL) {
    struct proc *zombie = zombie_procs;
    assert(zombie != pass);
    zombie_procs = zombie->node.next;
    proc_free(zombie);
  }

  // check whether there are sleeping procs to wake up
  const unsigned long time = current_clock();
  struct proc *       proc;
  queue_for_each(proc, &sleeping_procs, struct proc, node.queue)
  {
    // these are sorted by alarm, so we can stop whenever it wasn't reached yet
    if (proc->time.alarm > time) break;
    // otherwise we wake procs up by moving them to the ready queue
    PROC_CLEAR_QUEUE(proc);
    proc->state = READY;
    proc->time.quantum = QUANTUM;
    PROC_ENQUEUE_READY(proc);
  }

  switch (pass->state) {
  // put it back in the priority queue if it's still active
  case ACTIVE:
    pass->state = READY;
    pass->time.quantum = QUANTUM;
    PROC_ENQUEUE_READY(pass);
    break;

  // we'll only find a zombie proc here when it has just exited
  case ZOMBIE:
    // we cannot free the context we're in, so delay that operation to next time
    pass->node.next = zombie_procs;
    zombie_procs = pass;
    break;

  // when a process goes to sleep, we put it in a separate queue
  case SLEEPING:
    PROC_ENQUEUE_SLEEPING(pass);
    break;

  // unreachable
  case READY:
  case DEAD:
    assert(false);
  }

  // process that will take control of the execution
  assert(!queue_empty(&ready_procs));
  struct proc *take = PROC_DEQUEUE_READY();
  current_process = take;
  take->state = ACTIVE;

  // hand the cpu over to the newly scheduled process
  ctx_sw(&pass->ctx, &take->ctx);
}

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

static int p1(void *arg)
{
  (void)arg;
  for (;;) {
    printf(".");
    wait_clock(MS_TO_TICKS(1 * 1000));
  }
  return 1;
}

static int p2(void *arg)
{
  (void)arg;
  for (;;) {
    printf("-");
    wait_clock(MS_TO_TICKS(2 * 1000));
  }
  return 2;
}

static int p3(void *arg)
{
  (void)arg;
  for (;;) {
    printf("+");
    wait_clock(MS_TO_TICKS(5 * 1000));
  }
  return 3;
}

static int p4(void *arg)
{
  (void)arg;
  for (;;) {
    printf("*");
    wait_clock(MS_TO_TICKS(10 * 1000));
  }
  return 4;
}

static int init(void *arg)
{
  const int i = (int)arg;
  printf("Hello world\n");
  printf("The answer is %d\n", i);

  int pid;
  (void)pid;
  pid = start(p1, 256, 1, "P1", NULL);
  assert(pid > 0);
  pid = start(p2, 256, 1, "P2", NULL);
  assert(pid > 0);
  pid = start(p3, 256, 1, "P3", NULL);
  assert(pid > 0);
  pid = start(p4, 256, 1, "P4", NULL);
  assert(pid > 0);

  wait_clock(MS_TO_TICKS(60 * 1000));
  // TODO: kill children when waking up

  return 0;
}

static int wall_clock_daemon(void *arg)
{
  (void)arg;
  char time[] = "HH:MM:SS";
  for (;;) {
    unsigned seconds = current_clock() / CLOCKFREQ;
    unsigned minutes = seconds / 60;
    seconds %= 60;
    unsigned hours = minutes / 60;
    minutes %= 60;
    hours %= 24;
    sprintf(time, "%02u:%02u:%02u", hours, minutes, seconds);
    console_write_raw(time, 8, 24, 72);
    wait_clock(MS_TO_TICKS(1000));
  }
  return 0;
}

static void idle(void)
{
  // just testing
  int pid;
  (void)pid;
  pid = start(wall_clock_daemon, 256, 1, "clock", NULL);
  assert(pid > 0);
  pid = start(init, 256, MAXPRIO, "init", (void *)42);
  assert(pid > 0);

  sti();
  for (;;) hlt();
}
