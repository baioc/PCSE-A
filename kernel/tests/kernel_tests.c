/*
 * kernel_tests.c
 *
 *  Created on: 11/02/2021
 *      Author: Thibault Cantori
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "kernel_tests.h"
#include "test_console.h"
#include "test_clock.h"
#include "test_message_queue.h"
#include "userspace_tests.h"
#include "cpu.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/
void kernel_run_general_tests()
{
  test_console();
}

void kernel_run_process_tests()
{
  //test_clock();
  test_message_queue();
  // Run imported tests from user/tests directory
  //run_userspace_tests();
}
/*******************************************************************************
 * Internal function
 ******************************************************************************/
