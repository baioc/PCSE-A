#include "console.h"
#include "clock.h"
#include "process.h"
#include "mem.h"
#include "userspace_apps.h"

void kernel_start(void)
{
  console_init();
  clock_init();
  mem_init();
  uapps_init();
  process_init();
}
