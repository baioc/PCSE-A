/*
 * sem.c
 *
 *  Created on: 03/03/2021
 *      Authors: Antoine Briançon
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
  // from process.c
  extern link ready_procs;
  extern proc *current_process;
 /*******************************************************************************
  * Public function
  ******************************************************************************/
  /*
  Create a semaphore with a counter value of count
  */
  int screate(short int count){
    if(nbr_sem == MAXNBR_SEM || count < 0) return -1;
    //Looking for a blank place to create the semaphore
    int i = 0;
    // We should find a better way to detect if a semaphore isn't initialized
    while(list_sem[i].sid == i){
      i++;
    }
    list_sem[i].sid = i;
    list_sem[i].count = count;
    list_sem[i].list_blocked = (link)LIST_HEAD_INIT(list_sem[i].list_blocked);
    nbr_sem++;
    return i;
  }

  /*
  Delete the semaphore list_sem[sem] and free all the process in it list
  */
  int sdelete(int sem){
    if(sem >= MAXNBR_SEM || sem < 0 || list_sem[sem].sid != sem) return -1;
    proc *p;
    while(queue_empty(&(list_sem[sem].list_blocked)) == 0){
      p = queue_out(&(list_sem[sem].list_blocked), proc, blocked);
      p->state = READY;
      p->sjustdelete = 1;
      queue_add(p, &ready_procs, proc, node, priority);
    }
    list_sem[sem].sid = -1;
    list_sem[sem].count = -1;
    schedule();
    return 0;
  }

  /*
  Do the V operation on semaphore list_sem[sem]
  */
  int signal(int sem){
    if(sem < 0 || sem >= MAXNBR_SEM || list_sem[sem].sid != sem) return -1;
    if((short int)(list_sem[sem].count + 1) < list_sem[sem].count) return -2;
    list_sem[sem].count += 1;
    if(list_sem[sem].count <= 0){
      if(queue_empty(&(list_sem[sem].list_blocked)) != 0) return -3;
      proc *p = queue_out(&(list_sem[sem].list_blocked), proc, blocked);
      p->state = READY;
      queue_add(p, &ready_procs, proc, node, priority);
      schedule();
    }
    return 0;
  }

  /*
  Do the V operation count time on semaphore list_sem[sem]
  */
  int signaln(int sem, short int count){
    if(sem < 0 || sem >= MAXNBR_SEM || list_sem[sem].sid != sem) return -1;
    if((short int)(list_sem[sem].count + count) < list_sem[sem].count)
                                                                      return -2;
    proc *p;
    for(int i = 0; i < count; i++){
      list_sem[sem].count += 1;
      if(list_sem[sem].count <= 0){
        if(queue_empty(&(list_sem[sem].list_blocked)) != 0) return -3;
        p = queue_out(&(list_sem[sem].list_blocked), proc, blocked);
        p->state = READY;
        queue_add(p, &ready_procs, proc, node, priority);
      }
    }
    schedule();
    return 0;
  }

  /*
  Reset the semaphore list_sem[sem]
  */
  int sreset(int sem,short int count){
    if(sem >= MAXNBR_SEM || sem < 0 || count < 0 || list_sem[sem].sid != sem)
                                                                      return -1;
    proc *p;
    while(queue_empty(&(list_sem[sem].list_blocked)) == 0){
      p = queue_out(&(list_sem[sem].list_blocked), proc, blocked);
      p->state = READY;
      p->sjustreset = 1;
      queue_add(p, &ready_procs, proc, node, priority);
    }
    list_sem[sem].count = count;
    schedule();
    return 0;
  }

  /*
  Test the P operation on semaphore list_sem[sem] without blocking it
  */
  int try_wait(int sem){
    if(sem >= MAXNBR_SEM || sem < 0 || list_sem[sem].sid != sem) return -1;
    if((short int)(list_sem[sem].count - 1) > list_sem[sem].count) return -2;
    if(list_sem[sem].count <= 0) return -3;
    list_sem[sem].count -= 1;
    return 0;
  }

  /*
  Do the P operation on semaphore list_sem[sem]
  */
  int wait(int sem){
    if(sem >= MAXNBR_SEM || sem < 0 || list_sem[sem].sid != sem) return -1;
    if((short int)(list_sem[sem].count - 1) > list_sem[sem].count) return -2;
    list_sem[sem].count -= 1;
    if(list_sem[sem].count < 0){
      current_process->state = BLOCKED;
      current_process->sid = sem;
      current_process->sjustreset = 0;
      current_process->sjustdelete = 0;
      queue_add(current_process, &(list_sem[sem].list_blocked), proc,
                                                            blocked, priority);
      schedule();
      if(current_process->sjustdelete == 1) return -3; // After a sdelete
      if(current_process->sjustreset == 1) return -4; // After a sreset
    }
    return 0;
  }

  /*
  Return the value of the semaphore list_sem[sem]
  */
  int scount(int sem){
    if(sem >= MAXNBR_SEM || sem < 0 || list_sem[sem].sid != sem) return -1;
    return (int)(list_sem[sem].count) & 0x0000ffff;
  }
 /*******************************************************************************
  * Internal function
  ******************************************************************************/
