/*
 * process.c
 *
 *  Created on: 11/02/2021
 *      Authors: Antoine Briançon, Thibault Cantori
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "process.h"
#include "../shared/queue.h"
#include "../shared/malloc.c"
#include "stdint.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

typedef enum _proc_state {
   CHOSEN, // process currently running on processor
   READY,  // process waiting for his turn
   BLOCKED_ON_SEMAHPORE,
   BLOCKED_ON_IO,
   WAITING_FOR_CHILD,
   SLEEPING,
   ZOMBIE
 }proc_state;

 struct _proc {
     uint32_t pid; // between 1 and NBPROC
     uint32_t priority; // between 1 and MAXPRIO
     proc_state state;
     uint32_t saveZone[5];
     uint32_t ssize;
     const char *name;
     void *arg;
     link position; // useful for the list
     //uint32_t stack[];
     uint32_t stack[TAILLE_PILE];
 };
 typedef struct _proc proc;

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

link list_proc = LIST_HEAD_INIT(list_proc); // process list

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint32_t nbr_proc = 0;

/*******************************************************************************
 * Public function
 ******************************************************************************/

// test schedulding function
void ord(){
  if(nbr_proc > 1){
    proc *pass = queue_out(&list_proc,proc,position); // process who will give the execution
    proc *take = queue_top(&list_proc,proc,position); // process who will take the execution
    pass->priority = MAXPRIO - nbr_proc;
    pass->state = READY;
    take->state = CHOSEN;
    queue_add(pass,&list_proc,proc,position,priority);
    ctx_sw((int*)pass->saveZone,(int*)take->saveZone);
  }
}

int tstA2(){
  while(1){
    printf("A");
    for(uint32_t i = 0; i < 5000000; i++);
    ord();
  }
}

int tstB2(){
  while(1){
    printf("B");
    for(uint32_t i = 0; i < 5000000; i++);
    ord();
  }
}

// test function for the first process
 int tstA(void *arg){
   uint32_t i;
   printf(arg);
   while(1){
     printf("A");
     for(i = 0; i < 5000000; i++);
     ord();
   }
 }

 int tstB(void *arg){
   uint32_t i;
   printf(arg);
   while(1){
     printf("B");
     for(i = 0; i < 5000000; i++);
     ord();
   }
 }

int start(int (*pt_func)(void *), unsigned long ssize, int prio, const char *name, void *arg){
  if(nbr_proc == NBPROC){
    return 1;
  }
  proc new_proc;
  new_proc.pid = nbr_proc;
  nbr_proc ++;
  new_proc.priority = prio;
  new_proc.ssize = (uint32_t)ssize;
  new_proc.stack[TAILLE_PILE-1] = (uint32_t)(pt_func);
  new_proc.saveZone[1] = (uint32_t)(&(new_proc.stack[TAILLE_PILE-1]));
  /*
  new_proc.stack = mem_alloc(ssize*sizeof(uint32_t));
  new_proc.stack[ssize-1] = (uint32_t)(pt_func);
  new_proc.saveZone[1] = (uint32_t)(&(new_proc.stack[ssize-1]));
  */
  new_proc.name = name;
  new_proc.state = READY;
  new_proc.arg = arg;
  queue_add(&new_proc,&list_proc,proc,position,priority);
  return 0;
}

/*
 * Change priority of process referenced by pid to the value newprio
 * If priority changed and the process was in a queue, it needs to be placed
 * again in that queue depending on its new priority.
 * If the value of newprio is invalid, return value must be < 0. Otherwise,
 * return value is the previous priority of process
 */
//int chprio(int pid, int newprio) { return -1; }

/*
 * If value of pid is invalid, return value must be < 0. Otherwise, return value
 * is the current priority of process referenced by pid
 */
//int getprio(int pid) { return -1; }

/*******************************************************************************
 * Internal function
 ******************************************************************************/
