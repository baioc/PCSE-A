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

/*******************************************************************************
 * Types
 ******************************************************************************/

// Describe different states of a process
enum proc_state {
  ACTIVE,         // process currently running on processor
  READY,          // process waiting for his turn
  ZOMBIE,         // terminated but still in use
  DEAD,           // marks a free process slot
  AWAITING_CHILD, // process is waiting for one of its children
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
  int             priority;
  union {
    link         queue; // doubly-linked list node used by queues
    struct proc *next;  // singly-linked list pointer
  } node;
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

// Adds process into priority queue.
static void proc_list_add(struct proc *proc);

// Removes process from the queue it's currently in.
static void proc_list_del(struct proc *proc);

// References the highest-priority process in a queue.
static struct proc *proc_list_top(void);

// Pops the highest-priority process from the priority queue.
static struct proc *proc_list_out(void);

/*
  print all children's name of the chosen process
*/
static void show_children(void);

/*
  print all parent's name of the chosen process if it has one
*/
static void show_parent(struct proc *proc);

/*******************************************************************************
 * Variables
 ******************************************************************************/

// Table of ALL processes, indexed by their pid.
static struct proc process_table[NBPROC + 1] = {0};

#define IDLE_PROC process_table[0]
#define INIT_PROC process_table[1]

// Current process running on processor.
static struct proc *current_process = NULL;

// Process priority queue.
static link process_queue;

// Process disjoint lists.
static struct proc *free_procs = NULL;

/*******************************************************************************
 * Public function
 ******************************************************************************/

void process_init(void)
{
  // set up initial kernel process, 'idle', which has pid 0
  IDLE_PROC =
      (struct proc){.name = "idle", .pid = 0, .priority = 0, .parent = NULL};
  IDLE_PROC.children = (link)LIST_HEAD_INIT(IDLE_PROC.children);

  // initialize process queue and lists
  process_queue = (link)LIST_HEAD_INIT(process_queue);
  free_procs = NULL;
  for (int i = NBPROC; i >= 1; --i) { // all other procs begin dead
    struct proc *proc = &process_table[i];
    proc->state = DEAD;
    proc->node.next = free_procs;
    free_procs = proc;
  }

  // idle is the first process to run
  current_process = &IDLE_PROC;
  IDLE_PROC.state = ACTIVE;
  idle();
}

void schedule(void)
{
  // process that's passing the cpu over
  struct proc *pass = current_process;
  assert(pass != NULL);

  switch (pass->state) {
  // put it back in the priority queue if it's still active
  case ACTIVE:
    pass->state = READY;
    proc_list_add(pass);
    break;

  // we'll only find a zombie proc here when it has just exited
  case ZOMBIE:
    printf("\n* %s(%d) just died *\n", pass->name, pass->pid);
    // let the parent process know when one of its children just dies
    if (pass->parent->state == AWAITING_CHILD) {
      pass->parent->state = READY;
      proc_list_add(pass->parent);
    }
    break;

  case AWAITING_CHILD: // nothing to do
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

  // initialize children list and add itself to parent's children list
  new_proc->children = (link)LIST_HEAD_INIT(new_proc->children);
  new_proc->parent = current_process;
  queue_add(
      new_proc, &new_proc->parent->children, struct proc, siblings, parent);
  show_parent(new_proc);

  // this process is now ready to run
  new_proc->state = READY;
  proc_list_add(new_proc);

  // check if the new process should be run immediately or not
  if (current_process != &IDLE_PROC && current_process != &INIT_PROC &&
      new_proc->priority > current_process->priority)
  {
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
  case DEAD:
    return -1; // invalid pid
  case ZOMBIE:
    break; // can't kill what's already dead
  case ACTIVE:
    exit(0); // current process just killed itself... lets just exit
    break;
  case READY:
    proc_list_del(proc);
    zombify(proc, 0);
    break;
  case AWAITING_CHILD:
    zombify(proc, 0);
    break;
  }
  return 0;
}

int waitpid(int pid, int *retvalp)
{
  if (pid < 0) { // reap *any* child process
    if (queue_empty(&current_process->children)) return -1;

    for (;;) {
      printf("\nProcess %s waiting on pid %d: ", current_process->name, pid);
      // TODO: optimize this search with priorities
      struct proc *child;
      queue_for_each(child, &current_process->children, struct proc, siblings)
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
    struct proc *proc = &process_table[pid];
    if (proc->state == DEAD || proc->parent != current_process) return -1;

    for (;;) {
      printf("\nProcess %s waiting on pid %d: ", current_process->name, pid);
      if (proc->state == ZOMBIE) {
        printf("found %d!\n", proc->pid);
        if (retvalp != NULL) *retvalp = proc->retval;
        destroy(proc);
        return pid;
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

static void zombify(struct proc *proc, int retval)
{
  proc->retval = retval; // store exit code
  proc->state = ZOMBIE;  // change its state
  // when a parent process dies, its now-orphan children are adopted by init
  struct proc *child;
  queue_for_each(child, &proc->children, struct proc, siblings)
  {
    child->parent = &INIT_PROC;
    queue_add(child, &child->parent->children, struct proc, siblings, parent);
  }
}

static inline void destroy(struct proc *proc)
{
  assert(proc->state == ZOMBIE);

  // free resources
  mem_free(proc->kernel_stack, STACK_SIZE * sizeof(int));
  mem_free(proc->name, (strlen(proc->name) + 1) * sizeof(char));

  // add it to the free list
  proc->state = DEAD;
  proc->node.next = free_procs;
  free_procs = proc;
}

static int test(void *arg)
{
  const char c = *(char *)arg;
  for (int i = 0; i < 5 * (CLOCKFREQ / SCHEDFREQ); ++i) {
    printf("%c", c);
    hlt();
  }
  return c;
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
  }
  return 0;
}

static int init(void *arg)
{
  (void)arg;
  const int i = 42;
  printf("Hello world\n");
  printf("The answer is %d\n", i);

  int pid;
  (void)pid;

  pid = start(wall_clock_daemon, 256, 1, "clock", NULL);
  assert(pid > 0);

  pid = start(test, 256, 2, "test_A", "A");
  assert(pid > 0);

  pid = start(test, 256, 2, "test_B", "B");
  assert(pid > 0);

  pid = start(test, 256, 2, "test_C", "C");
  assert(pid > 0);

  show_children();

  // reaper daemon makes sure zombie children are properly killed
  for (;;) {
    if (waitpid(-1, NULL) < 0) hlt();
  }
  return 0;
}

static void idle(void)
{
  // idle must start init
  const int pid = start(init, 128, 1, "init", NULL);
  (void)pid;
  assert(pid == 1);

  // and then stay idle forever
  sti();
  for (;;) hlt();
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

/*
  print all children's name of the chosen process
*/
static void show_children(void)
{
  struct proc *p;
  int          i = 1;
  printf("%s :\n", current_process->name);
  queue_for_each(p, &(current_process->children), struct proc, siblings)
  {
    printf("Enfant %d : %s\n", i, p->name);
    i++;
  }
}

/*
  print all parent's name of the chosen process if it has one
*/
static void show_parent(struct proc *proc)
{
  printf("%s :\n", proc->name);
  if (proc->parent != NULL) {
    printf("Parent : %s\n", proc->parent->name);
  } else {
    printf("Le processus n'a pas de parent\n");
  }
}
