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
#include "queue.h"
#include "malloc.c"
#include "stdint.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

// Describe different states of a process
typedef enum _proc_state {
  CHOSEN, // process currently running on processor
  READY,  // process waiting for his turn
  BLOCKED_ON_SEMAHPORE,
  BLOCKED_ON_IO,
  WAITING_FOR_CHILD,
  SLEEPING,
  ZOMBIE
} proc_state;

// Describe a process
typedef struct _proc {
  uint32_t      pid;      // between 1 and NBPROC
  uint32_t      priority; // between 1 and MAXPRIO
  proc_state    state;
  uint32_t      save_zone[5]; // used to save context of process
  uint32_t      ssize;
  const char *  name;
  void *        arg;
  link          position; // useful for the list
  uint32_t *    stack;
  struct _proc *parent; // process which created this process (phase 3)
  struct _proc
      *children; // list of processes that this process created (phase 3)
} proc;

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

int idle();
int tstA();
int tstB();

/*
 * Changes context between two processes
 */
extern void ctx_sw(uint32_t save_zone1[5], uint32_t save_zone2[5]);

/*
 * Add process into activable processes list
 */
void proc_list_add(proc *proc_to_add);

/*
 * Remove process from activable processes list
 */
void proc_list_del(proc *proc_to_del);

/*
 * Return first process from activable processes list
 */
proc *proc_list_top();

/*
 * Remove first process from activable processes list
 */
proc *proc_list_out();

/*******************************************************************************
 * Variables
 ******************************************************************************/

// Current number of processes started on system
uint32_t nbr_proc = 0;

// Table of ALL processes. A process is referenced in this table by its pid - 1
// Because pids are numbered from 1 to NBPROC
proc process_table[NBPROC];

// List of activable processes
link list_proc = LIST_HEAD_INIT(list_proc);

// Current process running on processor
proc *chosen_process;

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

/*
 * Add process into activable processes list
 */
void proc_list_add(proc *proc_to_add)
{
  queue_add(proc_to_add, &list_proc, proc, position, priority);
}

/*
 * Remove process from activable processes list
 */
void proc_list_del(proc *proc_to_del)
{
  queue_del(proc_to_del, position);
}

/*
 * Return first process from activable processes list
 */
proc *proc_list_top()
{
  return queue_top(&list_proc, proc, position);
}

/*
 * Remove first process from activable processes list
 */
proc *proc_list_out()
{
  return queue_out(&list_proc, proc, position);
}

/*
 * Main function of idle process
 */
int idle()
{
  while (1) {
    printf("idle");
    schedule();
  }

  return 0;
}

/*
 * Main function of process A
 */
int tstA()
{
  while (1) {
    printf("A");
    for (uint32_t i = 0; i < 5000000; i++)
      ;
    schedule();
  }

  return 0;
}

/*
 * Main function of process B
 */
int tstB()
{
  while (1) {
    printf("B");
    for (uint32_t i = 0; i < 5000000; i++)
      ;
    schedule();
  }

  return 0;
}
