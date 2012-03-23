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
int circle_collide(float ax, float ay, float bx, float by,
                          float sors)
{
    return ((ax-bx)*(ax-bx) + (ay-by)*(ay-by) <= sors);
}

/* top-left point, lower-right point, top-left point, lower-right point */
int aabb_collide(float tlax, float tlay, float lrax, float lray,
                 float tlbx, float tlby, float lrbx, float lrby)
{
    /* horizontal overlap */
    if (lrax < tlbx || tlax > lrbx) return FALSE;
    /* vertical overlap */
    if (lray < tlby || tlay > lrby) return FALSE;
    return TRUE;
}
