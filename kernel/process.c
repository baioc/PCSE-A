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

#include "stddef.h"
#include "debug.h"
#include "cpu.h"
#include "queue.h"
#include "mem.h"
#include "string.h"
#include "stdbool.h"
#include "clock.h"
#include "console.h"

#ifdef KERNEL_TEST
#include "kernel_tests.h"
#endif

/*******************************************************************************
 * Macros
 ******************************************************************************/

// Total number of kenel processes.
#define NBPROC 1000

// Kernel stack size, in words.
#define STACK_SIZE 512

/// Scheduling quantum, in tick units.
#define QUANTUM (CLOCKFREQ / SCHEDFREQ)

/*******************************************************************************
 * Types
 ******************************************************************************/

// Describe different states of a process
enum proc_state {
  DEAD,           // marks a free process slot => free_procs queue
  ZOMBIE,         // terminated but still in use => no queue, access via parent
  SLEEPING,       // process is waiting on its alarm => sleeping_procs queue
  AWAITING_CHILD, // process is waiting for one of its children => no queue
  READY,          // process waiting for his turn => ready_procs queue
  ACTIVE,         // process currently running on processor => current_process
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
  link         node; // doubly-linked node used by lists
  struct proc *parent;
  link         children;
  link         siblings;
  int          retval;
};

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/// Changes context between two processes, defined in ctx_sw.S
extern void ctx_sw(struct context *old, struct context *new);

/// Defined in ctx_sw.S, this is called when a process implicitly exits.
extern void proc_exit(void);

// Makes the current process yield the CPU to the scheduler.
static void schedule(void);

// Starts 'init', enables interrupts and loops indefinitely.
static void idle(void);

// Process tree root that acts as a process reaper daemon.
static int init(void *);

// Sets P as the parent of C and adds C to P's children list.
static void filiate(struct proc *c, struct proc *p);

// Zombifies a process without freeing its resources.
static void zombify(struct proc *proc, int retval);

/*
 * Kills a zombie process once and for all.
 * This means freeing any resources it owns, and moving it to the free list.
 * NOTE: this also removes the process from its parent's children list.
 */
static void destroy(struct proc *proc);

/*******************************************************************************
 * Variables
 ******************************************************************************/

// Table of ALL processes, indexed by their pid.
static struct proc process_table[NBPROC + 1];

#define IDLE_PROC (&process_table[0])
#define INIT_PROC (&process_table[1])

// Current process running on processor.
static struct proc *current_process = NULL;

// Process disjoint lists.
static link ready_procs;
static link free_procs;
static link sleeping_procs;

/*******************************************************************************
 * Public function
 ******************************************************************************/

void process_init(void) // only called from kernel space
{
  // set up initial kernel process, 'idle', which has pid 0
  *IDLE_PROC =
      (struct proc){.name = "idle", .pid = 0, .priority = 0, .parent = NULL};
  IDLE_PROC->children = (link)LIST_HEAD_INIT(IDLE_PROC->children);

  // initialize process lists
  ready_procs = (link)LIST_HEAD_INIT(ready_procs);
  free_procs = (link)LIST_HEAD_INIT(free_procs);
  sleeping_procs = (link)LIST_HEAD_INIT(sleeping_procs);
  for (int i = 1; i <= NBPROC; ++i) { // all other procs begin dead
    struct proc *proc = &process_table[i];
    *proc = (struct proc){.pid = i};
    proc->state = DEAD;
    queue_add(proc, &free_procs, struct proc, node, pid);
  }

  // idle is the first process to run
  current_process = IDLE_PROC;
  IDLE_PROC->state = ACTIVE;
  IDLE_PROC->time.quantum = 0;
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
  assert(pt_func != NULL);
  (void)ssize;
  if (prio < 1 || prio > MAXPRIO) return -1;
  assert(name != NULL);

  // bail when we can't find a free slot in the process table
  if (queue_empty(&free_procs)) return -1;
  struct proc *new_proc = queue_bottom(&free_procs, struct proc, node);

  // otherwise set-up its pid (implicit) and priority (explicit)
  new_proc->pid = new_proc - process_table; // calculate index from pointer
  new_proc->priority = prio;

  // copy name
  new_proc->name = mem_alloc((strlen(name) + 1) * sizeof(char));
  if (new_proc->name == NULL) {
    return -1;
  }
  strcpy(new_proc->name, name);

  // allocate stack
  new_proc->kernel_stack = mem_alloc(STACK_SIZE * sizeof(int));
  if (new_proc->kernel_stack == NULL) {
    mem_free(new_proc->name, (strlen(new_proc->name) + 1) * sizeof(char));
    return -1;
  }

  // setup stack with the given arg, termination code and main function
  new_proc->kernel_stack[STACK_SIZE - 1] = (unsigned)(arg);
  new_proc->kernel_stack[STACK_SIZE - 2] = (unsigned)(proc_exit);
  new_proc->kernel_stack[STACK_SIZE - 3] = (unsigned)(pt_func);
  new_proc->ctx.esp = (unsigned)(&(new_proc->kernel_stack[STACK_SIZE - 3]));

  // initialize children list and filiate itself to parent
  new_proc->children = (link)LIST_HEAD_INIT(new_proc->children);
  filiate(new_proc, current_process);

  // this process is now ready to run
  queue_del(new_proc, node);
  new_proc->state = READY;
  queue_add(new_proc, &ready_procs, struct proc, node, priority);

  // check if the new process should be run immediately or not
  if (current_process != IDLE_PROC && current_process != INIT_PROC &&
      new_proc->priority > current_process->priority)
  {
    schedule();
  }

  return new_proc->pid;
}

