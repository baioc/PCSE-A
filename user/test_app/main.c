#include "stdio.h"

int main(void *arg)
{
  printf("Hello, User World!\n");
  printf("The answer is %d\n", (int)arg);
  return *(int *)0x101000;
}
