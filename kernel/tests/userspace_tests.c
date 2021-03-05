/*
 * userspace_tests.c
 *
 *  Created on: 26/02/2021
 *      Author: Thibault Cantori
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "userspace_tests.h"
#include "process.h"
#include "debug.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Types
 ******************************************************************************/

/*******************************************************************************
 * Internal function declaration
 ******************************************************************************/

static int test_0(void *arg);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Public function
 ******************************************************************************/

void run_userspace_tests()
{
  start(test_0, 0, MAXPRIO, "test_0", 0);
}

/*******************************************************************************
 * Internal function
 ******************************************************************************/

static int test_0(void *arg)
{
  (void)arg;
  register unsigned reg1 = 1u;
  register unsigned reg2 = 0xFFFFFFFFu;
  register unsigned reg3 = 0xBADB00B5u;
  register unsigned reg4 = 0xDEADBEEFu;

  printf("I'm a simple process running ...");

  unsigned i;
  for (i = 0; i < 10000000; i++) {
    if (reg1 != 1u || reg2 != 0xFFFFFFFFu || reg3 != 0xBADB00B5u ||
        reg4 != 0xDEADBEEFu)
    {
      printf(" and I feel bad. Bybye ...\n");
      assert(0);
    }
  }

  printf(" and I'm healthy. Leaving.\n");

  return 0;
}
