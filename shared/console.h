#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#define console_putbytes cons_write /* backward compatibility hack */

/// Prints a SIZE bytes long string starting at STR to the console.
int cons_write(const char *str, long size);

/// Reads a line of at most LENGHT characters from keyboard input into STRING.
unsigned long cons_read(char *string, unsigned long length);

/// Toggles character echo during cons_read().
void cons_echo(int on);

#endif
