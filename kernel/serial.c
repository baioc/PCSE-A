#include "cpu.h"
#include "debug.h"
#include "serial.h"

static void init_serial(void)
{
	static int done = 0;
	if (done) return;

	// Switch the serial controller to normal state
	outb(0x0f, 0x3f8 + 4);

	// Disable interrupts
	outb(0, 0x3f8 + 1);

	// Setup the baud rate to 115200 (divisor = 1)
	outb(0x83, 0x3f8 + 3);
	outb(1, 0x3f8);
	outb(0, 0x3f8 + 1);

	// Setup 8 bit transfer, no parity, 1 stop bit
	outb(0x03, 0x3f8 + 3);

	done = 1;
}

void write_com1(int c)
{
	init_serial();
	while ((inb(0x3f8 + 5) & 0x20) == 0);
	outb(c, 0x3f8);
}

int read_com1(void)
{
	int c;
	init_serial();
	do {
		c = inb(0x3f8 + 5);
	} while ((c & 0x1e) || ((c & 1) == 0));
	c = inb(0x3f8);
	return c;
}
