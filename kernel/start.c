#include "console.h"
#include "clock.h"
#include "process.h"

#ifdef KERNEL_TEST
#include "kernel_tests.h"
#endif

void kernel_start(void)
{
  console_init();
  clock_init();
  printf("%lu", sizeof(size_t));

#ifdef KERNEL_TEST
  kernel_run_general_tests();
#endif

  process_init();
}
