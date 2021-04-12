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
#define NBQUEUE 200

/*******************************************************************************
 * Types
 ******************************************************************************/

typedef struct proc proc;

struct message_queue {
  int  fid;
  bool in_use;

  // FIFO buffer and its related fields
  int *buffer;
  int  lenght;
  int  nb_send;
  int  id_send;
  int  id_received;

  // blocked processes
  link waiting_to_send;
  link waiting_to_receive;

  struct message_queue *next;
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

static struct message_queue *unused_queues = NULL;

/*******************************************************************************
 * Public function
 ******************************************************************************/

void mq_init(void)
{
  unused_queues = NULL;
  for (int i = NBQUEUE - 1; i >= 0; --i) {
    struct message_queue *mq = &queue_tab[i];
    *mq = (struct message_queue){.fid = i};
    mq->next = unused_queues;
    unused_queues = mq;
    mq->in_use = false;
  }
}

int pcreate(int count)
{
  if (count <= 0 || unused_queues == NULL) return -1;

  // get the first unused queue
  const int fid = unused_queues->fid;
  assert(&queue_tab[fid] == unused_queues);

  // we initialize the new message queue
  queue_tab[fid].buffer = mem_alloc(sizeof(int) * count);
  if (queue_tab[fid].buffer == NULL) return -1;
  queue_tab[fid].lenght = count;
  queue_tab[fid].nb_send = 0;
  queue_tab[fid].id_send = -1;
  queue_tab[fid].id_received = -1;
  queue_tab[fid].waiting_to_send =
      (link)LIST_HEAD_INIT(queue_tab[fid].waiting_to_send);
  queue_tab[fid].waiting_to_receive =
      (link)LIST_HEAD_INIT(queue_tab[fid].waiting_to_receive);

  // remove queue from free list
  unused_queues = unused_queues->next;
  queue_tab[fid].in_use = true;

  return fid;
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
    receiving_message(fid, (int *)p_to_receive->message);
    assert(p_to_receive->state == AWAITING_IO);
    assert(p_to_receive->m_queue_fid == fid);
    p_to_receive->state = READY;
    set_ready(p_to_receive);
    schedule();
  }

  // else if the queue is full
  else if (queue_tab[fid].nb_send == queue_tab[fid].lenght)
  {
    // block current process until someone wants to listen
    proc *active_process = get_current_process();
    active_process->message = message;
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
    sending_message(fid, p_to_send->message);

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
    active_process->message = (int)message;
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
  queue_tab[fid].next = unused_queues;
  unused_queues = &queue_tab[fid];
  queue_tab[fid].in_use = false;

  return 0;
}

int preset(int fid)
{
  if (!valid_fid(fid)) return -1;

  // unblock processes
  remove_waiting_processes(fid);

  // reset buffer indexes
  queue_tab[fid].nb_send = 0;
  queue_tab[fid].id_send = -1;
  queue_tab[fid].id_received = -1;

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
      value++;
    }
    *count = -value;

  } else if (!queue_empty(&queue_tab[fid].waiting_to_send)) {
    int   value = queue_tab[fid].nb_send;
    proc *iterator;
    queue_for_each(iterator, &queue_tab[fid].waiting_to_send, proc, node)
    {
      value++;
    }
    *count = value;

  } else {
    *count = queue_tab[fid].nb_send;
  }

  return 0;
}

void mq_changing_proc_prio(proc *p)
{
  assert(p->state == AWAITING_IO);
  assert(valid_fid(p->m_queue_fid));
  if (queue_empty(&queue_tab[p->m_queue_fid].waiting_to_send)) {
    queue_del(p, node);
    queue_add(
        p, &queue_tab[p->m_queue_fid].waiting_to_receive, proc, node, priority);
  } else {
    queue_del(p, node);
    queue_add(
        p, &queue_tab[p->m_queue_fid].waiting_to_send, proc, node, priority);
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
  queue_tab[fid].nb_send++;
  assert(queue_tab[fid].nb_send <= queue_tab[fid].lenght);
  queue_tab[fid].id_send = (queue_tab[fid].id_send + 1) % queue_tab[fid].lenght;
  queue_tab[fid].buffer[queue_tab[fid].id_send] = message;
}

static void receiving_message(int fid, int *message)
{
  assert(valid_fid(fid));
  assert(queue_tab[fid].nb_send > 0);
  queue_tab[fid].nb_send--;
  queue_tab[fid].id_received =
      (queue_tab[fid].id_received + 1) % queue_tab[fid].lenght;
  if (message != NULL)
    *message = queue_tab[fid].buffer[queue_tab[fid].id_received];
}

static void remove_waiting_processes(int fid)
{
  // All the processes wainting to send or to receive a message are
  // freed. They have a negative return value from psend or preceive.
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
