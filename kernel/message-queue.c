/*
 * message-queue.c
 *
 *  Created on: 10/03/2021
 *      Authors: Maxime Martin
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "mem.h"
#include "stddef.h"
#include "process.h"
#include "message-queue.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

// Total number of message queues
#define NBQUEUE 10

// Adds indice pointed to by P into the unused indices queue
#define MQUEUE_UNUSED_ID_ADD(p) queue_add((p), &indice_unused_gestion, struct indice_queue_tab, node_indice, priority)
// References the highest-priority unused indice in the queue
#define MQUEUE_UNUSED_ID_PRIOR() (queue_top(&indice_unused_gestion, struct indice_queue_tab, node_indice))
// Remove the hightest-priority unused indice of the queue
#define MQUEUE_UNUSED_ID_REMOVE() queue_out(&indice_unused_gestion, struct indice_queue_tab, node_indice)

// Adds indice pointed to by P into the used indices queue
#define MQUEUE_USED_ID_ADD(p) queue_add((p), &indice_used_gestion, struct indice_queue_tab, node_indice, priority)
// References the highest-priority used indice in the queue
#define MQUEUE_USED_ID_PRIOR() (queue_top(&indice_used_gestion, struct indice_queue_tab, node_indice))
// Remove the hightest-priority used indice of the queue
#define MQUEUE_USED_ID_REMOVE() queue_out(&indice_used_gestion, struct indice_queue_tab, node_indice)

/*******************************************************************************
 * Types
 ******************************************************************************/

 typedef struct _proc proc;

 struct indice_queue_tab {
   int indice;
   int priority;
   link node_indice;
 };

 struct message_queue {
   int *m_queue;
   int lenght;
   int id_send;
   int nb_send;
   int id_received;
   link waiting_to_send;
   link waiting_to_receive;
 };

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

void init_indice_gestion_list();

int valid_fid(int fid);

/*******************************************************************************
 * Variables
 ******************************************************************************/

struct message_queue* queue_tab[NBQUEUE];

static link indice_unused_gestion;

static link indice_used_gestion;

/*******************************************************************************
 * Public function
 ******************************************************************************/

 // Initialization of the queue, dealing with indice gestion in queue_tab
 void init_indice_gestion_list(){
   indice_unused_gestion = (link)LIST_HEAD_INIT(indice_unused_gestion);
   indice_used_gestion = (link)LIST_HEAD_INIT(indice_used_gestion);
   for (int i = 0; i < NBQUEUE; ++i){
     struct indice_queue_tab* indice_list = mem_alloc(sizeof(struct indice_queue_tab));
     indice_list->indice = i;
     indice_list->priority = NBQUEUE - 1 - i;
     MQUEUE_UNUSED_ID_ADD(indice_list);
   }
 }

// creats a new message-queue, with a lenght of size 'count'
 int pcreate(int count){
   // if the lenght "count" is not valid or if there is not space to create a
   // new messsage queue, return -1
   if(count <= 0 || queue_empty(&indice_unused_gestion)){
     return -1;
   }

   // we get the lowest possible indice to create the new message queue
   int fid = ((struct indice_queue_tab *)MQUEUE_UNUSED_ID_PRIOR())->indice;

   // we initialize the new message queue
   queue_tab[fid] = mem_alloc(sizeof(struct message_queue));
   queue_tab[fid]->m_queue = mem_alloc(sizeof(int)*count);
   queue_tab[fid]->lenght = count;
   queue_tab[fid]->id_send = -1;
   queue_tab[fid]->nb_send = 0;
   queue_tab[fid]->id_received = -1;
   queue_tab[fid]->waiting_to_send = (link)LIST_HEAD_INIT(queue_tab[fid]->waiting_to_send);
   queue_tab[fid]->waiting_to_receive = (link)LIST_HEAD_INIT(queue_tab[fid]->waiting_to_receive);

   // the structure corresponding to the indice we just used is move from the
   // queue of the unused indice to the queue of the used indice
   MQUEUE_USED_ID_ADD(MQUEUE_UNUSED_ID_REMOVE());

   // we return the valid fid to use the new message queue
   return fid;
 }

// sends the message 'message' in the message queue identified by fid
 int psend(int fid, int message){
   // if the given fid is incorrect, return -1
   if(valid_fid(fid) != 0){
     return -1;
   }

   // if the queue is empty, and there are processes waiting to receive a
   // message, we serve the highest priority process
   if(queue_tab[fid]->nb_send == 0 && queue_empty(&queue_tab[fid]->waiting_to_receive) == 0){
     queue_tab[fid]->nb_send ++;
     queue_tab[fid]->id_send = (queue_tab[fid]->id_send + 1) % queue_tab[fid]->lenght;
     queue_tab[fid]->m_queue[queue_tab[fid]->id_send] = message;
     proc* p_to_receive = queue_out(&queue_tab[fid]->waiting_to_receive, proc, node);
     p_to_receive->state = READY;
     queue_add(p_to_receive, &ready_procs, proc, node, priority);
     schedule();
     return 0;
   }

   // if the queue is full, the current process is blocked, waiting to send its
   // message
   else if(queue_tab[fid]->nb_send == queue_tab[fid]->lenght){
     proc* active_process = get_current_process();
     active_process->state = AWAITING_IO;
     queue_add(active_process, &queue_tab[fid]->waiting_to_send, proc, node, priority);
     schedule();
     return 0;
   }
   // if the queue is not full, we save the message and return 0
   else {
     queue_tab[fid]->nb_send ++;
     queue_tab[fid]->id_send = (queue_tab[fid]->id_send + 1) % queue_tab[fid]->lenght;
     queue_tab[fid]->m_queue[queue_tab[fid]->id_send] = message;
     return 0;
   }

   return -1;
 }

