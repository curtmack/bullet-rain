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
extern int circle_collide(fixed_t ax, fixed_t ay, fixed_t bx, fixed_t by,
                          fixed_t sors);

/* top-left point, lower-right point, top-left point, lower-right point */
extern int aabb_collide(fixed_t tlax, fixed_t tlay,
                        fixed_t lrax, fixed_t lray,
                        fixed_t tlbx, fixed_t tlby,
                        fixed_t lrbx, fixed_t lrby);

#endif /* !def COLLMATH_H */
