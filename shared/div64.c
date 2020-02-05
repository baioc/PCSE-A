/*
 * Copyright (C) 2002, Simon Nieuviarts.
 * License : LGPL.
 */
#include "div64.h"


#define log2 hard_bsr
static inline unsigned int hard_bsr(unsigned int n)
{
        unsigned long r;

        __asm__ __volatile__ ("bsr %1,%0": "=r" (r): "rm" (n));

        return r;
}

#define rshift64(x1, x0, n) \
__asm__ ("shrdl %0,%1; shrl %b4,%0; testb $32,%b4; je 0f; movl %0,%1; xorl %0,%0; 0:" :"=r" (x1), "=r" (x0) : "0" (x1), "1" (x0), "c" ((n)))

#define rshift64_32(x1, x0, n) \
__asm__ ("shrdl %1,%0; testb $32,%b3; je 0f; movl %1,%0; 0:" :"=r" (x0) : "r" ((x1)), "0" (x0), "c" ((n)))

/* remainder in x1 */
#define div64_32(x1, x0, y) \
__asm__ ("divl %4" :"=d" (x1), "=a" (x0) :"d" (x1), "a" (x0), "g" ((y)))

unsigned long long do_div64(unsigned long long x, unsigned long long y, unsigned long long *rem)
{
	unsigned int y1 = y >> 32;
	unsigned int y0 = y & 0xffffffff;
	unsigned int x1 = x >> 32;
	unsigned int x0 = x & 0xffffffff;
	if (y1) {
		do {
			unsigned int l = log2(y1) + 1;
			rshift64_32(y1, y0, l);
			rshift64(x1, x0, l);
			div64_32(x1, x0, y0 + 1);
		} while (0);
		do {
			unsigned long long r = x - (y * x0);
			while (r >= y) {
				x0++;
				r -= y;
			}
			if (rem) *rem = r;
		} while (0);
		return x0;
	} else if (x1 < y0) {
		div64_32(x1, x0, y0);
		if (rem) *rem = x1;
		return x0;
	} else if (y0) {
		unsigned int x2 = 0;
		div64_32(x2, x1, y0);
		div64_32(x2, x0, y0);
		if (rem) *rem = x2;
		return (((unsigned long long)x1) << 32) + x0;
	} else {
		if (rem) *rem = x;
		return 0;
	}
}
