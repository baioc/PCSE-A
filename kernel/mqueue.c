/*
 * mqueue.c
 *
 *  Created on: 10/03/2021
 *      Authors: Maxime Martin, baioc
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "mqueue.h"
#include "pm.h"

#include "stddef.h"
#include "debug.h"
#include "mem.h"
#include "queue.h"
#include "stdbool.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/// Total number of message queues.
#define NBQUEUE 256

/*******************************************************************************
 * Types
 ******************************************************************************/

typedef struct proc proc;

struct message_queue {
  int  fid;
  bool in_use;
  link node;

  // FIFO buffer and its related fields
  int *buffer;
  int  lenght;      // buffer capacity
  int  nb_send;     // used slots
  int  id_send;     // back index
  int  id_received; // front index

  // blocked processes
  link waiting_to_send;
  link waiting_to_receive;
};

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

static bool valid_fid(int fid);

// Send a message into a message queue
static void sending_message(int fid, int message);

// Receive a message from a message queue
static void receiving_message(int fid, int *message);

// Unblock all processes waiting on a message queue
static void remove_waiting_processes(int fid);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static struct message_queue queue_tab[NBQUEUE];
static link                 unused_queues;

/*******************************************************************************
 * Public function
 ******************************************************************************/

void mq_init(void)
{
  unused_queues = (link)LIST_HEAD_INIT(unused_queues);
  for (int i = NBQUEUE - 1; i >= 0; --i) {
    struct message_queue *mq = &queue_tab[i];
    mq->fid = i;
    mq->in_use = false;
    queue_add(mq, &unused_queues, struct message_queue, node, fid);
  }
}

int pcreate(int count)
{
  if (count <= 0 || count > INT32_MAX / (int)sizeof(int)) return -1;
  if (queue_empty(&unused_queues)) return -1;

  struct message_queue *mq = queue_bottom(&unused_queues, struct message_queue, node);
  mq->buffer = mem_alloc(sizeof(int) * count);
  if (mq->buffer == NULL) return -1;
  mq->lenght = count;
  mq->nb_send = 0;
  mq->id_send = 0;
  mq->id_received = 0;
  mq->waiting_to_send = (link)LIST_HEAD_INIT(mq->waiting_to_send);
  mq->waiting_to_receive = (link)LIST_HEAD_INIT(mq->waiting_to_receive);

  // remove queue from free list and reuse link in owned process
  queue_del(mq, node);
  mq->in_use = true;
  proc *p = get_current_process();
  queue_add(mq, &p->owned_queues, struct message_queue, node, in_use);

  return mq->fid;
}

int psend(int fid, int message)
{
  if (!valid_fid(fid)) return -1;

  // queue is empty and there are waiting processes
  if (queue_tab[fid].nb_send == 0 &&
      !queue_empty(&queue_tab[fid].waiting_to_receive))
  {
    // send message and yield to the highest-priority waiting proccess
    sending_message(fid, message);
    proc *p_to_receive =
        queue_out(&queue_tab[fid].waiting_to_receive, proc, node);
    assert(p_to_receive->state == AWAITING_IO);
    assert(p_to_receive->m_queue_fid == fid);
    receiving_message(fid, (int *)p_to_receive->message);
    p_to_receive->state = READY;
    set_ready(p_to_receive);
    schedule();
  }

  // else if the queue is full
  else if (queue_tab[fid].nb_send >= queue_tab[fid].lenght)
  {
    // block current process until someone wants to listen
    proc *active_process = get_current_process();
    active_process->message = (void *)message;
    active_process->m_queue_fid = fid;
    active_process->m_queue_rd = false;
    active_process->state = AWAITING_IO;
    queue_add(
        active_process, &queue_tab[fid].waiting_to_send, proc, node, priority);
    schedule();

    // check whether we unblocked because the queue was actually reset/deleted
    if (get_current_process()->m_queue_rd) return -1;
  }

  // else we send off the message and return right away
  else
  {
    sending_message(fid, message);
  }

  return 0;
}

int preceive(int fid, int *message)
{
  if (!valid_fid(fid)) return -1;

  // queue is full and there is at least one process waiting to send
  if (queue_tab[fid].nb_send == queue_tab[fid].lenght &&
      !queue_empty(&queue_tab[fid].waiting_to_send))
  {
    // receive oldest message
    receiving_message(fid, message);

    // immediately fill in the freed slot through one of the blocked emitters
    proc *p_to_send = queue_out(&queue_tab[fid].waiting_to_send, proc, node);
    assert(p_to_send->state == AWAITING_IO);
    assert(p_to_send->m_queue_fid == fid);
    sending_message(fid, (int)p_to_send->message);

    // and then unblock that process
    p_to_send->state = READY;
    set_ready(p_to_send);
    schedule();
  }

  // else if the queue is empty
  else if (queue_tab[fid].nb_send == 0)
  {
    // block current process until someone wants to send a message
    proc *active_process = get_current_process();
    active_process->message = (void *)message;
    active_process->m_queue_fid = fid;
    active_process->m_queue_rd = false;
    active_process->state = AWAITING_IO;
    queue_add(active_process,
              &queue_tab[fid].waiting_to_receive,
              proc,
              node,
              priority);
    schedule();

    // check whether we unblocked because the queue was actually reset/deleted
    if (get_current_process()->m_queue_rd) return -1;
  }

  // else we slurp the message and return right away
  else
  {
    receiving_message(fid, message);
  }

  return 0;
}

