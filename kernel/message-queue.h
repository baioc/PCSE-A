/*
 * message-queue.h
 *
 *  Created on: 10/03/2021
 *      Author: Maxime Martin
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

// Maximum size of the memory
 #define MAXMEMSIZE 0xF424000

/*******************************************************************************
 * Types
 ******************************************************************************/

 typedef struct _proc proc;

 // Represents an fid index, to manage indexes requiered by message-queues
 struct indice_queue_tab {
   int indice;
   int priority;
   link node_indice;
 };

// Represents a message queue
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

 // Tab of message queue
 struct message_queue* queue_tab[NBQUEUE];

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

 // initializes the gestion of indexes required by message queues
 void init_indice_gestion_list();

 /*
  * Creates a message queue
  * count : size of the message queue we want to create
  * return value is the fid of the newly created message queue, or -1 if it was
  * impossible to create a new message queue
  */
 int pcreate(int count);

 /*
  * Deletes a message queue, frees its structure, all messages are forgotten
  * and processes waiting to send or receive a message from this queue are
  * liberated
  * fid : index of the message queue we want to delete
  * return value is 0 if everything went as expected, -1 if the fid was
  * incorrect or if there was an issue
  */
 int pdelete(int fid);

 /*
  * Sends a message into a message queue
  * fid : index of the message queue we want to send the message to
  * message : the integer we want to send in the message queue
  * return value is 0 if everything went as expected, -1 if the fid was
  * incorrect or if pdelete or preset were executed while we were trying to
  * send the message
  */
 int psend(int fid, int message);

 /*
  * Receives a message from a message queue
  * fid : index of the message queue we want to receive a message from
  * *message : the integer where we stock the message received
  * return value is 0 if everything went as expected, -1 if the fid was
  * incorrect or if pdelete or preset were executed while we were trying to
  * send the message
  */
 int preceive(int fid, int *message);

 /*
  * Resets a message queue, all messages are forgotten and processes waiting
  * to send or receive a message from this queue are liberated
  * fid : index of the message queue we want to reset
  * return value is 0 if everything went as expected, -1 if the fid was
  * incorrect or if pdelete or preset were executed while we were trying to
  * send the message
  */
 int preset(int fid);

 /*
  * Gives the current state of a message queue
  * fid : index of the message queue we want to know the state of
  * *count : the integer where we stock the state of the message queue fid
  * return value is 0 if everything went as expected, -1 if the fid was
  * incorrect
  */
 int pcount(int fid, int *count);

 /*
  * Manages the change of priority for a process whose state is AWAITING_IO
  * *p : the process we want to change the priority of
  */
 void changing_proc_prio(proc* p);


#endif /* MESSAGE_QUEUE_H */
