#include "process.h"
#include "stddef.h"
#include "debug.h"
#include "stdio.h"

int main(void)
{
  int pid, ret;
  printf(":: reached init\n");

  printf(":: starting autotest\n");
  pid = start("autotest", 0x1BAD, 1, NULL);
  assert(pid > 0);
  while (waitpid(-1, &ret) != pid) {
  }

  printf(":: reached target user system\n");
  // TODO: interactive entry point

  for (;;) waitpid(-1, NULL);
}
