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
#include "userspace_apps.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

// Total number of kenel processes.
#define NBPROC 1000

// Kernel stack size, in words.
#define KERNEL_STACK_SIZE 512

/// Scheduling frequency in Hz, meaning a quantum has 1/SCHEDFREQ seconds.
#define SCHEDFREQ 50

/// Scheduling quantum, in tick units.
#define QUANTUM (CLOCKFREQ / SCHEDFREQ)

// Userspace virtual mappings.
#define MMAP_USER_START 0x40000000 /* 1 GiB */
#define MMAP_STACK_END  0xC0000000 /* 3 GiB */

/*******************************************************************************
 * Types
 ******************************************************************************/

struct proc {
  // current state
  enum {
    DEAD,     // free process slot => free_procs queue
    ZOMBIE,   // terminated but still in use => no queue, access via parent
    SLEEPING, // waiting on its alarm => sleeping_procs queue
    AWAITING_CHILD, // waiting for child => no queue, access via children
    READY,          // waiting for his turn with the CPU => ready_procs queue
    ACTIVE,         // currently running on the processor => current_process
  } state;

  // process identification
  int   pid;
  char *name;

  // Execution context used in switch_context()
  struct {
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t page_dir; // aka CR3
    uint32_t esp0;
  } ctx;

  // scheduling information
  int priority;
  union {
    unsigned long quantum; // remaining cpu time (in ticks) for this proc
    unsigned long alarm;   // timestamp (in ticks) to wake a sleeping process
  } time;
  link node; // doubly-linked node used by queues

  // filiation links
  struct proc *parent;
  link         children;
  link         siblings;

  // return value storage
  int retval;

  // dynamically allocated memory
  uint32_t *   kernel_stack;
  struct page *pages;
};

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/// Changes context between two processes, see switch.S
extern void switch_context(uint32_t *old, uint32_t *new);

/// Forges an interrupt stack and IRETs to userspace, see switch.S
extern void switch_mode_user(uint32_t ip, uint32_t sp, uint32_t *pgdir);

/// Makes the current process yield the CPU to the scheduler.
static void schedule(void);

// Starts "init", enables interrupts and loops indefinitely.
static void idle(void);

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

/**
 * Maps, on a process PROC's page directory, enough virtual memory for at least
 * SIZE bytes starting at the virtual address BASE. When CONTENTS is not null,
 * this will also copy SIZE bytes from that address into the mapped region.
 * BASE must be a page-aligned address.
 *
 * Note that this procedure will acquire physical pages for the allocated
 * region, as well as for any needed page tables, while applying user flags to
 * these page dir/table entries.
 *
 * On success, returns the highest mapped virtual address (so that+1 gives the
 * next page-aligned unmapped virtual address that could be used for subsequent
 * regions), otherwise returns 0 (and the page directory is now invalid).
 */
static uint32_t mmap_region(struct proc *proc, uint32_t base, size_t size,
                            const void *contents);

/*******************************************************************************
 * Variables
 ******************************************************************************/

// Kernel page directory.
extern unsigned pgdir[];

// Table of ALL processes, indexed by their pid.
static struct proc process_table[NBPROC + 1];

// Special kernel "idle" process.
#define IDLE_PROC (&process_table[0])

// Special user "init" process.
static struct proc *INIT_PROC = NULL;

// Current running process.
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
  // set up initial kernel process "idle"
  *IDLE_PROC =
      (struct proc){.pid = 0, .name = "idle", .priority = 0, .parent = NULL};
  IDLE_PROC->children = (link)LIST_HEAD_INIT(IDLE_PROC->children);
  IDLE_PROC->ctx.page_dir = (uint32_t)pgdir;

  // initialize process lists
  ready_procs = (link)LIST_HEAD_INIT(ready_procs);
  free_procs = (link)LIST_HEAD_INIT(free_procs);
  sleeping_procs = (link)LIST_HEAD_INIT(sleeping_procs);
  for (int i = NBPROC; i >= 1; --i) { // all other procs begin dead
    struct proc *proc = &process_table[i];
    *proc = (struct proc){.pid = i};
    proc->state = DEAD;
    queue_add(proc, &free_procs, struct proc, node, pid);
  }

  // idle must be the first process to run
  current_process = IDLE_PROC;
  IDLE_PROC->state = ACTIVE;
  IDLE_PROC->time.quantum = 0;
  idle();
}

