#include "mem.h"
#include "stddef.h"
#include "debug.h"
#include "stdio.h"

extern void *sbrk(int);

#define CHUNK 1024
#define TOTAL (8 * 1024 * CHUNK)

int main(void)
{
  for (int alloc = TOTAL; alloc > 0; alloc -= CHUNK) {
    const int progress = (100 * (TOTAL - alloc) - 1) / TOTAL + 1;
    printf("\r[ %3d%% ] brk = %#x", progress, sbrk(0));
    void *test = mem_alloc(CHUNK);
    assert(test != NULL);
  }
  printf("\n");
}
