/*
 * sem.c
 *
 *  Created on: 03/03/2021
 *      Authors: Antoine Briançon, Thibault Cantori
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
 #include "cpu.h"
 #include "mem.h"
 #include "process.h"
 /*******************************************************************************
  * Macros
  ******************************************************************************/
  #define MAXNBR_SEM 30
 /*******************************************************************************
  * Types
  ******************************************************************************/
  typedef struct semaph{
    short int sid;
    short int count;
  }semaph;
 /*******************************************************************************
  * Internal function declaration
  ******************************************************************************/
 /*******************************************************************************
  * Variables
  ******************************************************************************/
  int nbr_sem = 0;

  semaph list_sem[MAXNBR_SEM];
 /*******************************************************************************
  * Public function
  ******************************************************************************/
  /*
  Create a semaphore
  */
  int screate(short int count){
    if(nbr_sem == MAXNBR_SEM || count < 0) return -1;
    //Looking for a blank place to create the semaphore
    int i = 0;
    while(list_sem[i].sid < 0 || list_sem[i].sid > MAXNBR_SEM){
      i++;
    }
    list_sem[i].sid = i;
    list_sem[i].count = count;
    nbr_sem++;
    return i;
  }

  /*
  Delete a semaphore
  */
  int sdelete(int sem){
    if(sem >= MAXNBR_SEM || sem < 0) return -1;
    return 0;
  }
/*
  int sreset(semaph sem,short int count);
  int signal(int sem);
  int signaln(int sem,short int count);
  int try_wait(int sem);
  int wait(int sem);
  int scount(semaph sem);
*/
 /*******************************************************************************
  * Internal function
  ******************************************************************************/
