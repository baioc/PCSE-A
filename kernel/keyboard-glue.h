/*
 * keyboard.h
 *
 * Copyright (C) 2003 by Simon Nieuviarts
 *
 * Keyboard glue code, to interface linux keyboard code with the user kernel.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef KEYBOARD_GLUE_H
#define KEYBOARD_GLUE_H



/*
 * Symbols defined in kbd_linux.o, to call in user kernel
 */

/* Call this before calling the other functions of this module */
int keyboard_init(void);

/* Call this when you receive a scancode. It converts scancode sequences into
 * calls to kbd_leds, keyboard_data, etc.
 * Parameters :
 *   SCANCODE: The 7 low bits of the scancode.
 *   DOWN: was the highest bit of the scancode set ?
 */
void keyboard_handle_scancode(unsigned char scancode, int down);



/*
 * Symbols called by keyboard-glue.c, to define in user kernel.
 */

/* This function is called by the module when a sequence of scancodes is has
 * been decoded into a string.
 */
extern void keyboard_data(char *str);

/* Called when LEDS have to be changed */
extern void kbd_leds(unsigned char leds);



/*
 * Symbols defined in keyboard-glue.c, called by kbd_linux.o
 */
extern void keyboard_pckbd_leds(unsigned char leds);
extern void keyboard_pckbd_init_hw(void);

extern void keyboard_do_poke_blanked_console(void);
extern void keyboard_set_console(int nr);
extern int keyboard_fg_console(void);
extern int keyboard_last_console(void);
extern void keyboard_spawn_console(void);
extern int keyboard_vc_cons_allocated(unsigned int nr);
extern void keyboard_reset_vc(unsigned int new_console);
extern void keyboard_scrollback(int lines);
extern void keyboard_scrollfront(int lines);

#ifndef KEYBOARD_TTY_TYPE
#define KEYBOARD_TTY_TYPE void
#endif

extern void keyboard_send_intr(KEYBOARD_TTY_TYPE *tty);
extern void keyboard_send_str(KEYBOARD_TTY_TYPE *tty, char *str);
extern void keyboard_send_ch(KEYBOARD_TTY_TYPE *tty, int ch);

extern KEYBOARD_TTY_TYPE *keyboard_get_tty(int nr);
extern void keyboard_stop_tty(KEYBOARD_TTY_TYPE *tty);
extern void keyboard_start_tty(KEYBOARD_TTY_TYPE *tty);
extern int keyboard_tty_is_stopped(KEYBOARD_TTY_TYPE *tty);
extern int keyboard_tty_do_repeat_key(KEYBOARD_TTY_TYPE *tty);
extern void keyboard_do_SAK(KEYBOARD_TTY_TYPE *tty);

extern void keyboard_enable_tasklet(void);
extern void keyboard_schedule_tasklet(void);
extern void (*keyboard_tasklet_fn)(unsigned long);

extern void keyboard_ctrl_alt_del(void);
extern void keyboard_show_ptregs(void);
extern void keyboard_show_mem(void);
extern void keyboard_show_state(void);
extern void keyboard_add_randomness(unsigned char scancode);

#endif
