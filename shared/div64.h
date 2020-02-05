/*
 * For some computations, you may need 64-bit integers. The division needs an
 * helper function that is not available. Use these functions instead.
 */
#ifndef _DIV64_H_
#define _DIV64_H_

unsigned long long do_div64(unsigned long long x, unsigned long long y, unsigned long long *rem);

static inline unsigned long long div64(unsigned long long x, unsigned long long y)
{
	return do_div64(x, y, 0);
}

static inline unsigned long long mod64(unsigned long long x, unsigned long long y)
{
	unsigned long long __r;
	do_div64(x, y, &__r);
	return __r;
}

#endif
