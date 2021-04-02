#include "process.h"
#include "stddef.h"

int main(void)
{
	start("test_app", 0x1F00, 128, (void *)42);
	for (;;) waitpid(-1, NULL);
}
