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
#include "message-queue.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

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

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

// Return 0 if the given fid is valid, -1 otherwise
int valid_fid(int fid);

// Different steps needed to send a message into a message queue
void sending_message(int fid, int message);

// Different steps needed to receive a message from a message queue
void receiving_message(int fid, int* message);

// Different steps needed to remove waiting processes on a message queue
void remove_waiting_processes(int fid, int value);

/*******************************************************************************
 * Variables
 ******************************************************************************/

 static link indice_unused_gestion;

 static link indice_used_gestion;

/*******************************************************************************
 * Public function
 ******************************************************************************/

 int init_indice_gestion_list(){
   indice_unused_gestion = (link)LIST_HEAD_INIT(indice_unused_gestion);
   indice_used_gestion = (link)LIST_HEAD_INIT(indice_used_gestion);
   for (int i = 0; i < NBQUEUE; ++i){
     struct indice_queue_tab* indice_list = mem_alloc(sizeof(struct indice_queue_tab));
     if(indice_list == NULL){
       return -1;
     }
     indice_list->indice = i;
     indice_list->priority = NBQUEUE - 1 - i;
     MQUEUE_UNUSED_ID_ADD(indice_list);
   }
   return 0;
 }


 int pcreate(int count){
   // if the lenght "count" is not valid or if there is not space to create a
   // new messsage queue, return -1
   if(count <= 0 || queue_empty(&indice_unused_gestion) || MAXMEMSIZE < count){
     return -1;
   }

   // we get the lowest possible indice to create the new message queue
   int fid = ((struct indice_queue_tab *)MQUEUE_UNUSED_ID_PRIOR())->indice;

   // we initialize the new message queue
   queue_tab[fid] = mem_alloc(sizeof(struct message_queue));
   if(queue_tab[fid] == NULL){
     return -1;
   }

   queue_tab[fid]->m_queue = mem_alloc(sizeof(int)*count);
   if(queue_tab[fid]->m_queue == NULL){
     mem_free(queue_tab[fid]->m_queue, queue_tab[fid]->lenght*sizeof(int));
     return -1;
   }

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


 int psend(int fid, int message){
   // if the given fid is incorrect, return -1
   if(valid_fid(fid) != 0){
     return -1;
   }

   get_current_process()->m_queue_fid = fid;
   get_current_process()->next_sending_message = message;

   // if the queue is empty, and there are processes waiting to receive a
   // message, we serve the highest priority process
   if(queue_tab[fid]->nb_send == 0 && queue_empty(&queue_tab[fid]->waiting_to_receive) == 0){
     sending_message(fid, message);
     proc* p_to_receive = queue_out(&queue_tab[fid]->waiting_to_receive, proc, node);
     int* msg = p_to_receive->next_receiving_message;
     receiving_message(fid, msg);
     p_to_receive->state = READY;
     queue_add(p_to_receive, &ready_procs, proc, node, priority);
     schedule();
     return 0;
   }

   // else if the queue is full, the current process is blocked, waiting to
   // send its message through a consuming process
   else if(queue_tab[fid]->nb_send == queue_tab[fid]->lenght){
     proc* active_process = get_current_process();
     active_process->state = AWAITING_IO;
     queue_add(active_process, &queue_tab[fid]->waiting_to_send, proc, node, priority);
     schedule();
     if(get_current_process()->m_queue_rd_send == 1){
       get_current_process()->m_queue_rd_send = 0;
       return -1;
     } else {
       return 0;
     }
   }

   // else we save the message and return 0
   else {
     sending_message(fid, message);
     return 0;
   }
   return -1;
 }


 int preceive(int fid, int *message){
   // if the given fid is incorrect, return -1
   if(valid_fid(fid) != 0){
     return -1;
   }

   get_current_process()->m_queue_fid = fid;
   get_current_process()->next_receiving_message = message;

   // if the queue is full and there are processes waiting to send a message,
   // we directly consum and send the waiting message
   if(queue_tab[fid]->nb_send == queue_tab[fid]->lenght && queue_empty(&queue_tab[fid]->waiting_to_send) == 0){
     receiving_message(fid, message);
     proc* p_to_send = queue_out(&queue_tab[fid]->waiting_to_send, proc, node);
     int msg = p_to_send->next_sending_message;
     sending_message(fid, msg);
     p_to_send->state = READY;
     queue_add(p_to_send, &ready_procs, proc, node, priority);
     schedule();
     return 0;
   }

   // else if the queue is empty, the current process is blocked, waiting to
   // receive a message
   else if(queue_tab[fid]->nb_send == 0){
     proc* active_process = get_current_process();
     active_process->state = AWAITING_IO;
     queue_add(active_process, &queue_tab[fid]->waiting_to_receive, proc, node, priority);
     schedule();
     if(get_current_process()->m_queue_rd_receive == 1){
       get_current_process()->m_queue_rd_receive = 0;
       return -1;
     } else {
       return 0;
     }
   }

   // else we get the message and return 0
   else {
     receiving_message(fid, message);
     return 0;
   }

   return -1;
 }


 int pdelete(int fid){
   // if the given fid is incorrect, return -1
   if(valid_fid(fid) != 0){
     return -1;
   }

   remove_waiting_processes(fid, 1);

   // we delete the message queue corresponding to fid
   mem_free(queue_tab[fid]->m_queue, queue_tab[fid]->lenght*sizeof(int));
   mem_free(queue_tab[fid], sizeof(struct message_queue));

   // the fid indice can now be used by a new message queue
   struct indice_queue_tab* indice_iterator;
   queue_for_each(indice_iterator, &indice_used_gestion, struct indice_queue_tab, node_indice){
     if(indice_iterator->indice == fid){
       queue_del(indice_iterator, node_indice);
       MQUEUE_UNUSED_ID_ADD(indice_iterator);
       schedule();
       return 0;
     }
   }

   return -1;
 }


 int preset(int fid){
   // if the given fid is incorrect, return -1
   if(valid_fid(fid) != 0){
     return -1;
   }

   remove_waiting_processes(fid, 1);

   // we reset all the attributes of the message queue fid
   mem_free(queue_tab[fid]->m_queue, queue_tab[fid]->lenght * sizeof(int));
   queue_tab[fid]->m_queue = mem_alloc(sizeof(int)*queue_tab[fid]->lenght);
   if(queue_tab[fid]->m_queue == NULL){
     mem_free(queue_tab[fid]->m_queue, queue_tab[fid]->lenght*sizeof(int));
     return -1;
   }
   queue_tab[fid]->id_send = -1;
   queue_tab[fid]->nb_send = 0;
   queue_tab[fid]->id_received = -1;

   schedule();

   return 0;
 }

 int pcount(int fid, int *count){
   // if the given fid is incorrect, return -1
   if(valid_fid(fid) != 0){
     return -1;
   }
   int value = 0;
   if(queue_empty(&queue_tab[fid]->waiting_to_receive) == 0){
     proc* indice_iterator;
     queue_for_each(indice_iterator, &queue_tab[fid]->waiting_to_receive, proc, node){
       value ++;
     }
     *count = - value;
     return 0;
  } else if(queue_empty(&queue_tab[fid]->waiting_to_send) == 0){
    proc* indice_iterator;
    value = queue_tab[fid]->nb_send;
    queue_for_each(indice_iterator, &queue_tab[fid]->waiting_to_send, proc, node){
      value ++;
    }
    *count = value;
    return 0;
  }
  *count = queue_tab[fid]->nb_send;
  return 0;
 }

  void changing_proc_prio(proc* p){
    if(queue_empty(&queue_tab[p->m_queue_fid]->waiting_to_send) != 0){
      queue_del(p, node);
      queue_add((p), &queue_tab[p->m_queue_fid]->waiting_to_receive, proc, node, priority);
    } else {
      queue_del(p, node);
      queue_add((p), &queue_tab[p->m_queue_fid]->waiting_to_send, proc, node, priority);
    }
  }

/*******************************************************************************
 * Internal function
 ******************************************************************************/

int valid_fid(int fid){
  struct indice_queue_tab* indice_iterator;
  queue_for_each(indice_iterator, &indice_used_gestion, struct indice_queue_tab, node_indice){
    if(fid == indice_iterator->indice){
      return 0;
    }
  }
  return -1;
}

void sending_message(int fid, int message){
  queue_tab[fid]->nb_send ++;
  queue_tab[fid]->id_send = (queue_tab[fid]->id_send + 1) % queue_tab[fid]->lenght;
  queue_tab[fid]->m_queue[queue_tab[fid]->id_send] = message;
}

void receiving_message(int fid, int* message){
  queue_tab[fid]->nb_send --;
  queue_tab[fid]->id_received = (queue_tab[fid]->id_received + 1) % queue_tab[fid]->lenght;
  *message = queue_tab[fid]->m_queue[queue_tab[fid]->id_received];
}

void remove_waiting_processes(int fid, int value){
  // All the processes wainting to send or to receive a message are
  // freed. They have a negative return value from psend or preceive.
  proc* blocked_send;
  while(queue_empty(&queue_tab[fid]->waiting_to_send) == 0){
    blocked_send = queue_out(&queue_tab[fid]->waiting_to_send, proc, node);
    blocked_send->state = READY;
    blocked_send->m_queue_rd_send = value;
    blocked_send->m_queue_fid = -1;
    queue_add(blocked_send, &ready_procs, proc, node, priority);
  }

  proc* blocked_receive;
  while(queue_empty(&queue_tab[fid]->waiting_to_receive) == 0){
    blocked_receive = queue_out(&queue_tab[fid]->waiting_to_receive, proc, node);
    blocked_receive->state = READY;
    blocked_receive->m_queue_rd_receive = value;
    blocked_receive->m_queue_fid = -1;
    queue_add(blocked_receive, &ready_procs, proc, node, priority);
  }
}