int chprio(int pid, int newprio)
{
  // process referenced by that pid doesn't exist, or newprio is invalid
  if (pid < 1 || pid > NBPROC || newprio < 1 || newprio > MAXPRIO) {
    return -1;
  }

  struct proc *proc = &process_table[pid];
  if (proc->state == DEAD || proc->state == ZOMBIE) return -1;

  const int old_prio = proc->priority;
  if (newprio == old_prio) return old_prio; // change priority only if needed

  proc->priority = newprio;

  switch (proc->state) {
  case ACTIVE: { // check whether current process shouldn't be running anymore
    const struct proc *top = queue_top(&ready_procs, struct proc, node);
    if (current_process->priority < top->priority) schedule();
    break;
  }
  case READY: // re-add process to ready queue with new priority
    queue_del(proc, node);
    queue_add(proc, &ready_procs, struct proc, node, priority);
    if (proc->priority > current_process->priority) schedule();
    break;
  // new priority will take effect when it wakes up
  case AWAITING_CHILD:
  case SLEEPING:
  case ZOMBIE:
    break;
  case DEAD: // unreachable
    assert(false);
  }

  return old_prio;
}

int getprio(int pid)
{
  if (pid < 1 || pid > NBPROC) return -1;
  struct proc *proc = &process_table[pid];
  if (proc->state == DEAD) return -1;
  const int prio = proc->priority;
  return prio;
}

int getpid(void)
{
  const int pid = current_process->pid;
  return pid;
}

void exit(int retval)
{
  zombify(current_process, retval);
  schedule();
  for (assert(false);;) { // assert is just a sanity check
    // ensures gcc marks exit as `noreturn`
  }
}

int kill(int pid)
{

  if (pid < 1 || pid > NBPROC) return -1;
  struct proc *proc = &process_table[pid];

  switch (proc->state) {
  case DEAD: // invalid pid
    return -1;
  case ZOMBIE: // can't kill what's already dead
    return -2; // to be compliant with test_4 (kill return value should be
               // negative)
  case ACTIVE: // current process just killed itself :'( lets just exit
    exit(0);
    break;
  case READY:
  case SLEEPING:
    queue_del(proc, node);
    zombify(proc, 0);
    break;
  case AWAITING_CHILD:
    zombify(proc, 0);
    break;
  }

  return 0;
}

void sleep(unsigned long ticks)
{
  // setup alarm, go to sleep and yield
  current_process->time.alarm = current_clock() + ticks;
  current_process->state = SLEEPING;
  schedule();
}

