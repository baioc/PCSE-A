#ifndef __CONSOLE_H__
#define __CONSOLE_H__

/**
 * This is the function called by printf to send its output to the screen. You
 * have to implement it in the kernel and in the user program.
 */
void console_putbytes(const char *s, int len);

/**
 * Initialize console subsystem.
 */
void console_init(void);

/**
 * Like console_putbytes, but writes to some fixed position and does not
 * interpret control characters.
 */
void console_write_raw(const char *s, int len, int line, int column);

#endif