int pdelete(int fid)
{
  if (!valid_fid(fid)) return -1;

  // unblock processes
  remove_waiting_processes(fid);

  // free buffer
  mem_free(queue_tab[fid].buffer, queue_tab[fid].lenght * sizeof(int));

  // place queue back in the free list
  queue_del(&queue_tab[fid], node);
  queue_tab[fid].in_use = false;
  queue_add(&queue_tab[fid], &unused_queues, struct message_queue, node, fid);

  return 0;
}

int preset(int fid)
{
  if (!valid_fid(fid)) return -1;

  // unblock processes
  remove_waiting_processes(fid);

  // reset buffer indexes
  queue_tab[fid].nb_send = 0;
  queue_tab[fid].id_send = 0;
  queue_tab[fid].id_received = 0;

  // yield in case there were higher priority process waiting
  schedule();

  return 0;
}

int pcount(int fid, int *count)
{
  if (!valid_fid(fid)) return -1;
  if (count == NULL) return 0; // in case of nullptr, this is a noop

  if (!queue_empty(&queue_tab[fid].waiting_to_receive)) {
    int   value = 0;
    proc *iterator;
    queue_for_each(iterator, &queue_tab[fid].waiting_to_receive, proc, node)
    {
      value -= 1;
    }
    *count = value;

  } else if (!queue_empty(&queue_tab[fid].waiting_to_send)) {
    int   value = queue_tab[fid].nb_send;
    proc *iterator;
    queue_for_each(iterator, &queue_tab[fid].waiting_to_send, proc, node)
    {
      value += 1;
    }
    *count = value;

  } else {
    *count = queue_tab[fid].nb_send;
  }

  return 0;
}

void mq_process_init(struct proc *p)
{
  p->owned_queues = (link)LIST_HEAD_INIT(p->owned_queues);
}

void mq_process_destroy(struct proc *p)
{
  while (!queue_empty(&p->owned_queues)) {
    struct message_queue *mq = queue_out(&p->owned_queues, struct message_queue, node);
    pdelete(mq->fid);
  }
}

void mq_process_chprio(struct proc *p)
{
  assert(p->state == AWAITING_IO);
  assert(valid_fid(p->m_queue_fid));
  struct message_queue *mq = &queue_tab[p->m_queue_fid];
  if (queue_empty(&mq->waiting_to_send)) {
    queue_del(p, node);
    queue_add(p, &mq->waiting_to_receive, proc, node, priority);
  } else {
    queue_del(p, node);
    queue_add(p, &mq->waiting_to_send, proc, node, priority);
  }
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

static inline bool valid_fid(int fid)
{
  return fid >= 0 && fid < NBQUEUE && queue_tab[fid].in_use;
}

static void sending_message(int fid, int message)
{
  assert(valid_fid(fid));
  struct message_queue *mq = &queue_tab[fid];
  assert(mq->nb_send < mq->lenght);
  assert(mq->id_send >= 0 && mq->id_send < mq->lenght);
  mq->buffer[mq->id_send] = message;
  mq->id_send = (mq->id_send + 1) % mq->lenght;
  mq->nb_send++;
}

static void receiving_message(int fid, int *message)
{
  assert(valid_fid(fid));
  struct message_queue *mq = &queue_tab[fid];
  assert(mq->nb_send > 0);
  assert(mq->id_received >= 0 && mq->id_received < mq->lenght);
  if (message != NULL) *message = mq->buffer[mq->id_received];
  mq->id_received = (mq->id_received + 1) % mq->lenght;
  mq->nb_send--;
}

static void remove_waiting_processes(int fid)
{
  assert(valid_fid(fid));

  while (!queue_empty(&queue_tab[fid].waiting_to_send)) {
    proc *blocked_send = queue_out(&queue_tab[fid].waiting_to_send, proc, node);
    assert(blocked_send->state == AWAITING_IO);
    assert(blocked_send->m_queue_fid == fid);
    blocked_send->m_queue_rd = true;
    blocked_send->state = READY;
    set_ready(blocked_send);
  }

  while (!queue_empty(&queue_tab[fid].waiting_to_receive)) {
    proc *blocked_receive =
        queue_out(&queue_tab[fid].waiting_to_receive, proc, node);
    assert(blocked_receive->state == AWAITING_IO);
    assert(blocked_receive->m_queue_fid == fid);
    blocked_receive->m_queue_rd = true;
    blocked_receive->state = READY;
    set_ready(blocked_receive);
  }
}
