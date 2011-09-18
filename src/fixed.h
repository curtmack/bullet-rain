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

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#else
#include "SDL.h"
#endif

/*
 * We're going to roll our own solution here
 * We could use _Fract, _Accum types, but they're too finicky
 * and don't guarantee what size the integral part will be
 * Here, we're always using 16.16 numbers
 */
typedef Sint32 fixed_t;

/*
 * For the most part, everything works like you'd expect
 * e.g. to add, just do a+b
 * What's here is everything else
 */

/* constants */
#define fixzero ((fixed_t)0x00000000)
#define fixone  ((fixed_t)0x00010000)
#define fixmax  ((fixed_t)0x7FFFFFFF) /* two's complement signing */
#define fixmin  ((fixed_t)0x80000000) /* makes these work out this way */

/* increment and decrement */
#define fixinc(a) (a += fixone)
#define fixdec(a) (a -= fixone)

/*
 * tofixed(a,b) = a + b/65536
 * This makes no assumptions that a and b are the same
 * sign and correctly handles either case
 * i.e. if b is positive it's added, if b is negative it's subtracted
 */
#define tofixed(in,fr) (fixed_t)(((in)<<16) + (fr))

/*
 * Going the other way
 * Things can get a bit tricky with 2's complement, so we
 * double-negative to work around it
 */
#define intpart(a) ((int)((a)>>16))
#define fracpart(a) ((int)((a)&0x0000FFFF))

/* These have to be functions for the memory used for temp */
extern fixed_t fixmul(fixed_t a, fixed_t b);
extern fixed_t fixdiv(fixed_t a, fixed_t b);

#endif /* !def FIXED_H */
