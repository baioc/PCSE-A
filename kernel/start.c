#include "debugger.h"
#include "cpu.h"
#include "stdio.h"
#include "console.h"

#ifdef KERNEL_TEST
#include "kernel_tests.h"
#endif

int fact(int n)
{
	if (n < 2)
		return 1;

	return n * fact(n-1);
}


void kernel_start(void)
{
	int i;
//	call_debugger();
	i = 10;

	i = fact(i);

#ifdef KERNEL_TEST
    kernel_run_tests();
#endif

	console_putbytes("test", 4);

	printf("\fHello world");

	while(1)
	  hlt();

	return;
}
