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

/*******************************************************************************
 * Macros
 ******************************************************************************/

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
 * Change priority of process referenced by pid to the value newprio
 * If priority changed and the process was in a queue, it needs to be placed
 * again in that queue depending on its new priority.
 * If the value of newprio is invalid, return value must be < 0. Otherwise, return
 * value is the previous priority of process
 */
int chprio(int pid, int newprio);

/*
* If value of pid is invalid, return value must be < 0. Otherwise, return value
* is the current priority of process referenced by pid
*/
int getprio(int pid);


#endif /* _process_H_ */
