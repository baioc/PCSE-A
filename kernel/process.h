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

/// Ticks the current process time, may end up calling the scheduler.
void process_tick(void);

/**
 * Creates and starts a new a process.
 * name : name of user program to find among the system's apps.
 * ssize : size of user stack, in bytes.
 * prio : priority of process for execution.
 * arg : generic, word-sized, argument passed to the created process' main().
 */
int start(const char *name, unsigned long ssize, int prio, void *arg);

/// Terminates the current process with the given exit code.
void exit(int retval);

/// Kills the process with the given pid, returning 0 on success.
int kill(int pid);

/**
 * Change priority of process referenced by pid to the value newprio.
 * If the value of newprio is invalid, return value is negative.
 * Otherwise, return value is the previous priority of process
 */
int chprio(int pid, int newprio);

/**
 * If value of pid is invalid, return value is negative. Otherwise, returns
 * the current priority of process referenced by pid.
 */
int getprio(int pid);

/// Returns pid of the calling process.
int getpid(void);

/// Makes the current process yield the CPU for at least TICKS jiffies.
void sleep(unsigned long ticks);

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

#endif /* _process_H_ */
