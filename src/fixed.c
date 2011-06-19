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

fixed_t fixmul(fixed_t a, fixed_t b)
{
    int64_t temp;

    temp = (int64_t)a*(int64_t)b;
    return (fixed_t)(temp >> 16);
}
fixed_t fixdiv(fixed_t a, fixed_t b)
{
    int64_t temp;
    
    warn(b!=fixzero, "Division by zero");
    if (b==fixzero) return a; /* a/0 = a, simpler than panicking */
    
    temp = (int64_t)a << 16;
    return (fixed_t) (temp/(int)b);
}
