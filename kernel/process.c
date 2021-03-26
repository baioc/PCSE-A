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

/*******************************************************************************
 * Types
 ******************************************************************************/
typedef struct _proc proc;
typedef struct _context context;
/*******************************************************************************
 * Macros
 ******************************************************************************/

/// Scheduling quantum, in tick units.
#define QUANTUM (CLOCKFREQ / SCHEDFREQ)

// Adds process pointed to by P into the ready priority queue.
#define PROC_ENQUEUE_READY(p) \
  queue_add((p), &ready_procs, proc, node, priority)

// Pops and returns the highest-priority process from the ready queue.
#define PROC_DEQUEUE_READY() (queue_out(&ready_procs, proc, node))

// References the highest-priority process in a queue.
#define PROC_READY_TOP() (queue_top(&ready_procs, proc, node))

// Adds process pointed to by P into the sleeping queue, sorted by its alarm.
#define PROC_ENQUEUE_SLEEPING(p) \
  queue_add((p), &sleeping_procs, proc, node, time.alarm)

// Adds process pointed to by P into the free queue, sorted by its pid.
#define PROC_ENQUEUE_FREE(p) queue_add((p), &free_procs, proc, node, pid)

// References the lowest-pid process in the free list.
#define PROC_FREE_FIRST() (queue_bottom(&free_procs, proc, node))

// Adds process pointed to by C to the list of P's children.
#define ADD_CHILD(c, p) \
  queue_add((c), &(p)->children, proc, siblings, state)

// Remove process referenced by c from the list of its parent
#define REMOVE_CHILD(c) queue_del((c), siblings)


/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/// Changes context between two processes, defined in ctx_sw.S
extern void ctx_sw(context *old, context *new);

/// Defined in ctx_sw.S, this is called when a process implicitly exits.
extern void proc_exit(void);

// Starts 'init', enables interrupts and loops indefinitely.
static void idle(void);

// Process tree root that acts as a process reaper daemon.
static int init(void *);

// Zombifies a process without freeing its resources.
static void zombify(proc *p, int retval);

/**
 * Kills a zombie process once and for all.
 * This means freeing any resources it owns and moving it to the free list.
 */
static void destroy(proc *p);

/*******************************************************************************
 * Variables
 ******************************************************************************/

// Table of ALL processes, indexed by their pid.
static proc process_table[NBPROC + 1];

#define IDLE_PROC (&process_table[0])
#define INIT_PROC (&process_table[1])

// Current process running on processor.
proc *current_process = NULL;

// Process disjoint lists.
link free_procs;
link sleeping_procs;
link ready_procs;

extern semaph list_sem[MAXNBR_SEM];
/*******************************************************************************
 * Public function
 ******************************************************************************/

