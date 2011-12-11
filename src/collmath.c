/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * collmath.c
 * Contains code for basic collision detection calculations
 */

#include "geometry.h"
#include "collmath.h"

/* point, point, sum of the radii squared */
int circle_collide(fixed_t ax, fixed_t ay, fixed_t bx, fixed_t by,
                          fixed_t sors)
{
    /* Here, we need to prevent overflow */
    Sint64 tmp;
    tmp = (Sint64) fixmul(ax-bx, ax-bx) + fixmul(ay-by, ay-by);
    if ((Sint32)tmp != tmp) {
        /* We overflowed, just return false */
        return FALSE;
    }
    else return ((fixed_t)tmp <= sors);
}

/* top-left point, lower-right point, top-left point, lower-right point */
int aabb_collide(fixed_t tlax, fixed_t tlay, fixed_t lrax, fixed_t lray,
                 fixed_t tlbx, fixed_t tlby, fixed_t lrbx, fixed_t lrby)
{
    /* horizontal overlap */
    if (lrax < tlbx || tlax > lrbx) return FALSE;
    /* vertical overlap */
    if (lray < tlby || tlay > lrby) return FALSE;
    return TRUE;
}
