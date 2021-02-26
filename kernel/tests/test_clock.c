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
void test_clock_count_for_five();

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
  test_clock_count_for_five();
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

/*
 * Activates clock for 5 seconds
 */
void test_clock_count_for_five()
{
  // enable interrupts and begin handler daemon
  sti();
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
  // deactivate interrupts
  cli();
}
