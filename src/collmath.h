/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * collmath.h
 * Contains function prototypes for basic collision detection calculations
 */

#ifndef COLLMATH_H

#define COLLMATH_H

#include "compile.h"
#include "geometry.h"

/* All of these functions return TRUE on collision and FALSE on miss */

/* point, point, sum of the radii squared */
int circle_collide(rect_point a, rect_point b, fixed_t sors);

/* top-left point, lower-right point, top-left point, lower-right point */
int aabb_collide(rect_point tla, rect_point lra, 
                 rect_point tlb, rect_point lrb);

#endif /* !def COLLMATH_H */
