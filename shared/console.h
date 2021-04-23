#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#define console_putbytes cons_write /* backward compatibility hack */

/// Prints a SIZE bytes long string starting at STR to the console.
int cons_write(const char *str, long size);

#endif