void process_tick(void)
{
  assert(current_process->state == ACTIVE);
  if (current_process->time.quantum-- == 0) schedule();
}

int start(const char *name, unsigned long ssize, int prio, void *arg)
{
  const struct uapps *app = uapp_get(name);
  if (app == NULL) return -1;
  if (ssize == 0) return -1;
  if (prio < 1 || prio > MAXPRIO) return -1;

  // bail when we can't find a free slot in the process table
  if (queue_empty(&free_procs)) return -1;
  struct proc *new_proc = queue_bottom(&free_procs, struct proc, node);

  // otherwise set-up its pid, priority and name
  new_proc->pid = new_proc - process_table; // calculate index from pointer
  new_proc->priority = prio;
  new_proc->name = (char *)app->name;

  // allocate kernel stack
  new_proc->kernel_stack = mem_alloc(KERNEL_STACK_SIZE * sizeof(int));
  if (new_proc->kernel_stack == NULL) return -1;
  new_proc->ctx.esp0 = (uint32_t)&new_proc->kernel_stack[KERNEL_STACK_SIZE];

  // initialize frame list and pagedir (copy kernel's first 64 pgdir entries)
  new_proc->pages = page_alloc();
  if (new_proc->pages == NULL) goto FAIL_FREE_STACK;
  new_proc->pages->next = NULL;
  new_proc->ctx.page_dir = new_proc->pages->frame * PAGE_SIZE;
  memcpy((uint32_t *)new_proc->ctx.page_dir, pgdir, 64 * sizeof(pgdir[0]));

  // map and copy application code into process virtual memory
  const uint32_t app_last = mmap_region(new_proc,
                                        MMAP_USER_START,
                                        (char *)app->end - (char *)app->start,
                                        app->start);
  if (app_last == 0) goto FAIL_FREE_PAGES;

  // allocate and map user stack
  uint32_t stack_first = MMAP_STACK_END - ssize;
  stack_first -= stack_first % PAGE_SIZE; // round down to page-aligned address
  // we don't use the entire memory space, so data and stack memory really can't
  // bump into each other, but we'll add the check anyway while making sure
  // there's at least one unmapped page between them (to catch stack overflows)
  if (stack_first - PAGE_SIZE <= app_last) goto FAIL_FREE_PAGES;
  const uint32_t stack_last =
      mmap_region(new_proc, stack_first, MMAP_STACK_END - stack_first, NULL);
  if (stack_last == 0) goto FAIL_FREE_PAGES;
  assert(stack_last == MMAP_STACK_END - 1);

  // setup initial user stack with the given arg
  uint32_t *stack_bottom =
      translate((uint32_t *)new_proc->ctx.page_dir, MMAP_STACK_END - 4);
  assert(stack_bottom != NULL);
  stack_bottom[0] = (uint32_t)arg;
  stack_bottom[-1] = (uint32_t)NULL; // acts as padding to get arg in 4(%esp)

  // setup kernel stack that moves to userspace on RET
  new_proc->kernel_stack[KERNEL_STACK_SIZE - 1] = new_proc->ctx.page_dir;
  new_proc->kernel_stack[KERNEL_STACK_SIZE - 2] = MMAP_STACK_END - 2 * 4;
  new_proc->kernel_stack[KERNEL_STACK_SIZE - 3] = MMAP_USER_START;
  new_proc->kernel_stack[KERNEL_STACK_SIZE - 4] = (uint32_t)NULL;
  new_proc->kernel_stack[KERNEL_STACK_SIZE - 5] = (uint32_t)switch_mode_user;
  new_proc->ctx.esp = (uint32_t)&new_proc->kernel_stack[KERNEL_STACK_SIZE - 5];

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

FAIL_FREE_PAGES:
  while (new_proc->pages != NULL) {
    struct page *page = new_proc->pages;
    new_proc->pages = page->next;
    page_free(page);
  }
FAIL_FREE_STACK:
  mem_free(new_proc->kernel_stack, KERNEL_STACK_SIZE * sizeof(int));
  return -1;
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
    break;
  // unreachable
  case ZOMBIE:
  case DEAD:
    BUG();
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
  BUG(); // unreachable
}

