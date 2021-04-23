#include "process.h" // exit

extern int main(void *arg);

/* Tell the compiler the function is in a special section */
void _start(void *arg) __attribute__((section(".text.init")));

void _start(void *arg)
{
  exit(main(arg));
}
