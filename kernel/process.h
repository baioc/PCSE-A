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
 #define TAILLE_PILE 512
 // TODO make sure that kernel will support 1000 processes by the end of project
 #define NBPROC  30
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
extern void ctx_sw(int saveZone1[5],int saveZone2[5]);
int start(int (*pt_func)(void *), unsigned long ssize, int prio, const char *name, void *arg);
int chprio(int pid, int newprio);
int getprio(int pid);
extern void ctx_sw(int saveZone1[5],int saveZone2[5]);

#endif /* _process_H_ */
