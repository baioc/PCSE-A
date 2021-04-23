#include "debug.h"

#include "stdarg.h"
#include "process.h"

void __attribute__((noreturn)) panic(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);

  exit(1);
}
