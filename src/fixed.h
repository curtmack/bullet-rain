/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * fixed.h
 * Fixed-point arithmetic routines. Mostly macros.
 */

#ifndef FIXED_H

#define FIXED_H

#include "debug.h"
#include <stdint.h>

/*
 * We're going to roll our own solution here
 * We could use _Fract, _Accum types, but they're too finicky
 * and don't guarantee what size the integral part will be
 * Here, we're always using 16.16 numbers
 */
typedef int32_t fixed_t;

/*
 * For the most part, everything works like you'd expect
 * e.g. to add, just do a+b
 * What's here is everything else
 */

/* constants */
#define fixzero ((fixed_t)0x00000000)
#define fixone  ((fixed_t)0x00010000);

/* increment and decrement */
#define fixinc(a) (a += fixone)
#define fixdec(a) (a -= fixone)

/*
 * tofixed(a,b) = a + b/65536
 * This makes no assumptions that a and b are the same
 * sign and correctly handles either case
 * i.e. if b is positive it's added, if b is negative it's subtracted
 */
#define tofixed(a,b) (fixed_t)(((a)<<16) + (b))

/*
 * Going the other way
 * Things can get a bit tricky with 2's complement, so we
 * double-negative to work around it
 */
#define intpart(a) (a>=0 ? (int)(a>>16) : -(int)((-a)>>16))
#define fracpart(a) (a>=0 ? (int)(a&0x0000FFFF) : -(int)((-a)&0x0000FFFF))

/* These have to be inlines for the memory used for temp */
inline fixed_t fixmul(fixed_t a, fixed_t b)
{
    int64_t temp;

    temp = (int64_t)a*(int64_t)b;
    return (fixed_t)(temp >> 16);
}
inline fixed_t fixdiv(fixed_t a, fixed_t b)
{
    int64_t temp;
    
    warn(b!=fixzero, "Division by zero");
    if (b==fixzero) return a; /* a/0 = a, simpler than panicking */
    
    temp = (int64_t)a << 16;
    return (fixed_t) (temp/(int)b);
}

#endif /* !def FIXED_H */
