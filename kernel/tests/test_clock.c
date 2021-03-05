/*
 * test_clock.c
 *
 *  Created on: 26/02/2021
 *      Author: thedrin
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "test_clock.h"
#include "stdio.h"
#include "interrupts.h"
#include "clock.h"
#include "cpu.h"
#include "console.h"
#include "process.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

/*
 * Activates clock for 5 seconds
 */
static int test_clock_count_for_five(void *arg);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

/*
 * Run all tests related to the clock.
 */
void test_clock()
{
  start(test_clock_count_for_five, 0, MAXPRIO, "test_clock_count_for_five", 0);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

/*
 * Activates clock for 5 seconds
 */
static int test_clock_count_for_five(void *arg)
{
  (void)arg;

  char time[] = "HH:MM:SS";
  for (int i = 0; i < CLOCKFREQ * 5; i++) {
    hlt();
    unsigned seconds = current_clock() / CLOCKFREQ;
    unsigned minutes = seconds / 60;
    seconds %= 60;
    unsigned hours = minutes / 60;
    minutes %= 60;
    hours %= 24;
    sprintf(time, "%02u:%02u:%02u", hours, minutes, seconds);
    console_write_raw(time, 8, 24, 72);
  }

  return 0;
}
