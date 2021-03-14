/*
 * message-queue.h
 *
 *  Created on: 10/03/2021
 *      Authors: Maxime Martin
 */

#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

/*******************************************************************************
 * Includes
 ******************************************************************************/

 #include "process.h"
 #include "debug.h"
 #include "queue.h"
 #include "cpu.h"
 #include "clock.h"
 #include "console.h"

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

 int pcreate(int count);

 int psend(int fid, int message);

 int preceive(int fid, int *message);

 int pdelete(int fid);

 int preset(int fid);

 void init_indice_gestion_list();

 void print_unused_indice_gestion_list();

 int valid_fid(int fid);

 void test();

#endif /* MESSAGE_QUEUE_H */
