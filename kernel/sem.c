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
#include "stdint.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define MAXNBR_SEM 10000

/*******************************************************************************
 * Types
 ******************************************************************************/

typedef struct proc proc;

struct semaph {
  int  sid;
  bool in_use;

  // either value represents the semaphore's value, or blocked is not empty
  link      blocked;
  short int value;

  link node;
};

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

  struct semaph *sem = queue_bottom(&free_list, struct semaph, node);
  sem->blocked = (link)LIST_HEAD_INIT(sem->blocked);
  sem->value = count;
  queue_del(sem, node);
  sem->in_use = true;
  proc *p = get_current_process();
  queue_add(sem, &p->owned_semaphores, struct semaph, node, in_use);

  return sem->sid;
}

int sdelete(int sem)
{
  if (sem < 0 || sem >= MAXNBR_SEM || !list_sem[sem].in_use) return -1;

  const bool sched = !queue_empty(&list_sem[sem].blocked);
  while (!queue_empty(&list_sem[sem].blocked)) {
    proc *p = queue_out(&list_sem[sem].blocked, proc, node);
    assert(p->state == BLOCKED);
    p->sjustdelete = true;
    p->state = READY;
    set_ready(p);
  }

  queue_del(&list_sem[sem], node);
  list_sem[sem].in_use = false;
  queue_add(&list_sem[sem], &free_list, struct semaph, node, sid);

  if (sched) schedule();

  return 0;
}

int signal(int sem)
{
  return signaln(sem, 1);
}

int signaln(int sem, short int count)
{
  if (sem < 0 || sem >= MAXNBR_SEM || !list_sem[sem].in_use) return -1;
  if (count <= 0) return -1;                              // this is unspecified
  if (list_sem[sem].value > INT16_MAX - count) return -2; // overflow check

  // if the semaphore isn't blocking anyone, increment and return
  struct semaph *s = &list_sem[sem];
  if (queue_empty(&s->blocked)) {
    assert(s->value >= 0);
    s->value += count;
    return 0;
  }

  // otherwise, for each signal and while someone is blocked, unblock them
  while (count > 0 && !queue_empty(&s->blocked)) {
    proc *p = queue_out(&s->blocked, proc, node);
    assert(p->state == BLOCKED);
    p->state = READY;
    set_ready(p);
    --count;
  }

  // check for left-over signals after unblockings
  if (queue_empty(&s->blocked) && count >= 0) s->value = count;

  schedule();

  return 0;
}

int sreset(int sem, short int count)
{
  if (sem < 0 || sem >= MAXNBR_SEM || !list_sem[sem].in_use) return -1;
  if (count < 0) return -1;

  const bool sched = !queue_empty(&list_sem[sem].blocked);
  while (!queue_empty(&list_sem[sem].blocked)) {
    proc *p = queue_out(&list_sem[sem].blocked, proc, node);
    assert(p->state == BLOCKED);
    p->sjustreset = true;
    p->state = READY;
    set_ready(p);
  }

  list_sem[sem].value = count;

  if (sched) schedule();

  return 0;
}

int wait(int sem)
{
  const int try = try_wait(sem);
  if (try != -3) return try; // non-blocking

  // we know for a fact this should block, so block
  proc *p = get_current_process();
  p->sid = sem;          // indicates where is is blocked
  p->sjustreset = false; // flags which are checked just below
  p->sjustdelete = false;
  p->state = BLOCKED;
  queue_add(p, &list_sem[sem].blocked, proc, node, priority);
  schedule();

  // check whether we unblocked because of a delete/reset
  if (get_current_process()->sjustdelete) return -3;
  if (get_current_process()->sjustreset) return -4;

  return 0;
}

int try_wait(int sem)
{
  if (sem < 0 || sem >= MAXNBR_SEM || !list_sem[sem].in_use) return -1;
  if (list_sem[sem].value < INT16_MIN + 1) return -2; // overflow

  if (!queue_empty(&list_sem[sem].blocked) || list_sem[sem].value <= 0)
    return -3; // would have blocked

  list_sem[sem].value -= 1; // non-blocking
  return 0;
}

int scount(int sem)
{
  if (sem < 0 || sem >= MAXNBR_SEM || !list_sem[sem].in_use) return -1;

  // if the semaphore isn't blocking anyone, return its value
  struct semaph *s = &list_sem[sem];
  if (queue_empty(&s->blocked)) {
    assert(s->value >= 0);
    return (unsigned int)s->value & 0x0000ffff;
  }

  // otherwise, count blocks
  short int count = 0;
  proc *    p;
  queue_for_each(p, &list_sem[sem].blocked, proc, node)
  {
    count--;
  }
  return (unsigned int)count & 0x0000ffff;
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
  queue_add(p, &list_sem[p->sid].blocked, proc, node, priority);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/
