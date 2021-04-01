#include "process.h" // exit

extern int main(void);

/* Tell the compiler the function is in a special section */
void _start(void) __attribute__((section(".text.init")));

void _start(void)
{
  exit(main());
}
