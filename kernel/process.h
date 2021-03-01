/*
 * process.h
 *
 *  Created on: 11/02/2021
 *      Authors: Antoine Briançon, Thibault Cantori, baioc
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

// Max scheduling priority (min is 1).
#define MAXPRIO 256

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/// Initializes the process management subsystem and moves to process "idle".
void process_init(void);

/*
 * Create a process
 * pt_func : main function of process
 * ssize : size of stack
 * prio : priority of process for execution
 * name : name of process, must be null-terminated
 * arg : arguments passed to the main function pt_func
 */
int start(int (*pt_func)(void *), unsigned long ssize, int prio,
          const char *name, void *arg);

/// Terminates the current process with the given exit code.
void exit(int retval);

/// Kills the process with the given pid, returning 0 on success.
int kill(int pid);

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

/// Makes the current process yield the CPU to the scheduler.
void schedule(void);

#endif /* _process_H_ */
