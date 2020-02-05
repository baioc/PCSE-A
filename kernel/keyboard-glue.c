/*
 * keyboard.c
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
struct tty {
	char stopped:1;
};

#define KEYBOARD_TTY_TYPE struct tty

#include "keyboard-glue.h"


/* HW management */

void keyboard_pckbd_leds(unsigned char leds)
{
	kbd_leds(leds);
}

void keyboard_pckbd_init_hw(void)
{
//XXX
}


/* Console */

/* To wake up a blanked console */
void keyboard_do_poke_blanked_console(void)
{
/*
do_poke_blanked_console=1;
schedule_console_callback();
*/
}

/* Keyboard asked to change the current console */
void keyboard_set_console(int nr)
{
        (void)nr;
}

int keyboard_fg_console(void)
{
	return 0;
}

int keyboard_last_console(void)
{
	return 0;
}

/* Should start a new console */
void keyboard_spawn_console(void)
{
}

int keyboard_vc_cons_allocated(unsigned int nr)
{
	return nr == 0;
}

/* Reset console. See vt.c:reset_vc() */
void keyboard_reset_vc(unsigned int new_console)
{
        (void)new_console;
}

void keyboard_scrollback(int lines)
{
        (void)lines;
}

void keyboard_scrollfront(int lines)
{
        (void)lines;
}


/* TTY */

struct tty console_tty;

void keyboard_send_intr(struct tty *tty)
{
        (void)tty;
}

void keyboard_send_str(struct tty *tty, char *str)
{
        (void)tty;
	keyboard_data(str);
}

void keyboard_send_ch(struct tty *tty, int ch)
{
        (void)tty;
	char data[2];
	data[0] = ch;
	data[1] = 0;
	keyboard_data(data);
}

/* Get tty associated with this console */
struct tty *keyboard_get_tty(int nr)
{
        (void)nr;
	return &console_tty;
}

void keyboard_stop_tty(struct tty *tty)
{
	if (tty->stopped) return;
	tty->stopped = 1;
}

void keyboard_start_tty(struct tty *tty)
{
	if (!tty->stopped) return;
	tty->stopped = 0;
}

int keyboard_tty_is_stopped(struct tty *tty)
{
	return tty->stopped;
}

/* Should return 0 only if characters are not echoed locally and input buffer is not
 * empty (== heavy load)
 */
int keyboard_tty_do_repeat_key(struct tty *tty)
{
        (void)tty;
	return 1;
}

/* "Secure Attention Key", no need. */
void keyboard_do_SAK(struct tty *tty)
{
        (void)tty;
}


/* LEDs process level management */

static char keyboard_tasklet_enabled = 0;

void keyboard_enable_tasklet(void)
{
	keyboard_tasklet_enabled = 1;
}

void keyboard_schedule_tasklet(void)
{
	if (keyboard_tasklet_enabled && keyboard_tasklet_fn)
		keyboard_tasklet_fn(0);
}


/* Not useful */

void keyboard_ctrl_alt_del(void)
{
}

void keyboard_show_ptregs(void)
{
}

void keyboard_show_mem(void)
{
}

void keyboard_show_state(void)
{
}

void keyboard_add_randomness(unsigned char scancode)
{
        (void)scancode;
}

void do_scancode(int scancode)
{
	static int init = 0;
	if (!init) {
		init = 1;
		keyboard_init();
	}
	if ((scancode != 0x00) && (scancode != 0xff)) {
		keyboard_handle_scancode(scancode, !(scancode & 0x80));
		keyboard_tasklet_fn(0);
	}
}
