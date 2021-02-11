/*
 * test_console.c
 *
 *  Created on: 10/02/2021
 *      Author: Thibault Cantori
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "stdint.h"
#include "stdio.h"

#include "console.h"
#include "cpu.h"
#include "debug.h"

#include "test_console.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

#define SCREEN_BASE_ADDR (uint16_t *)0xB8000
#define SCREEN_BASE_SIZE (CONSOLE_H * CONSOLE_L * 2)

#define SCREEN_CMD_PORT 0x3D4
#define SCREEN_DAT_PORT 0x3D5

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

/*
 * Check if cursor position is correctly when sending '\n' character
 */
static void test_console_newline();

/*
 * Return current cursor position
 */
static uint16_t test_console_get_cursor_pos();

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
void test_console() {
  test_console_clear();
  test_console_newline();
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

/*
 * Check if the screen is emptied when '\f' character is sent.
 * Also check cursor's position
 */
static void test_console_clear() {
  uint16_t  pos;
  uint16_t *current_mem_pos;

  // Start by printing a character at every position on screen
  for (uint8_t line = 0; line < CONSOLE_H; line++) {
    for (uint8_t col = 0; col < CONSOLE_L; col++) {
      printf("0");
    }
  }

  // Then clear screen by sending '\f'
  printf("\f");

  // Check every position
  for (uint8_t line = 0; line < CONSOLE_H; line++) {
    for (uint8_t col = 0; col < CONSOLE_L; col++) {
      current_mem_pos = SCREEN_BASE_ADDR + line * CONSOLE_H + col;
      assert(*current_mem_pos == ((uint16_t)' ' | ((uint16_t)0b00001111 << 8)));
    }
  }

  // Check cursor position
  pos = test_console_get_cursor_pos();
  assert(pos == 0);
}

/*
 * Check if cursor position is correctly set when sending '\n' character
 */
static void test_console_newline() {
  uint16_t cursor_pos;
  uint16_t cursor_line, cursor_col;

  // Clear screen first
  printf("\f");
  // At that point screen clear has been tested, so cursor is at position (0, 0)

  // Tests printing a newline character on every line apart from the last
  for (uint8_t line = 0; line < CONSOLE_H - 1; line++) {
    printf("\n");
    cursor_pos  = test_console_get_cursor_pos();
    cursor_line = cursor_pos / CONSOLE_L;
    cursor_col  = cursor_pos % CONSOLE_L;
    assert(cursor_line == line + 1);
    assert(cursor_col == 0);
  }
  // Print a newline on last line
  printf("\n");

  // Check if we stay on that line
  cursor_pos  = test_console_get_cursor_pos();
  cursor_line = cursor_pos / CONSOLE_L;
  cursor_col  = cursor_pos % CONSOLE_L;
  assert(cursor_line == CONSOLE_H - 1);
  assert(cursor_col == 0);
}

/*
 * Return current cursor position
 */
static uint16_t test_console_get_cursor_pos() {
  uint8_t lower_part, higher_part;

  outb(0x0f, SCREEN_CMD_PORT);
  lower_part = inb(SCREEN_DAT_PORT);

  outb(0x0e, SCREEN_CMD_PORT);
  higher_part = inb(SCREEN_DAT_PORT);
  return (higher_part << 8) + lower_part;
}
