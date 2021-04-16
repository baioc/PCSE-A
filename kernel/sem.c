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
  // from sem.h
  typedef struct _semaph semaph;
  typedef struct _sid_queue sid_queue;

  // from process.h
  typedef struct _proc proc;
 /*******************************************************************************
  * Internal function declaration
  ******************************************************************************/
  static int is_used(int sem);
 /*******************************************************************************
  * Variables
  ******************************************************************************/
  int nbr_sem = 0; // number of semaphore created and not deleted

  semaph list_sem[MAXNBR_SEM]; // semaphore tab

  static link unused_sid; // list of unused semaphore id
  static link used_sid; // list of used semaphore id

  // from process.h
  extern link ready_procs;
  extern proc *current_process;
 /*******************************************************************************
  * Public function
  ******************************************************************************/
  /*
  This function is called only one time, in process_init from process.c
  It init the lists of used and unused id (used_sid will remain empty of
  course)
  It is useful to know which sids are used by active semaphore and which
  are unused
  */
  void init_indice_sem(){
    unused_sid = (link)LIST_HEAD_INIT(unused_sid);
    used_sid = (link)LIST_HEAD_INIT(used_sid);
    for (int i = 0; i < MAXNBR_SEM; i++){
      sid_queue *indice_list = mem_alloc(sizeof(sid_queue));
      if(indice_list == NULL){
        exit(1);
      }
      indice_list->id = i;
      indice_list->prio = MAXNBR_SEM - 1 - i;
      queue_add(indice_list, &unused_sid, sid_queue, node_sid, prio);
    }
  }
  /*
  Create a semaphore with a counter value of count
  Returns -1 if the value of count is negative or if we cannot allocate more
  semaphores
  Otherwise, it returns the sid of the semaphore created
  */
  int screate(short int count){
    if(nbr_sem == MAXNBR_SEM || count < 0) return -1;
    sid_queue *indice_list = queue_out(&unused_sid, sid_queue, node_sid);
    queue_add(indice_list, &used_sid, sid_queue, node_sid, prio);
    list_sem[indice_list->id].sid = indice_list->id;
    list_sem[indice_list->id].count = count;
    list_sem[indice_list->id].list_blocked =
              (link)LIST_HEAD_INIT(list_sem[indice_list->id].list_blocked);
    nbr_sem++;
    return indice_list->id;
  }

  /*
  Delete the semaphore list_sem[sem] and free all the process in it list
  Returns -1 if sem arg is wrong
  Returns -2 if the sid index management is wrong
  Otherwise it returns 0 if the semaphore has been properly deleted
  */
  int sdelete(int sem){
    if(sem >= MAXNBR_SEM || sem < 0 || is_used(sem) == 0) return -1;
    proc *p;
    // First we free all the process blocked by this semaphore
    while(queue_empty(&(list_sem[sem].list_blocked)) == 0){
      p = queue_out(&(list_sem[sem].list_blocked), proc, blocked);
      p->state = READY;
      p->sjustdelete = 1; // useful for wait function
      queue_add(p, &ready_procs, proc, node, priority);
    }
    // We move the sid index from used_sid to unused_sid
    sid_queue *iterator;
    queue_for_each(iterator, &used_sid, sid_queue, node_sid){
      if(iterator->id == sem){
        queue_del(iterator, node_sid);
        queue_add(iterator, &unused_sid, sid_queue, node_sid, prio);
        nbr_sem--;
        schedule();
        return 0;
      }
    }
    return -2; // We are not supposed to reach this line
  }

  /*
  Do the V operation on semaphore list_sem[sem]
  Returns -1 if sem arg is wrong
  Returns -2 if we excess counter capacity
  Returns -3 if counter is negative but no process are blocked
  Otherwise it returns 0 if the operation went well
  */
  int signal(int sem){
    if(sem < 0 || sem >= MAXNBR_SEM || is_used(sem) == 0) return -1;
    if((short int)(list_sem[sem].count + 1) < list_sem[sem].count) return -2;
    list_sem[sem].count += 1;
    // if count <= 0 after increment, unlock the process with the highest prio
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
  Like the signal function but we do not call schedule before all the counter
  increment (and process releases if needed) are done
  Same return values than signal
  */
  int signaln(int sem, short int count){
    if(sem < 0 || sem >= MAXNBR_SEM || is_used(sem) == 0) return -1;
    if((short int)(list_sem[sem].count + count) < list_sem[sem].count) return -2;
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
  Reset the semaphore list_sem[sem] and set it counter with a new value (count)
  
  */
  int sreset(int sem,short int count){
    if(sem >= MAXNBR_SEM || sem < 0 || count < 0 || is_used(sem) == 0) return -1;
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
    if(sem >= MAXNBR_SEM || sem < 0 || is_used(sem) == 0) return -1;
    if((short int)(list_sem[sem].count - 1) > list_sem[sem].count) return -2;
    if(list_sem[sem].count <= 0) return -3;
    list_sem[sem].count -= 1;
    return 0;
  }

  /*
  Do the P operation on semaphore list_sem[sem]
  */
  int wait(int sem){
    if(sem >= MAXNBR_SEM || sem < 0 || is_used(sem) == 0) return -1;
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
    if(sem >= MAXNBR_SEM || sem < 0 || is_used(sem) == 0) return -1;
    return (int)(list_sem[sem].count) & 0x0000ffff;
  }
 /*******************************************************************************
  * Internal function
  ******************************************************************************/
  static int is_used(int sem){
    sid_queue *iterator;
    queue_for_each(iterator, &used_sid, sid_queue, node_sid){
      if(iterator->id == sem){
        return 1;
      }
    }
    return 0;
  }
