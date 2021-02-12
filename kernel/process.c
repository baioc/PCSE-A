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

enum process_state {
  CHOSEN, // process currently running on processor
  READY,  // process waiting for his turn
  BLOCKED_ON_SEMAHPORE,
  BLOCKED_ON_IO,
  WAITING_FOR_CHILD,
  SLEEPING,
  ZOMBIE
}

struct process {
  uint32_t pid;      // between 1 and NBPROC
  uint32_t priority; // between 1 and MAXPRIO
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

/*******************************************************************************
 * Internal function
 ******************************************************************************/
