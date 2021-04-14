/*
 * sem.h
 *
 *  Created on: 11/02/2021
 *      Authors: Antoine Briançon, Thibault Cantori
 */

#ifndef _sem_H_
#define _sem_H_
/*******************************************************************************
 * Includes
 ******************************************************************************/
 #include "cpu.h"
 #include "mem.h"
 #include "process.h"
 #include "queue.h"
 /*******************************************************************************
  * Macros
  ******************************************************************************/
  #define MAXNBR_SEM 256
 /*******************************************************************************
  * Types
  ******************************************************************************/
  struct _semaph{
    short int sid;
    short int count;
    link      list_blocked; // List of process blocked on this semaphore
  };
  typedef struct _semaph semaph;

  // Represents an sid index, to manage indexes requiered by sem
  struct _sid_queue {
    short int id;
    int       prio;
    link      node_sid;
  };
  typedef struct _sid_queue sid_queue;
 /*******************************************************************************
  * Internal function declaration
  ******************************************************************************/
 /*******************************************************************************
  * Variables
  ******************************************************************************/
 /*******************************************************************************
  * Public function
  ******************************************************************************/
  void init_indice_sem();
  /*
  Create a semaphore
  */
  int screate(short int count);
  /*
  Delete a semaphore
  */
  int sdelete(int sem);
  /*
  Reset the semaphore list_sem[sem]
  */
  int sreset(int sem,short int count);
  /*
  Do the V operation on semaphore list_sem[sem]
  */
  int signal(int sem);
  /*
  Do the V operation count time on semaphore list_sem[sem]
  */
  int signaln(int sem,short int count);
  /*
  Test the P operation on semaphore list_sem[sem] without blocking it
  */
  int try_wait(int sem);
  /*
  Do the P operation on semaphore list_sem[sem]
  */
  int wait(int sem);
  /*
  Return the value of the semaphore list_sem[sem]
  */
  int scount(int sem);
 /*******************************************************************************
  * Internal function
  ******************************************************************************/

#endif /* _sem_H_ */