int waitpid(int pid, int *retvalp)
{

  if (pid < 0) { // reap *any* child process
    if (queue_empty(&current_process->children)) return -1;

    for (;;) {
      struct proc *child;
      queue_for_each(child, &current_process->children, struct proc, siblings)
      {
        if (child->state == ZOMBIE) {
          if (retvalp != NULL) *retvalp = child->retval;
          destroy(child);
          return child->pid;
        }
      }

      current_process->state = AWAITING_CHILD;
      schedule();
    }

  } else { // reap a specific child
    if (pid < 1 || pid > NBPROC) return -1;
    struct proc *proc = &process_table[pid];
    if (proc->state == DEAD || proc->parent != current_process) return -1;

    for (;;) {
      if (proc->state == ZOMBIE) {
        if (retvalp != NULL) *retvalp = proc->retval;
        destroy(proc);
        return pid;
      }

      current_process->state = AWAITING_CHILD;
      schedule();
    }
  }
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

static void schedule(void)
{
  // process that's passing the cpu over
  struct proc *pass = current_process;
  assert(pass != NULL);

  // check whether there are sleeping procs to wake up
  const unsigned long now = current_clock();
  struct proc *       proc;
CHECK_ALARM:
  queue_for_each(proc, &sleeping_procs, struct proc, node)
  {
    // these are sorted by alarm, so we can stop whenever one wasn't reached
    if (proc->time.alarm > now) break;

    // otherwise we wake procs up by moving them to the ready queue
    queue_del(proc, node);
    proc->state = READY;
    queue_add(proc, &ready_procs, struct proc, node, priority);
    // an element was deleted => iterator is now invalid and we must refresh it
    goto CHECK_ALARM;
  }

  switch (pass->state) {
  // put it back in the priority queue if it's still active
  case ACTIVE:
    pass->state = READY;
    queue_add(pass, &ready_procs, struct proc, node, priority);
    break;

  // we'll only find a zombie proc here when it has just exited
  case ZOMBIE:
    // let the parent process know when one of its children just dies
    if (pass->parent->state == AWAITING_CHILD) {
      pass->parent->state = READY;
      queue_add(pass->parent, &ready_procs, struct proc, node, priority);
    }
    break;

  // when a process goes to sleep, we put it in a separate queue
  case SLEEPING:
    queue_add(pass, &sleeping_procs, struct proc, node, time.alarm);
    break;

  // nothing to do
  case AWAITING_CHILD:
    break;

  // unreachable
  case READY:
  case DEAD:
    assert(false);
  }

  // process that will take control of the execution
  assert(!queue_empty(&ready_procs));
  struct proc *take = queue_out(&ready_procs, struct proc, node);
  current_process = take;
  take->state = ACTIVE;
  take->time.quantum = QUANTUM;

  // hand the cpu over to the newly scheduled process
  ctx_sw(&pass->ctx, &take->ctx);
}

static void filiate(struct proc *c, struct proc *p)
{
  c->parent = p;
  queue_add(c, &p->children, struct proc, siblings, state);
}

static void zombify(struct proc *proc, int retval)
{
  proc->retval = retval; // store exit code to be read later
  proc->state = ZOMBIE;

  // when a parent process dies, its hierarchy is adopted and killed by init
  while (!queue_empty(&proc->children)) {
    // remove child and recursively kill its children as well
    struct proc *child = queue_out(&proc->children, struct proc, siblings);
    kill(child->pid);

    // then, re-filiate it to init, which will wake up shortly to kill zombies
    filiate(child, INIT_PROC);
    if (INIT_PROC->state == AWAITING_CHILD) {
      INIT_PROC->state = READY;
      queue_add(INIT_PROC, &ready_procs, struct proc, node, priority);
    }
  }
}

static void destroy(struct proc *proc)
{
  assert(proc->state == ZOMBIE);

  // remove proc from its parent children list
  if (proc->parent != NULL) queue_del(proc, siblings);

  // free resources
  mem_free(proc->kernel_stack, STACK_SIZE * sizeof(int));
  mem_free(proc->name, (strlen(proc->name) + 1) * sizeof(char));

  // add it to the free list
  proc->state = DEAD;
  queue_add(proc, &free_procs, struct proc, node, pid);
}

static void idle(void)
{
  // idle must start init
  int pid;
  (void)pid;
  pid = start(init, 256, MAXPRIO, "init", NULL);
  assert(pid == 1);

  // and then stay idle forever
  sti();
  for (;;) hlt();
}

static int wall_clock_daemon(void *arg);
static int p1(void *arg);
static int p2(void *arg);
static int p3(void *arg);
static int p4(void *arg);

static int init(void *arg)
{
  (void)arg;

#ifdef KERNEL_TEST
  printf("---KERNEL TESTS---\n");
  kernel_run_process_tests();
  // wait for all tests (child) to end
  while (waitpid(-1, NULL) != -1)
    ;
  printf("---END OF KERNEL TESTS---\n");
#endif

  printf("Hello world\n");
  start(wall_clock_daemon, 512, 5, "clock", NULL);
  int pids[] = {
      start(p1, 1024, 2, "P1", "."),
      start(p2, 1024, 2, "P2", "-"),
      start(p3, 1024, 2, "P3", "+"),
      start(p4, 1024, 2, "P4", "*"),
  };
  wait_clock(MS_TO_TICKS(31 * 1000));
  for (int i = 0; i < 4; ++i) kill(pids[i]);
  printf("\nThe answer still is %d\n", 42);

  // reaper daemon makes sure zombie children are properly killed
  for (;;) waitpid(-1, NULL);

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

static int p1(void *arg)
{
  const char c = *(char *)arg;
  for (;;) {
    printf("%c", c);
    wait_clock(MS_TO_TICKS(1 * 1000));
  }
  return 1;
}

static int p2(void *arg)
{
  const char c = *(char *)arg;
  for (;;) {
    printf("%c", c);
    wait_clock(MS_TO_TICKS(2 * 1000));
  }
  return 2;
}

static int p3(void *arg)
{
  const char c = *(char *)arg;
  for (;;) {
    printf("%c", c);
    wait_clock(MS_TO_TICKS(5 * 1000));
  }
  return 3;
}

static int p4(void *arg)
{
  const char c = *(char *)arg;
  for (;;) {
    printf("%c", c);
    wait_clock(MS_TO_TICKS(10 * 1000));
  }
  return 4;
}
