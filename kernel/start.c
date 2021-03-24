#include "console.h"
#include "clock.h"
#include "process.h"
#include "userspace_apps.h"

#ifdef KERNEL_TEST
#include "kernel_tests.h"
#endif

void kernel_start(void)
{
  console_init();
  clock_init();
#ifdef KERNEL_TEST
  kernel_run_general_tests();
#endif
  uapps_init();
  process_init();
}
