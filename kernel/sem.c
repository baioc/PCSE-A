/*
 * sem.c
 *
 *  Created on: 03/03/2021
 *      Authors: Antoine Briançon, Thibault Cantori
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
 #include "sem.h"
 /*******************************************************************************
  * Macros
  ******************************************************************************/
 /*******************************************************************************
  * Types
  ******************************************************************************/
  typedef struct _semaph semaph;

  typedef struct _proc proc;
 /*******************************************************************************
  * Internal function declaration
  ******************************************************************************/
 /*******************************************************************************
  * Variables
  ******************************************************************************/
  int nbr_sem = 0;

  semaph list_sem[MAXNBR_SEM];
 /*******************************************************************************
  * Public function
  ******************************************************************************/
  /*
  Create a semaphore
  */
  int screate(short int count){
    if(nbr_sem == MAXNBR_SEM || count < 0) return -1;
    //Looking for a blank place to create the semaphore
    int i = 0;
    while(list_sem[i].sid == i){ // We should find a better way to detect if a semaphore isn't initialized
      i++;
    }
    list_sem[i].sid = i;
    list_sem[i].count = count;
    list_sem[i].list_blocked = (link)LIST_HEAD_INIT(list_sem[i].list_blocked);
    nbr_sem++;
    return i;
  }

  /*
  Delete a semaphore
  */
  int sdelete(int sem){
    if(sem >= MAXNBR_SEM || sem < 0) return -1;
    proc *p;
    while(queue_empty(&(list_sem[sem].list_blocked)) == 0){
      p = queue_out(list_sem[sem].list_blocked, proc, blocked);
      p->state = READY;
      queue_add(p, &ready_procs, proc, node, priority);
    }
    list_sem[sem].sid = -1;
    list_sem[sem].count = -1;
    return 0;
  }
/*
  int sreset(semaph sem,short int count);
  int signal(int sem);
  int signaln(int sem,short int count);
  int try_wait(int sem);
  int wait(int sem);
  int scount(semaph sem);
*/
 /*******************************************************************************
  * Internal function
  ******************************************************************************/
