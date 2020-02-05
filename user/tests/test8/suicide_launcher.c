#include "sysapi.h"

int main(void *arg)
{
	int pid1;
        (void)arg;
	pid1 = start("suicide", 4000, 192, 0);
	assert(pid1 > 0);
	return pid1;
}

