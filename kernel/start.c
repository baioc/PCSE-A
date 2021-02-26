#include "stdio.h"
#include "debugger.h"
#include "console.h"
#include "clock.h"
#include "process.h"

#ifdef KERNEL_TEST
#include "kernel_tests.h"
#endif

void kernel_start(void)
{
  // initialize subsystems
  console_init();
  clock_init();

  // testing
  int i = 42;
  printf("Hello world\n");
  printf("The answer is %d\n", i);
  // call_debugger();

#ifdef KERNEL_TEST
    kernel_run_tests();
#endif

  process_init();
}
