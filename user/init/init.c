#include "process.h"
#include "stddef.h"
#include "debug.h"
#include "stdio.h"
#include "clock.h"

static void wait_dot(int n)
{
  for (int i = 0; i < n; ++i) {
    wait_clock(MS_TO_TICKS(1000));
    printf(".");
  }
}

int main(void)
{
  int pid;
  printf(":: reached init\n");

  printf(":: reached target user system ");
  wait_dot(3);
  printf("\f");
  pid = start("shell", 2048, 128, 0);
  assert(pid > 0);
  while (waitpid(-1, NULL) != pid) continue;
  printf("\f");
  printf(":: shutting down\n");

  printf(":: waiting for running tasks to finish ");
  while (waitpid(-1, NULL) > 0) {
    printf(".");
    continue;
  }
  printf("\n");
  printf(":: reached target power off ");
  wait_dot(3);
  printf("\n");
}
