#include "console.h"
#include "clock.h"
#include "process.h"

void kernel_start(void)
{
  console_init();
  clock_init();
  process_init();
}
