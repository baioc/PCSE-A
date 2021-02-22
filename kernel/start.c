#include "debugger.h"
#include "cpu.h"
#include "stdio.h"
#include "console.h"
#include "process.h"
#include <stddef.h>

int fact(int n)
{
	if (n < 2)
		return 1;

	return n * fact(n-1);
}


void kernel_start(void)
{
	printf("\fPROC");
	int i = 43;
	void *a = &i;
	if(start(tstA2,512,256,"proc1",a) == 1){
		printf("erreur A");
	}
	if(start(tstB2,512,256,"proc2",a) == 1){
		printf("erreur B");
	}
	tstA2();
	/*
	int i;
//	call_debugger();
	i = 10;

	i = fact(i);
	console_putbytes("test", 4);

	printf("\fHello world");
	*/
	while(1)
	  hlt();

	return;
}
