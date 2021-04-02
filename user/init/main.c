#include "process.h"
#include "stddef.h"
#include "debug.h"

int main(void)
{
  int pid, ret;
  pid = start("test_app", 0x1BAD, 1, NULL);
  assert(pid > 0);

  for (;;) waitpid(-1, NULL);
}
