#include "process.h"
#include "stddef.h"
#include "debug.h"
#include "stdio.h"

int main(void)
{
  int pid;
  printf(":: reached init\n");

  printf(":: testing dynamic user heap\n");
  pid = start("sbrktest", 0x1B0B, 1, NULL);
  assert(pid > 0);
  waitpid(pid, NULL);

  printf(":: starting autotest\n");
  pid = start("autotest", 0x1BAD, 1, NULL);
  assert(pid > 0);
  while (waitpid(-1, NULL) != pid) continue;

  printf(":: reached target user system\n");
  // TODO: interactive entry point

  for (;;) waitpid(-1, NULL);
}
