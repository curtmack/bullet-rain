/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * fixed.c
 * Everything that couldn't go in fixed.h
 */
 
#include "fixed.h"

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#else
#include "SDL.h"
#endif

fixed_t fixmul(fixed_t a, fixed_t b)
{
    Sint64 tmp;

    tmp = (Sint64)a*(Sint64)b;
    tmp = tmp >> 16;
    
    /*
     * Explanation for the following:
     * 
     * This system has a hiccup when it comes to collision detection at certain
     * distances with circles.
     * The problem is that the collision detection calculation for circles
     * deals with distances squared. When the distance becomes greater than
     * roughly 181, the square will overflow.
     * To compensate for this, we need to detect overflow, and make sure it gets
     * "capped" rather than truncated. For our purposes, this makes more sense,
     * as it's reasonable to interpret 0x7FFFFFFF as "really really big" for
     * collision detection purposes.
     * Theoretically, this could become a problem when calucated the
     * sum-of-radii-squared, but that's a very fringe case - player radius is
     * often quite small, and bullet radius > 181 would cover the whole screen
     * on most games.
     */
    
    if ((Sint32)(tmp) != tmp) {
        /*
         * We overflowed, test to see which direction 
         * We have to go with the signs of a and b here because tmp
         * might have had its sign blown up by the overflow
         */
        verbose("Overflow detected");
        if (((a < 0) && (b > 0)) || ((a > 0) && (b < 0))) {
            return fixmin;
        }
        else {
            return fixmax;
        }
    }
    else return (fixed_t)(tmp);
}
fixed_t fixdiv(fixed_t a, fixed_t b)
{
    Sint64 tmp;
    
    warn(b!=fixzero, "Division by zero");
    if (b==fixzero) return a; /* a/0 = a, simpler than panicking */
    
    tmp = (Sint64)a << 16;
    tmp /= (int)b;
    
    if ((Sint32)(tmp) != tmp) {
        /*
         * We overflowed, test to see which direction 
         * We have to go with the signs of a and b here because tmp
         * might have had its sign blown up by the overflow
         */
        verbose("Overflow detected");
        if (((a < 0) && (b > 0)) || ((a > 0) && (b < 0))) {
            return fixmin;
        }
        else {
            return fixmax;
        }
    }
    else return (fixed_t)(tmp);
    
    return (fixed_t) (tmp/(int)b);
}
