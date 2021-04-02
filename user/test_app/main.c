#include "stdio.h"

int main(void *arg)
{
  printf("Je demarre\n");
  // *(int *)0xB8000 = 1 / (int)arg;
  __asm__ __volatile__("cli":::"memory");
  printf("Je stop\n");
}