// recieves a message from the queue identified by fid
 int preceive(int fid, int *message){
   // if the given fid is incorrect, return -1
   if(valid_fid(fid) != 0){
     return -1;
   }

   // if the queue is full and there are processes waiting to send a message,
   // we serve the highest priority process
   if(queue_tab[fid]->nb_send == queue_tab[fid]->lenght && queue_empty(&queue_tab[fid]->waiting_to_send) == 0){
     queue_tab[fid]->nb_send --;
     queue_tab[fid]->id_received = (queue_tab[fid]->id_received + 1) % queue_tab[fid]->lenght;
     *message = queue_tab[fid]->id_received;
     proc* p_to_send = queue_out(&queue_tab[fid]->waiting_to_send, proc, node);
     p_to_send->state = READY;
     queue_add(p_to_send, &ready_procs, proc, node, priority);
     schedule();
     return 0;
   }

   // if the queue is empty, the current process is blocked, waiting to receive
   // a message
   if(queue_tab[fid]->nb_send == 0){
     proc* active_process = get_current_process();
     active_process->state = AWAITING_IO;
     queue_add(active_process, &queue_tab[fid]->waiting_to_receive, proc, node, priority);
     schedule();
     return 0;
   }

   // if the queue is not empty, we get the message and return 0
   if(queue_tab[fid]->nb_send != 0){
     queue_tab[fid]->nb_send --;
     queue_tab[fid]->id_received = (queue_tab[fid]->id_received + 1) % queue_tab[fid]->lenght;
     *message = queue_tab[fid]->id_received;
     return 0;
   }

   return -1;
 }

// deletes the message queue identified by fid
 int pdelete(int fid){
   // if the given fid is incorrect, return -1
   if(valid_fid(fid) != 0){
     return -1;
   }

   remove_waiting_processes(fid);

   // we delete the message queue corresponding to fid
   mem_free(queue_tab[fid]->m_queue, queue_tab[fid]->lenght*sizeof(int));
   mem_free(queue_tab[fid], sizeof(struct message_queue));

   // the fid indice can now be used by a new message queue
   struct indice_queue_tab* indice_iterator;
   queue_for_each(indice_iterator, &indice_used_gestion, struct indice_queue_tab, node_indice){
     if(indice_iterator->indice == fid){
       queue_del(indice_iterator, node_indice);
       MQUEUE_UNUSED_ID_ADD(indice_iterator);
       return 0;
     }
   }

   return -1;
 }

 // resets the message queue identified by fid
 int preset(int fid){
   // if the given fid is incorrect, return -1
   if(valid_fid(fid) != 0){
     return -1;
   }

   remove_waiting_processes(fid);

   // we reset all the attributes of the message queue fid
   mem_free(queue_tab[fid]->m_queue, queue_tab[fid]->lenght * sizeof(int));
   queue_tab[fid]->m_queue = mem_alloc(sizeof(int)*queue_tab[fid]->lenght);
   queue_tab[fid]->id_send = -1;
   queue_tab[fid]->nb_send = 0;
   queue_tab[fid]->id_received = -1;
   return 0;
 }

 int pcount(int fid, int *count){
   // if the given fid is incorrect, return -1
   if(valid_fid(fid) != 0){
     return -1;
   }
   if(pcount != 0 && queue_empty(queue_tab[fid]->waiting_to_receive) == 0){
     struct indice_queue_tab* indice_iterator;
     *pcount = 0;
     queue_for_each(indice_iterator, &queue_tab[fid]->waiting_to_receive, proc, node){
       *pcount ++;
     }
     *pcount = - *pcount;
  } else if(pcount != 0 && queue_empty(queue_tab[fid]->waiting_to_send) == 0){
    struct indice_queue_tab* indice_iterator;
    *pcount = queue_tab[fid]->nb_send;
    queue_for_each(indice_iterator, &queue_tab[fid]->waiting_to_send, proc, node){
      *pcount ++;
    }
  }
  return 0;
 }

/*******************************************************************************
 * Internal function
 ******************************************************************************/

// Return 0 if the given fid is valid, -1 otherwise
int valid_fid(int fid){
  struct indice_queue_tab* indice_iterator;
  queue_for_each(indice_iterator, &indice_used_gestion, struct indice_queue_tab, node_indice){
    if(fid == indice_iterator->indice){
      return 0;
    }
  }
  return -1;
}

void remove_waiting_processes(int fid){
  // All the processes wainting to send or to receive a message are
  // freed. They have a negative return value from psend or preceive
  proc* blocked_send;
  while(queue_empty(&queue_tab[fid]->waiting_to_send) == 0){
    blocked_send = queue_out(&queue_tab[fid]->waiting_to_send, proc, node);
    blocked_send->state = READY;
    queue_add(blocked_send, &ready_procs, proc, node, priority);
  }

  proc* blocked_receive;
  while(queue_empty(&queue_tab[fid]->waiting_to_receive) == 0){
    blocked_receive = queue_out(&queue_tab[fid]->waiting_to_receive, proc, node);
    blocked_receive->state = READY;
    queue_add(blocked_receive, &ready_procs, proc, node, priority);
  }
}
