/*
 * process.h
 *
 *  Created on: 11/02/2021
 *      Authors: Antoine Briançon, Thibault Cantori
 */

#ifndef _process_H_
#define _process_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "stdint.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

// TODO make sure that kernel will support 1000 processes by the end of project
#define NBPROC  30
#define MAXPRIO 256
#define SCHEDFREQ 100

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*
 * Create a process
 * pt_func : main function of process
 * ssize : size of stack
 * prio : priority of process for execution
 * name : name of process
 * arg : arguments passed to the main function pt_func
 */
int start(int (*pt_func)(void *), unsigned long ssize, int prio,
          const char *name, void *arg);

/*
 * Initialize the system with the main process "idle"
 * For now, also creates two other processes A and B
 */
void process_init();

/*
 * Change priority of process referenced by pid to the value newprio
 * If priority changed and the process was in a queue, it needs to be placed
 * again in that queue depending on its new priority.
 * If the value of newprio is invalid, return value must be < 0. Otherwise,
 * return value is the previous priority of process
 */
int chprio(int pid, int newprio);

/*
 * If value of pid is invalid, return value must be < 0. Otherwise, return value
 * is the current priority of process referenced by pid
 */
int getprio(int pid);

/*
 * Returns pid of calling process
 */
int getpid(void);

/*
 * First draft of scheduling function
 * For now, there are only three differents processes (idle, A and B)
 */
void schedule();

/*
  print all children's name of the chosen process
*/
void show_children(void);

/*
  print all parent's name of the chosen process if it has one
*/
void show_parent(void);

#endif /* _process_H_ */
