#ifndef _CLOCK_H_
#define _CLOCK_H_

/// Ticks per second.
#define CLOCKFREQ 100

// Converts time from MS miliseconds to tick units.
#define MS_TO_TICKS(ms) ((ms) * CLOCKFREQ / 1000)

// Converts time from TS kernel ticks to milisecond units.
#define TICKS_TO_MS(ts) ((ts) * 1000 / CLOCKFREQ)

/**
 * Writes to output parameters the internal quartz oscillator frequency and
 * how many of these oscillations elapse from one tick to the next.
 */
void clock_settings(unsigned long *quartz, unsigned long *ticks);

/// Returns current jiffies (the number of ticks elapsed since startup).
unsigned long current_clock(void);

/// Sends the current process to sleep for at least the given number of ticks.
void wait_clock(unsigned long clock);

#endif // _CLOCK_H_