void process_init(void) // only called from kernel space
{
  // set up initial kernel process, 'idle', which has pid 0
  *IDLE_PROC =
      (proc){.name = "idle", .pid = 0, .priority = 0, .parent = NULL};
  IDLE_PROC->children = (link)LIST_HEAD_INIT(IDLE_PROC->children);

  // initialize process lists
  ready_procs = (link)LIST_HEAD_INIT(ready_procs);
  free_procs = (link)LIST_HEAD_INIT(free_procs);
  sleeping_procs = (link)LIST_HEAD_INIT(sleeping_procs);
  for (int i = 1; i <= NBPROC; ++i) { // all other procs begin dead
    proc *p = &process_table[i];
    *p = (proc){.pid = i};
    p->state = DEAD;
    PROC_ENQUEUE_FREE(p);
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
  proc *new_proc = PROC_FREE_FIRST();
  queue_del(new_proc, node);

  // otherwise set-up its pid (implicit) and priority (explicit)
  new_proc->pid = new_proc - process_table; // calculate index from pointer
  new_proc->priority = prio;

  // copy name
  new_proc->name = mem_alloc((strlen(name) + 1) * sizeof(char));
  if (new_proc->name == NULL) {
    PROC_ENQUEUE_FREE(new_proc);
    return -1;
  }
  strcpy(new_proc->name, name);

  // allocate stack
  new_proc->kernel_stack = mem_alloc(STACK_SIZE * sizeof(int));
  if (new_proc->kernel_stack == NULL) {
    mem_free(new_proc->name, (strlen(new_proc->name) + 1) * sizeof(char));
    PROC_ENQUEUE_FREE(new_proc);
    return -1;
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
  return pid;
}

int chprio(int pid, int newprio)
{

  // process referenced by that pid doesn't exist, or newprio is invalid
  if (pid < 1 || pid > NBPROC || newprio < 1 || newprio > MAXPRIO) {
    return -1;
  }

  proc *p = &process_table[pid];
  if (p->state == DEAD || p->state == ZOMBIE) return -1;

  const int old_prio = p->priority;
  if (newprio == old_prio) return old_prio; // change priority only if needed

  p->priority = newprio;

  switch (p->state) {
  case ACTIVE: // check whether current process shouldn't be running anymore
    if (current_process->priority < PROC_READY_TOP()->priority) schedule();
    break;
  case READY:
    queue_del(p, node);    // remove process from the ready queue
    PROC_ENQUEUE_READY(p); // place it again with the updated priority
    if (p->priority > current_process->priority) schedule();
    break;
  // new priority will take effect when it wakes up
  case BLOCKED:
    queue_del(p, blocked);    // remove process from the blocked list
    queue_add(p, &(list_sem[p->sid].list_blocked), proc, blocked, priority);
    // place it again with the updated priority
    break;
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
  proc *p = &process_table[pid];
  if (p->state == DEAD || p->state == ZOMBIE) return -1;
  const int prio = p->priority;
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
  proc *p = &process_table[pid];

  switch (p->state) {
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
    queue_del(p, node);
    zombify(p, 0);
    break;
  case BLOCKED:
  case AWAITING_CHILD:
    zombify(p, 0);
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
      printf("\nProcess %s waiting on pid %d: ", current_process->name, pid);
      proc *child;
      queue_for_each(child, &current_process->children, proc, siblings)
      {
        if (child->state == ZOMBIE) {
          printf("found %d!\n", child->pid);
          if (retvalp != NULL) *retvalp = child->retval;
          destroy(child);
          return child->pid;
        }
      }

      printf("will block ...\n");
      current_process->state = AWAITING_CHILD;
      schedule();
    }

  } else { // reap a specific child
    if (pid < 1 || pid > NBPROC) return -1;
    proc *p = &process_table[pid];
    if (p->state == DEAD || p->parent != current_process) return -1;

    for (;;) {
      printf("\nProcess %s waiting on pid %d: ", current_process->name, pid);
      if (p->state == ZOMBIE) {
	printf("found %d!\n", p->pid);
	if (retvalp != NULL) *retvalp = p->retval;
	destroy(p);
	return pid;
      }

      printf("will block ...\n");
      current_process->state = AWAITING_CHILD;
      schedule();
    }
  }
}

void schedule(void)
{
  // process that's passing the cpu over
  proc *pass = current_process;
  assert(pass != NULL);

  // check whether there are sleeping procs to wake up
  const unsigned long now = current_clock();
  proc *       p;
  do {
    p = 0;
    queue_for_each(p, &sleeping_procs, proc, node)
    {
      // these are sorted by alarm,
      // so we can stop whenever it wasn't reached yet
      if (p->time.alarm > now) break;
      // otherwise we wake procs up by moving them to the ready queue
      queue_del(p, node);
      p->state = READY;
      p->time.quantum = QUANTUM;
      PROC_ENQUEUE_READY(p);
      // an element was deleted, we need to iterate on the list again
      break;
    }
  } while (p->time.alarm <= now && &p->node != &sleeping_procs);

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

  // we'll only find a blocked proc here when it has just been blocked
  case BLOCKED:
    printf("\n* %s(%d) just blocked *\n", pass->name, pass->pid);
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
  proc *take = PROC_DEQUEUE_READY();
  current_process = take;
  take->state = ACTIVE;

  // hand the cpu over to the newly scheduled process
  ctx_sw(&pass->ctx, &take->ctx);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

static void zombify(proc *p, int retval)
{
  if(p->state == BLOCKED){
    queue_del(p, blocked); // remove process from blocked list
    list_sem[p->sid].count += 1; // increments counter of semaphore
  }
  p->retval = retval; // store exit code
  p->state = ZOMBIE;  // change its state
  // when a parent process dies, its now-orphan children are adopted by init
  proc *child;

  // empty proc children list
  do {
    queue_for_each(child, &p->children, proc, siblings)
    {
      child->parent = INIT_PROC;
      // remove child from its parent's children list
      REMOVE_CHILD(child);
      // add it to init's children
      ADD_CHILD(child, INIT_PROC);
      if (INIT_PROC->state == AWAITING_CHILD) {
      	INIT_PROC->state = READY;
      	INIT_PROC->time.quantum = QUANTUM;
      	PROC_ENQUEUE_READY(INIT_PROC);
      }
      // a child was deleted from proc's children, we need to iterate again
      break;
    }
  } while (!queue_empty(&p->children));
}

static void destroy(proc *p)
{
  assert(p->state == ZOMBIE);

  // free resources
  mem_free(p->kernel_stack, STACK_SIZE * sizeof(int));
  mem_free(p->name, (strlen(p->name) + 1) * sizeof(char));

  // remove proc from its parent children list
  if (p->parent != NULL) REMOVE_CHILD(p);

  // add it to the free list
  p->state = DEAD;
  PROC_ENQUEUE_FREE(p);
}

static void idle(void)
{
  int pid;

  // idle must start init
  pid = start(init, 256, 1, "init", NULL);
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

#ifdef KERNEL_TEST
  printf("---KERNEL TESTS---\n");
  kernel_run_process_tests();

  // wait for all tests (child) to end
  while (waitpid(-1, NULL) != -1)
    ;
  printf("---END OF KERNEL TESTS---\n");
#endif

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
