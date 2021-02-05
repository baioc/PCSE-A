#include "debugger.h"
#include "cpu.h"
#include "stdio.h"
#include "console.h"

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
	console_putbytes("test", 4);

	printf("\fHello world");
	while(1)
	  hlt();

	return;
}
