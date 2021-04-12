/*
 * sem.c
 *
 *  Created on: 03/03/2021
 *      Authors: Antoine Briançon, baioc
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "sem.h"
#include "pm.h"

#include "stddef.h"
#include "queue.h"
#include "stdbool.h"
#include "debug.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define MAXNBR_SEM 500

/*******************************************************************************
 * Types
 ******************************************************************************/

typedef struct proc proc;

typedef struct semaph {
  int            sid;
  bool           in_use;
  short int      count;
  link           list_blocked; // List of process blocked on this semaphore
  struct semaph *next;
} semaph;

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static struct semaph *free_list = NULL;

static struct semaph list_sem[MAXNBR_SEM];

/*******************************************************************************
 * Public function
 ******************************************************************************/

void sem_init(void)
{
  free_list = NULL;
  for (int i = MAXNBR_SEM - 1; i >= 0; --i) {
    list_sem[i] = (struct semaph){.sid = i, .in_use = false};
    list_sem[i].next = free_list;
    free_list = &list_sem[i];
  }
}

int screate(short int count)
{
  if (count < 0 || free_list == NULL) return -1;

  // pop first free semaphore
  struct semaph *sem = free_list;
  free_list = free_list->next;
  sem->in_use = true;

  sem->sid = sem - list_sem;
  sem->count = count;
  sem->list_blocked = (link)LIST_HEAD_INIT(sem->list_blocked);

  return sem->sid;
}

int sdelete(int sem)
{
  if (sem < 0 || sem >= MAXNBR_SEM || !list_sem[sem].in_use) return -1;

  while (!queue_empty(&(list_sem[sem].list_blocked))) {
    proc *p = queue_out(&(list_sem[sem].list_blocked), proc, node);
    assert(p->state == BLOCKED);
    p->sjustdelete = true;
    p->state = READY;
    set_ready(p);
  }

  list_sem[sem].in_use = false;
  list_sem[sem].next = free_list;
  free_list = &list_sem[sem];

  return 0;
}

int signal(int sem)
{
  return signaln(sem, 1);
}

int signaln(int sem, short int count)
{
  if (sem < 0 || sem >= MAXNBR_SEM || !list_sem[sem].in_use) return -1;

  // also check for overflow
  const unsigned short new_count = (unsigned short)list_sem[sem].count + count;
  if (count < 0 || new_count < list_sem[sem].count) return -2;

  for (int i = 0; i < count; i++) {
    list_sem[sem].count += 1;
    if (list_sem[sem].count <= 0) {
      assert(!queue_empty(&(list_sem[sem].list_blocked)));
      proc *p = queue_out(&(list_sem[sem].list_blocked), proc, node);
      assert(p->state == BLOCKED);
      p->state = READY;
      set_ready(p);
    }
  }

  schedule();

  return 0;
}

int sreset(int sem, short int count)
{
  if (count < 0 || sem < 0 || sem >= MAXNBR_SEM || !list_sem[sem].in_use)
    return -1;

  while (!queue_empty(&(list_sem[sem].list_blocked))) {
    proc *p = queue_out(&(list_sem[sem].list_blocked), proc, node);
    assert(p->state == BLOCKED);
    p->sjustreset = true;
    p->state = READY;
    set_ready(p);
  }

  list_sem[sem].count = count;

  return 0;
}

int wait(int sem)
{
  const int try = try_wait(sem);
  if (try != -3) return try;

  // we know for a fact this should block, so do a blocking P
  list_sem[sem].count -= 1;
  assert(list_sem[sem].count < 0);
  proc *p = get_current_process();
  p->sid = sem;
  p->sjustreset = false;
  p->sjustdelete = false;
  p->state = BLOCKED;
  queue_add(p, &(list_sem[sem].list_blocked), proc, node, priority);
  schedule();

  // check whether we unblocked because of a delete/reset
  if (get_current_process()->sjustdelete == 1) return -3;
  if (get_current_process()->sjustreset == 1) return -4;

  return 0;
}

int try_wait(int sem)
{
  if (sem < 0 || sem >= MAXNBR_SEM || !list_sem[sem].in_use) return -1;

  // check for overflow
  const unsigned short new_count = (unsigned short)list_sem[sem].count - 1;
  if (new_count > list_sem[sem].count) return -2;

  if (list_sem[sem].count <= 0) return -3; // would have blocked

  list_sem[sem].count -= 1; // non-blocking
  return 0;
}

int scount(int sem)
{
  if (sem < 0 || sem >= MAXNBR_SEM || !list_sem[sem].in_use) return -1;
  return (unsigned int)list_sem[sem].count & 0x0000ffff;
}

void sem_changing_proc_prio(proc *p)
{
  assert(p->state == BLOCKED);
  assert(p->sid >= 0 && p->sid < MAXNBR_SEM && list_sem[p->sid].in_use);
  queue_del(p, node);
  queue_add(p, &(list_sem[p->sid].list_blocked), proc, node, priority);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/
