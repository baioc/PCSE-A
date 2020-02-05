/*
 * kbd.h
 *
 * Copyright (C) 2005 by Simon Nieuviarts
 *
 * Keyboard scancode handling.
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
#ifndef __KBD_H__
#define __KBD_H__

/* Call this function for each scancode received to translate them to
characters. */
void do_scancode(int scancode);

/* You have to implement this function. It is called by do_scancode with
a string that is the translation of the scancodes into characters. */
void keyboard_data(char *str);

/* You may implement this function to keep keyboard LEDs in sync with the
state of the keyboard driver (do_scancode). */
void kbd_leds(unsigned char leds);

#endif
