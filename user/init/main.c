#include "process.h"
#include "stddef.h"
#include "debug.h"
#include "stdio.h"

int main(void)
{
  int pid, ret;

  pid = start("test_app", 0x1F00, 128, (void *)42);
  assert(pid > 0);
  waitpid(pid, &ret);
  printf("test_app returned %d\n", ret);

  for (;;) waitpid(-1, NULL);
}
