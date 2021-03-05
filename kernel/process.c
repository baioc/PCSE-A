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

// Adds process pointed to by P into the ready priority queue.
#define PROC_ENQUEUE_READY(p) \
  queue_add((p), &ready_procs, struct proc, node, priority)

// Pops and returns the highest-priority process from the ready queue.
#define PROC_DEQUEUE_READY() (queue_out(&ready_procs, struct proc, node))

// References the highest-priority process in a queue.
#define PROC_READY_TOP() (queue_top(&ready_procs, struct proc, node))

// Adds process pointed to by P into the sleeping queue, sorted by its alarm.
#define PROC_ENQUEUE_SLEEPING(p) \
  queue_add((p), &sleeping_procs, struct proc, node, time.alarm)

// Adds process pointed to by P into the free queue, sorted by its pid.
#define PROC_ENQUEUE_FREE(p) queue_add((p), &free_procs, struct proc, node, pid)

// References the lowest-pid process in the free list.
#define PROC_FREE_FIRST() (queue_bottom(&free_procs, struct proc, node))

// Adds process pointed to by C to the list of P's children.
#define ADD_CHILD(c, p) \
  queue_add((c), &(p)->children, struct proc, siblings, state)

/*******************************************************************************
 * Types
 ******************************************************************************/

// Describe different states of a process
enum proc_state {
  DEAD,           // marks a free process slot
  ZOMBIE,         // terminated but still in use
  SLEEPING,       // process is waiting on its alarm
  AWAITING_CHILD, // process is waiting for one of its children
  READY,          // process waiting for his turn
  ACTIVE,         // process currently running on processor
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

/**
 * Makes the current process yield the CPU to the scheduler.
 * NOTE: this routine supposes interrupts are disabled and will re-enable them
 * when switching context back to user space.
 */
static void schedule(void);

// Starts 'init', enables interrupts and loops indefinitely.
static void idle(void);

// Process tree root that acts as a process reaper daemon.
static int init(void *);

// Zombifies a process without freeing its resources.
static void zombify(struct proc *proc, int retval);

/**
 * Kills a zombie process once and for all.
 * This means freeing any resources it owns and moving it to the free list.
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
    PROC_ENQUEUE_FREE(proc);
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
  COME_FROM_USERSPACE();

  assert(pt_func != NULL);
  (void)ssize;
  if (prio < 1 || prio > MAXPRIO) RETURN_TO_USERSPACE(-1);
  assert(name != NULL);

  // bail when we can't find a free slot in the process table
  if (queue_empty(&free_procs)) RETURN_TO_USERSPACE(-1);
  struct proc *new_proc = PROC_FREE_FIRST();
  queue_del(new_proc, node);

  // otherwise set-up its pid (implicit) and priority (explicit)
  new_proc->pid = new_proc - process_table; // calculate index from pointer
  new_proc->priority = prio;

  // copy name
  new_proc->name = mem_alloc((strlen(name) + 1) * sizeof(char));
  if (new_proc->name == NULL) {
    PROC_ENQUEUE_FREE(new_proc);
    RETURN_TO_USERSPACE(-1);
  }
  strcpy(new_proc->name, name);

  // allocate stack
  new_proc->kernel_stack = mem_alloc(STACK_SIZE * sizeof(int));
  if (new_proc->kernel_stack == NULL) {
    mem_free(new_proc->name, (strlen(new_proc->name) + 1) * sizeof(char));
    PROC_ENQUEUE_FREE(new_proc);
    RETURN_TO_USERSPACE(-1);
  }

  // setup stack with the given arg, termination code and main function
  new_proc->kernel_stack[STACK_SIZE - 1] = (unsigned)(arg);
  new_proc->kernel_stack[STACK_SIZE - 2] = (unsigned)(proc_exit);
  new_proc->kernel_stack[STACK_SIZE - 3] = (unsigned)(pt_func);
  new_proc->ctx.esp = (unsigned)(&(new_proc->kernel_stack[STACK_SIZE - 3]));

  // initialize children list and add itself to parent's children list
  new_proc->children = (link)LIST_HEAD_INIT(new_proc->children);
  new_proc->parent = current_process;
  ADD_CHILD(new_proc, new_proc->parent);

  // this process is now ready to run
  new_proc->state = READY;
  new_proc->time.quantum = QUANTUM;
  PROC_ENQUEUE_READY(new_proc);

  // check if the new process should be run immediately or not
  if (current_process != IDLE_PROC && current_process != INIT_PROC &&
      new_proc->priority > current_process->priority)
  {
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
    if (current_process->priority < PROC_READY_TOP()->priority) schedule();
    break;
  case READY:
    queue_del(proc, node);    // remove process from the ready queue
    PROC_ENQUEUE_READY(proc); // place it again with the updated priority
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
  zombify(current_process, retval);
  schedule();
  for (assert(false);;) { // assert is just a sanity check
    // ensures gcc marks exit as `noreturn`
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

int waitpid(int pid, int *retvalp)
{
  COME_FROM_USERSPACE();

  if (pid < 0) { // reap *any* child process
    if (queue_empty(&current_process->children)) RETURN_TO_USERSPACE(-1);

    for (;;) {
      printf("\nProcess %s waiting on pid %d: ", current_process->name, pid);
      struct proc *child;
      queue_for_each(child, &current_process->children, struct proc, siblings)
      {
        if (child->state == ZOMBIE) {
          printf("found %d!\n", child->pid);
          if (retvalp != NULL) *retvalp = child->retval;
          destroy(child);
          RETURN_TO_USERSPACE(child->pid);
        }
      }

      printf("will block ...\n");
      current_process->state = AWAITING_CHILD;
      schedule();
    }

  } else { // reap a specific child
    if (pid < 1 || pid > NBPROC) RETURN_TO_USERSPACE(-1);
    struct proc *proc = &process_table[pid];
    if (proc->state == DEAD || proc->parent != current_process)
      RETURN_TO_USERSPACE(-1);

    for (;;) {
      printf("\nProcess %s waiting on pid %d: ", current_process->name, pid);
      if (proc->state == ZOMBIE) {
        printf("found %d!\n", proc->pid);
        if (retvalp != NULL) *retvalp = proc->retval;
        destroy(proc);
        RETURN_TO_USERSPACE(pid);
      }

      printf("will block ...\n");
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
  queue_for_each(proc, &sleeping_procs, struct proc, node)
  {
    // these are sorted by alarm, so we can stop whenever it wasn't reached yet
    if (proc->time.alarm > now) break;
    // otherwise we wake procs up by moving them to the ready queue
    queue_del(proc, node);
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
    printf("\n* %s(%d) just died *\n", pass->name, pass->pid);
    // let the parent process know when one of its children just dies
    if (pass->parent->state == AWAITING_CHILD) {
      pass->parent->state = READY;
      pass->parent->time.quantum = QUANTUM;
      PROC_ENQUEUE_READY(pass->parent);
    }
    break;

  // when a process goes to sleep, we put it in a separate queue
  case SLEEPING:
    PROC_ENQUEUE_SLEEPING(pass);
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
  struct proc *take = PROC_DEQUEUE_READY();
  current_process = take;
  take->state = ACTIVE;

  // hand the cpu over to the newly scheduled process
  ctx_sw(&pass->ctx, &take->ctx);
}

static void zombify(struct proc *proc, int retval)
{
  proc->retval = retval; // store exit code
  proc->state = ZOMBIE;  // change its state
  // when a parent process dies, its now-orphan children are adopted by init
  struct proc *child;
  queue_for_each(child, &proc->children, struct proc, siblings)
  {
    child->parent = INIT_PROC;
    ADD_CHILD(child, INIT_PROC);
    if (INIT_PROC->state == AWAITING_CHILD) {
      INIT_PROC->state = READY;
      INIT_PROC->time.quantum = QUANTUM;
      PROC_ENQUEUE_READY(INIT_PROC);
    }
  }
}

static void destroy(struct proc *proc)
{
  assert(proc->state == ZOMBIE);

  // free resources
  mem_free(proc->kernel_stack, STACK_SIZE * sizeof(int));
  mem_free(proc->name, (strlen(proc->name) + 1) * sizeof(char));

  // add it to the free list
  proc->state = DEAD;
  PROC_ENQUEUE_FREE(proc);
}

static void idle(void)
{
#ifdef KERNEL_TEST
  kernel_run_process_tests();
#endif

  // idle must start init
  const int pid = start(init, 256, 1, "init", NULL);
  (void)pid;
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
  printf("Hello world\n");
  printf("The answer is %d\n", 42);

  int pids[] = {
      [0] = start(wall_clock_daemon, 512, 1, "clock", NULL),
      [1] = start(p1, 1024, 2, "P1", "."),
      [2] = start(p2, 1024, 2, "P2", "-"),
      [3] = start(p3, 1024, 2, "P3", "+"),
      [4] = start(p4, 1024, 2, "P4", "*"),
  };
  wait_clock(MS_TO_TICKS(30 * 1000));
  for (int i = 1; i <= 4; ++i) {
    kill(pids[i]);
    int ret = -1;
    waitpid(pids[i], &ret);
    printf("\n* P%d returned %d *\n", i, ret);
  }

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
