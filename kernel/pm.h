/**
 * pm.h
 * Kernelspace shared declarations and definitions for process management.
 */

#ifndef _PM_H_
#define _PM_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "stdint.h"
#include "stdbool.h"
#include "queue.h"
#include "mem.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define MAX_SHM_PAGES 128 /* per process */

/*******************************************************************************
 * Types
 ******************************************************************************/

struct shm_page; // forward decl

struct proc {
  // current state
  enum {
    DEAD,     // free process slot => free_procs queue
    ZOMBIE,   // terminated but still in use => no queue, access via parent
    SLEEPING, // waiting on its alarm => sleeping_procs queue
    BLOCKED,  // blocked in a semaphore => acess via sem sid
    AWAITING_CHILD, // waiting for child => no queue, access via children
    AWAITING_IO,    // blocked in a message queue => access via mqueue fid
    READY,          // waiting for his turn with the CPU => ready_procs queue
    ACTIVE,         // currently running on the processor => current_process
  } state;

  // process identification
  int   pid;
  char *name;

  // XXX: do not reorder ctx fields, used in switch_context()
  struct {
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t page_dir; // aka CR3
    uint32_t esp0;
  } ctx;
  uint32_t *kernel_stack;

  // scheduling information
  int priority;
  union {
    unsigned long quantum; // remaining cpu time (in ticks) for this proc
    unsigned long alarm;   // timestamp (in ticks) to wake a sleeping process
  } time;
  link node; // doubly-linked node used by queues

  // filiation links and temporary return storage
  int          retval;
  struct proc *parent;
  link         children;
  link         siblings;

  // paging structures, private and shared
  struct page *    pages;
  uint32_t         shm_begin; // virtual address base for shared pages
  struct shm_page *shm_slots[MAX_SHM_PAGES];

  // semaphore-related fields
  int  sid;         // id of the semaphore blocking it (if BLOCKED)
  bool sjustreset;  // whether sem was reset
  bool sjustdelete; // whether sem was deleted

  // message queue-related fields
  void *message;     // message being sent/received
  int   m_queue_fid; // queue in which the process is blocked, if any
  bool  m_queue_rd;  // whether the queue was deleted/reset
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/// Makes the current process yield the CPU to the scheduler. @[process.c]
void schedule(void);

/// Gets a reference to the currently active process. @[process.c]
struct proc *get_current_process(void);

/// Puts a hijacked process back into the scheduler's ready queue. @[process.c]
void set_ready(struct proc *proc);

#endif /* _PM_H_ */
