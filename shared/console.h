#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#define console_putbytes cons_write /* backward compatibility hack */

/// Prints a SIZE bytes long string starting at STR to the console.
int cons_write(const char *str, long size);

/// Gets a maximum LENGHT characters string from the keyboard
unsigned long cons_read(char *string, unsigned long length);

void cons_echo(int on);

#endif
