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
int circle_collide(rect_point a, rect_point b, fixed_t sors) {
    /* Here, we need to prevent overflow */
    Sint64 tmp;
    tmp = (Sint64) fixmul(a.x-b.x, a.x-b.x) + fixmul(a.y-b.y, a.y-b.y);
    if ((Sint32)tmp != tmp) {
        /* We overflowed, just return false */
        return FALSE;
    }
    else return ((fixed_t)tmp <= sors);
}

/* top-left point, lower-right point, top-left point, lower-right point */
int aabb_collide(rect_point tla, rect_point lra, 
                 rect_point tlb, rect_point lrb) {
    return (
        /* horizontal overlap */
        ((tla.x > tlb.x && tla.x < lrb.x) || (lra.x > tlb.x && lra.x < lrb.x))
        &&
        /* vertical overlap */
        ((tla.y > tlb.y && tla.y < lrb.y) || (lra.y > tlb.y && lra.y < lrb.y))
    );
}