int kill(int pid)
{
  if (pid < 1 || pid > NBPROC) return -1;
  struct proc *proc = &process_table[pid];

  switch (proc->state) {
  case DEAD:   // invalid pid
  case ZOMBIE: // can't kill what's already dead
    return -1;
  case ACTIVE: // current process just killed itself :'( lets just exit
    exit(0);
    break;
  case READY:
  case SLEEPING:
    queue_del(proc, node);
    /* fall through */
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
    assert(proc->state == SLEEPING);
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

  // we'll only find a zombie proc here when it has just called exit()
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
    BUG();
  }

  // process that will take control of the execution
  assert(!queue_empty(&ready_procs));
  struct proc *take = queue_out(&ready_procs, struct proc, node);
  current_process = take;
  take->state = ACTIVE;
  take->time.quantum = QUANTUM;

  // hand the cpu over to the newly scheduled process
  if (pass != take)
    switch_context((uint32_t *)&pass->ctx, (uint32_t *)&take->ctx);
}

static inline void filiate(struct proc *c, struct proc *p)
{
  c->parent = p;
  queue_add(c, &p->children, struct proc, siblings, state);
}

static void zombify(struct proc *proc, int retval)
{
  proc->retval = retval; // store exit code to be read later
  proc->state = ZOMBIE;

  // when a process dies, its children must be adopted by init
  while (!queue_empty(&proc->children)) {
    struct proc *child = queue_out(&proc->children, struct proc, siblings);
    filiate(child, INIT_PROC);
    // when there are zombie children, wake init up to avoid a resource leak
    if (child->state == ZOMBIE && INIT_PROC->state == AWAITING_CHILD) {
      INIT_PROC->state = READY;
      queue_add(INIT_PROC, &ready_procs, struct proc, node, priority);
    }
  }

  // let waiting parent know its child is now a ready-to-be-reaped zombie
  assert(proc->parent != NULL);
  if (proc->parent->state == AWAITING_CHILD) {
    proc->parent->state = READY;
    queue_add(proc->parent, &ready_procs, struct proc, node, priority);
  }
}

static void destroy(struct proc *proc)
{
  assert(proc->state == ZOMBIE);

  // remove proc from its parent children list
  if (proc->parent != NULL) queue_del(proc, siblings);

  // free resources
  while (proc->pages != NULL) {
    struct page *page = proc->pages;
    proc->pages = page->next;
    page_free(page);
  }
  mem_free(proc->kernel_stack, KERNEL_STACK_SIZE * sizeof(int));

  // add it to the free list
  proc->state = DEAD;
  queue_add(proc, &free_procs, struct proc, node, pid);
}

static void idle(void)
{
  // idle must start init
  const int pid = start("init", 4000, MAXPRIO, NULL);
  assert(pid > 0);
  INIT_PROC = &process_table[pid];

  // and then stay idle forever
  sti();
  for (;;) hlt();
}

static uint32_t mmap_region(struct proc *proc, uint32_t base, size_t size,
                            const void *contents)
{
  assert(base % PAGE_SIZE == 0); // assert alignment
  const unsigned flags = (PAGE_USER | PAGE_WRITE | PAGE_PRESENT); // user R/W

  // map region in chunks of maximum size equal to that of a page
  while (size != 0) {
    const size_t chunk = size >= PAGE_SIZE ? PAGE_SIZE : size;

    // try to allocate a page for the actual data
    struct page *page = page_alloc();
    if (page == NULL) return 0;
    page->next = proc->pages;
    proc->pages = page;

    // compute page-aligned addresses, both virtual and physical
    const uint32_t virt = base;
    const uint32_t real = page->frame * PAGE_SIZE;

    // copy contents if needed
    if (contents != NULL) {
      memcpy((void *)real, contents, chunk);
      contents = (char *)contents + chunk;
    }

    // we might need to allocate a page table as well
    int pde = page_map((uint32_t *)proc->ctx.page_dir, virt, real, flags);
    if (pde >= 0) {
      // fortunately, it fits exactly in a page
      struct page *page_tab = page_alloc();
      if (page_tab == NULL) return 0;
      page_tab->next = proc->pages;
      proc->pages = page_tab;

      // add page directory entry, then try the mapping again (should work now)
      ((uint32_t *)proc->ctx.page_dir)[pde] =
          (page_tab->frame * PAGE_SIZE) | flags;
      pde = page_map((uint32_t *)proc->ctx.page_dir, virt, real, flags);
      assert(pde < 0);
    }

    base += PAGE_SIZE;
    size -= chunk;
  }

  return base - 1;
}
