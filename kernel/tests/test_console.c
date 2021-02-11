/*
 * test_console.c
 *
 *  Created on: 10/02/2021
 *      Author: Thibault Cantori
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "stdio.h"
#include "stdint.h"

#include "console.h"

#include "test_console.h"

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
 * Check if the screen is emptied when '\f' character is sent.
 * Also check cursor's position
 */
static void test_console_clear();

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
void test_console() { test_console_clear(); }

/*******************************************************************************
 * Internal function
 ******************************************************************************/

/*
 * Check if the screen is emptied when '\f' character is sent.
 * Also check cursor's position
 */
static void test_console_clear() {
  // Start by printing a character at every position on screen
  for (uint8_t line = 0; line < CONSOLE_H; line++) {
    for (uint8_t col = 0; col < CONSOLE_L; col++) {
      printf("0");
    }
  }
  // Then clear screen by sending '\f'

  // Check every position

  // Check cursor position
}
