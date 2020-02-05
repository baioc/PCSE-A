#ifndef KEYBOARD_GLUE_H
#define KEYBOARD_GLUE_H


/* Symbols defined in kbd_linux.o, to call in client kernel. */
int keyboard_init(void);
void keyboard_handle_scancode(unsigned char scancode, int down);


/* Symbols called by keyboard-glue.c, to define in client kernel. */
extern void keyboard_data(char *str);
extern void kbd_leds(unsigned char leds);


/* Symbols defined in keyboard-glue.c, called by kbd_linux.o */
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
