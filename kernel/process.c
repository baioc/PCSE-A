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

#include "stdint.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

// TODO make sure that kernel will support 1000 processes by the end of project
#define NBPROC  30
#define MAXPRIO 256

/*******************************************************************************
 * Types
 ******************************************************************************/

typedef enum process_state {
   CHOSEN, // process currently running on processor
   READY,  // process waiting for his turn
   BLOCKED_ON_SEMAHPORE,
   BLOCKED_ON_IO,
   WAITING_FOR_CHILD,
   SLEEPING,
   ZOMBIE
 }

 struct process {
     uint32_t pid; // between 1 and NBPROC
     uint32_t priority; // between 1 and MAXPRIO
     char name[8];
     process_state state;
     uint32_t saveZone[5];
     uint32_t stack[TAILLE_PILE];
     int endormi;
 };

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

/*
  Stocker les processus dans une liste chaînée
*/

int start(int (*pt_func)(void *), unsigned long ssize, int prio, const char *name, void *arg){

}

/*
 * Change priority of process referenced by pid to the value newprio
 * If priority changed and the process was in a queue, it needs to be placed
 * again in that queue depending on its new priority.
 * If the value of newprio is invalid, return value must be < 0. Otherwise,
 * return value is the previous priority of process
 */
int chprio(int pid, int newprio) { return -1; }

/*
 * If value of pid is invalid, return value must be < 0. Otherwise, return value
 * is the current priority of process referenced by pid
 */
int getprio(int pid) { return -1; }

/*******************************************************************************
 * Internal function
 ******************************************************************************/
