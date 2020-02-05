/*
 * Ensimag - Projet système
 * Copyright (C) 2012 - Damien Dejean <dam.dejean@gmail.com>
 */

#include "sysapi.h"

/*******************************************************************************
 * Division 64 bits
 ******************************************************************************/
#if defined DIVISION_FOIREUSE
        static int
log32(unsigned long x)
{
        unsigned long n;
        int l = 0;
        n = x & 0xffff0000;
        if (n) {
                x = n;
                l = 16;
        }
        n = x & 0xff00ff00;
        if (n) {
                x = n;
                l += 8;
        }
        n = x & 0xf0f0f0f0;
        if (n) {
                x = n;
                l += 4;
        }
        n = x & 0xcccccccc;
        if (n) {
                x = n;
                l += 2;
        }
        n = x & 0xaaaaaaaa;
        if (n) {
                x = n;
                l++;
        }
        return l;
}

        static unsigned long long
lshift64(unsigned long long val, unsigned int idx)
{
        unsigned long val1 = val >> 32;
        unsigned long val0 = val & 0xffffffff;
        unsigned long res1, res0;
        if (idx >= 32) {
                res0 = 0;
                res1 = val0 << (idx - 32);
        } else if (idx) {
                res0 = val0 << idx;
                res1 = (val1 << idx) + (val0 >> (32 - idx));
        } else {
                return val;
        }
        return (((unsigned long long)res1) << 32) + res0;
}

unsigned long long div64(unsigned long long num, unsigned long long div, unsigned long long *rem)
{
        unsigned int div1 = div >> 32;
        unsigned int div0 = div & 0xffffffff;
        unsigned int num1 = num >> 32;
        unsigned int num0 = num & 0xffffffff;

        if (!div) goto zero;
        if (!num1) {
                if (div1)
                        goto zero;
                else
                        goto div32;
        }
        unsigned int lnum = log32(num1) + 32;
        unsigned int ldiv = (div1) ? log32(div1) + 32 : log32(div0);
        if (ldiv > lnum) goto zero;
        ldiv = lnum - ldiv;

        unsigned long long mdiv = lshift64(div, ldiv);
        unsigned long long mul = lshift64(1, ldiv);
        unsigned long long q = 0;

        do {
                if (num >= mdiv) {
                        q += mul;
                        num -= mdiv;
                }
                mul /= 2;
                if (!mul) {
                        if (rem) *rem = num;
                        return q;
                }
                mdiv /= 2;
        } while (num >> 32);

        num0 = num & 0xffffffff;
        div0 = div & 0xffffffff;
div32:
        if (rem) *rem = num0 % div0;
        return q + (num0 / div0);
zero:
        if (rem) *rem = num;
        return 0;
}
#else
unsigned long long div64(unsigned long long x, unsigned long long div, unsigned long long *rem)
{
        unsigned long long mul = 1;
        unsigned long long q;

        if ((div > x) || !div) {
                if (rem) *rem = x;
                return 0;
        }

        while (!((div >> 32) & 0x80000000ULL)) {
                unsigned long long newd = div + div;
                if (newd > x) break;
                div = newd;
                mul += mul;
        }

        q = mul;
        x -= div;
        while (1) {
                mul /= 2;
                div /= 2;
                if (!mul) {
                        if (rem) *rem = x;
                        return q;
                }
                if (x < div) continue;
                q += mul;
                x -= div;
        }
}
#endif

/*******************************************************************************
 * Pseudo random number generator
 ******************************************************************************/
static unsigned long long mul64(unsigned long long x, unsigned long long y)
{
        unsigned long a, b, c, d, e, f, g, h;
        unsigned long long res = 0;
        a = x & 0xffff;
        x >>= 16;
        b = x & 0xffff;
        x >>= 16;
        c = x & 0xffff;
        x >>= 16;
        d = x & 0xffff;
        e = y & 0xffff;
        y >>= 16;
        f = y & 0xffff;
        y >>= 16;
        g = y & 0xffff;
        y >>= 16;
        h = y & 0xffff;
        res = d * e;
        res += c * f;
        res += b * g;
        res += a * h;
        res <<= 16;
        res += c * e;
        res += b * f;
        res += a * g;
        res <<= 16;
        res += b * e;
        res += a * f;
        res <<= 16;
        res += a * e;
        return res;
}

static const uint_fast64_t _multiplier = 0x5DEECE66DULL;
static const uint_fast64_t _addend = 0xB;
static const uint_fast64_t _mask = (1ULL << 48) - 1;
static uint_fast64_t _seed = 1;

// Assume that 1 <= _bits <= 32
static uint_fast32_t randBits(int _bits)
{
        uint_fast32_t rbits;
        uint_fast64_t nextseed = (mul64(_seed, _multiplier) + _addend) & _mask;
        _seed = nextseed;
        rbits = nextseed >> 16;
        return rbits >> (32 - _bits);
}

short randShort()
{
        return randBits(15);
}

void setSeed(uint_fast64_t _s)
{
        _seed = _s;
}

unsigned long rand()
{
        return randBits(32);
}

