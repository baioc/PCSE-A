/*
 * test_message_queue.c
 *
 *  Created on: 21/03/2021
 *      Author: Maxime MARTIN
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "stdint.h"
#include "stdio.h"
#include "mem.h"

#include "process.h"
#include "message-queue.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

static int sender();

static int receiver();

static int sender2();

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

/*
 * Run all tests related to the console.
 * For now, it only tests prints on screen.
 */
void test_message_queue()
{
  int a = start(sender, 1024, 2, "sender", NULL);
  int b = start(receiver, 1024, 2, "reveiver", NULL);
  int c = start(sender2, 1024, 2, "sender2", NULL);
  kill(a);
  kill(b);
  kill(c);
  printf("Message queues are working well\n");
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

 static int sender(){
   assert(pcreate(2) == -1);
   assert(psend(0, 10) == -1);
   assert(psend(0, 11) == -1);
   return 0;
 }

 static int receiver(){
   int *message = (int *) mem_alloc(sizeof(int));
   preceive(1, message);
   assert(*message == 10);
   return 0;
 }

 static int sender2(){
   assert(psend(0,12) == -1);
   printf("Couldn't send, the queue is full");
   return 0;
 }
