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

/// Max scheduling priority (min is 1).
#define MAXPRIO 256

/// Scheduling frequency in Hz, meaning a quantum is 1/SCHEDFREQ seconds.
#define SCHEDFREQ 50

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

/**
 * Waits for a child process to finish and reads its return value.
 *
 * This procedure may block while waiting for a child process with the specified
 * PID (a negative value here means any child will do) or it may return directly
 * in case the given PID is invalid or there are no children to wait for.
 * In the first case the return value is the pid of the child that woke this
 * process up and in the second one it is a strictly negative value.
 *
 * When RETVALP is not null and a child process was successfully awaited on,
 * the value it points to will be overwritten with the child's exit code.
 */
int waitpid(int pid, int *retvalp);

/// Makes the current process yield the CPU to the scheduler.
void schedule(void);

#endif /* _process_H_ */
