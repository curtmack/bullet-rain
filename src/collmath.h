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
extern int circle_collide(float ax, float ay, float bx, float by,
                          float sors);

/* top-left point, lower-right point, top-left point, lower-right point */
extern int aabb_collide(float tlax, float tlay,
                        float lrax, float lray,
                        float tlbx, float tlby,
                        float lrbx, float lrby);

#endif /* !def COLLMATH_H */
