/*
 * message-queue.h
 *
 *  Created on: 10/03/2021
 *      Authors: Maxime Martin
 */

#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

/*******************************************************************************
 * Includes
 ******************************************************************************/

 #include "debug.h"
 #include "queue.h"
 #include "cpu.h"
 #include "clock.h"
 #include "console.h"
 #include "process.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

 // Total number of message queues
 #define NBQUEUE 10

 #define MAXMEMSIZE 0xF424000

/*******************************************************************************
 * Types
 ******************************************************************************/

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
 * Variables
 ******************************************************************************/

 struct message_queue* queue_tab[NBQUEUE];

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

 void init_indice_gestion_list();

 int pcreate(int count);

 int pdelete(int fid);

 int psend(int fid, int message);

 int preceive(int fid, int *message);

 int preset(int fid);

 int pcount(int fid, int *count);

 void changing_proc_prio(proc* p);

 void print_list(int fid);

 void print_waiting_send_proc(int fid);

 void print_waiting_receive_proc(int fid);

#endif /* MESSAGE_QUEUE_H */
