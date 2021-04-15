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

#define MAXNBR_SEM 512

/*******************************************************************************
 * Types
 ******************************************************************************/

typedef struct proc proc;

typedef struct semaph {
  int       sid;
  bool      in_use;
  link      node;
  short int count;
  link      list_blocked; // List of process blocked on this semaphore
} semaph;

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static struct semaph list_sem[MAXNBR_SEM];
static link          free_list;

/*******************************************************************************
 * Public function
 ******************************************************************************/

void sem_init(void)
{
  free_list = (link)LIST_HEAD_INIT(free_list);
  for (int i = MAXNBR_SEM - 1; i >= 0; --i) {
    struct semaph *sem = &list_sem[i];
    sem->sid = i;
    sem->in_use = false;
    queue_add(sem, &free_list, struct semaph, node, sid);
  }
}

int screate(short int count)
{
  if (count < 0) return -1;
  if (queue_empty(&free_list)) return -1;

  struct semaph *sem  = queue_bottom(&free_list, struct semaph, node);
  sem->count = count;
  sem->list_blocked = (link)LIST_HEAD_INIT(sem->list_blocked);
  queue_del(sem, node);
  sem->in_use = true;
  proc *p = get_current_process();
  queue_add(sem, &p->owned_semaphores, struct semaph, node, in_use);

  return sem->sid;
}

int sdelete(int sem)
{
  if (sem < 0 || sem >= MAXNBR_SEM || !list_sem[sem].in_use) return -1;

  while (!queue_empty(&list_sem[sem].list_blocked)) {
    proc *p = queue_out(&list_sem[sem].list_blocked, proc, node);
    assert(p->state == BLOCKED);
    p->sjustdelete = true;
    p->state = READY;
    set_ready(p);
  }

  queue_del(&list_sem[sem], node);
  list_sem[sem].in_use = false;
  queue_add(&list_sem[sem], &free_list, struct semaph, node, sid);

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

  bool sched = false;
  for (int i = 0; i < count; i++) {
    list_sem[sem].count += 1;
    if (list_sem[sem].count <= 0 && !queue_empty(&list_sem[sem].list_blocked)) {
      proc *p = queue_out(&list_sem[sem].list_blocked, proc, node);
      assert(p->state == BLOCKED);
      p->state = READY;
      set_ready(p);
      sched = true;
    }
  }
  if (sched) schedule();

  return 0;
}

int sreset(int sem, short int count)
{
  if (count < 0 || sem < 0 || sem >= MAXNBR_SEM || !list_sem[sem].in_use)
    return -1;

  while (!queue_empty(&list_sem[sem].list_blocked)) {
    proc *p = queue_out(&list_sem[sem].list_blocked, proc, node);
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
  queue_add(p, &list_sem[sem].list_blocked, proc, node, priority);
  schedule();

  // check whether we unblocked because of a delete/reset
  if (get_current_process()->sjustdelete) return -3;
  if (get_current_process()->sjustreset) return -4;

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
  return ((unsigned int)list_sem[sem].count) & 0x0000ffff;
}

void sem_process_init(struct proc *p)
{
  p->owned_semaphores = (link)LIST_HEAD_INIT(p->owned_semaphores);
}

void sem_process_destroy(struct proc *p)
{
  while (!queue_empty(&p->owned_semaphores)) {
    struct semaph *sem = queue_out(&p->owned_semaphores, struct semaph, node);
    sdelete(sem->sid);
  }
}

void sem_process_chprio(struct proc *p)
{
  assert(p->state == BLOCKED);
  assert(p->sid >= 0 && p->sid < MAXNBR_SEM && list_sem[p->sid].in_use);
  queue_del(p, node);
  queue_add(p, &list_sem[p->sid].list_blocked, proc, node, priority);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/
