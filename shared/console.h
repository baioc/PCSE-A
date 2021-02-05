#ifndef __CONSOLE_H__
#define __CONSOLE_H__


#define CONSOLE_H 25
#define CONSOLE_L 80

#define CL_NOIR 0
#define CL_BLEU 1
#define CL_VERT 2
#define CL_CYAN 3
#define CL_ROUGE 4
#define CL_MAGENTA 5
#define CL_MARRON 6
#define CL_GRIS 7
#define CL_LIGHT 0b1000


/*
 * This is the function called by printf to send its output to the screen. You
 * have to implement it in the kernel and in the user program.
 */
extern void console_putbytes(const char *s, int len);

#endif
