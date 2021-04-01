#include "debug.h"
#include "stdarg.h"

void __attribute__((noreturn)) panic(const char *fmt, ...)
{
	va_list ap;

	printf("PANIC: ");
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	while (1);
}
