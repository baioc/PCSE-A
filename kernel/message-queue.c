/*
 * m-queue.c
 *
 *  Created on: 4/03/2021
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

// Total number of message queues
#define NBQUEUE 10

// Adds indice pointed to by P into the unused indices queue
#define MQUEUE_UNUSED_ID_ADD(p) queue_add((p), &indice_unused_gestion, struct indice_queue_tab, node, priority)

// References the highest-priority unused indice in the queue
#define MQUEUE_UNUSED_ID_PRIOR() (queue_top(&indice_unused_gestion, struct indice_queue_tab, node))

// Remove the hightest-priority unused indice of the queue
#define MQUEUE_UNUSED_ID_REMOVE() queue_out(&indice_unused_gestion, struct indice_queue_tab, node)

// Adds indice pointed to by P into the used indices queue
#define MQUEUE_USED_ID_ADD(p) queue_add((p), &indice_used_gestion, struct indice_queue_tab, node, priority)

// References the highest-priority used indice in the queue
#define MQUEUE_USED_ID_PRIOR() (queue_top(&indice_used_gestion, struct indice_queue_tab, node))

// Remove the hightest-priority used indice of the queue
#define MQUEUE_USED_ID_REMOVE() queue_out(&indice_used_gestion, struct indice_queue_tab, node)

/*******************************************************************************
 * Types
 ******************************************************************************/

 struct indice_queue_tab {
   int indice;
   int priority;
   link node;
 };

 struct message_queue {
   int *m_queue;
   int lenght;
   int id_send;
   int id_received;
 };

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/

struct message_queue* queue_tab[NBQUEUE];

static link indice_unused_gestion;

static link indice_used_gestion;

/*******************************************************************************
 * Public function
 ******************************************************************************/

 int pcreate(int count){
   if(count <= 0 || queue_empty(&indice_unused_gestion)){
     return -1;
   }
   int fid = ((struct indice_queue_tab *)MQUEUE_UNUSED_ID_PRIOR())->indice;

   queue_tab[fid] = mem_alloc(sizeof(struct message_queue));
   queue_tab[fid]->m_queue = mem_alloc(sizeof(int)*count);
   queue_tab[fid]->lenght = count;
   queue_tab[fid]->id_send = 0;
   queue_tab[fid]->id_received = 0;

   MQUEUE_USED_ID_ADD(MQUEUE_UNUSED_ID_REMOVE());

   return fid;
 }

 void psend(int fid, int message){
   queue_tab[fid]->m_queue[0] = message;
 }

/*******************************************************************************
 * Internal function
 ******************************************************************************/

// Initialization of the queue, dealing with indice gestion in queue_tab
void init_indice_gestion_list(){
  indice_unused_gestion = (link)LIST_HEAD_INIT(indice_unused_gestion);
  indice_used_gestion = (link)LIST_HEAD_INIT(indice_used_gestion);;
  for (int i = 0; i < NBQUEUE; ++i){
    struct indice_queue_tab* indice_list = mem_alloc(sizeof(struct indice_queue_tab));
    indice_list->indice = i;
    indice_list->priority = NBQUEUE - 1 - i;
    MQUEUE_UNUSED_ID_ADD(indice_list);
  }
}

// Print all the indice of the list
void print_unused_indice_gestion_list(){
  struct indice_queue_tab* indice_iterator;
  queue_for_each(indice_iterator, &indice_unused_gestion, struct indice_queue_tab, node){
    printf("%i\n", indice_iterator->indice);
  }
}

int valid_fid(int fid){
  struct indice_queue_tab* indice_iterator;
  queue_for_each(indice_iterator, &indice_used_gestion, struct indice_queue_tab, node){
    if(fid == indice_iterator->indice){
      return 0;
    }
  }
  return -1;
}

void test(){
  printf("%i\n", queue_tab[0]->m_queue[0]);
  printf("%i\n", valid_fid(0));
  printf("%i\n", valid_fid(1));
}
