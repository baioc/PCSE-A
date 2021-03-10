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

 /*******************************************************************************
  * Internal function declaration
  ******************************************************************************/
 /*******************************************************************************
  * Variables
  ******************************************************************************/
  extern link ready_procs;
 /*******************************************************************************
  * Public function
  ******************************************************************************/
  /*
  Create a semaphore
  */
  int screate(short int count);
  /*
  Delete a semaphore
  */
  int sdelete(int sem);
  int sreset(semaph sem,short int count);
  int signal(int sem);
  int signaln(int sem,short int count);
  int try_wait(int sem);
  int wait(int sem);
  int scount(semaph sem);
 /*******************************************************************************
  * Internal function
  ******************************************************************************/

#endif /* _sem_H_ */
