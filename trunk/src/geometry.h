/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * geometry.h
 * Contains typedefs and function prototypes for working with
 * angles and points
 */

#ifndef GEOMETRY_H

#define GEOMETRY_H
 
#include "fixed.h"

/* Angles are fixed-point numbers */
typedef fixed_t angle_t;

/* We'll also use these for polar conversions */
typedef struct {
    fixed_t x,y;
} rect_point;

typedef struct {
    fixed_t r;
    angle_t t;
} polar_point;

/* Angles run from 0 to 256 (like 0 to 2pi) */
#define zeroangle (angle_t)(0x00000000)
#define maxangle (angle_t)(0x01000000)
#define makeangle(in,fr) (angle_t)((((in)<<16) + (fr)) & 0x00FFFFFF);

fixed_t lookup_sin(angle_t ang);
fixed_t lookup_cos(angle_t ang);

rect_point  polar_to_rect(polar_point p);

#if 0 /* deferred */
polar_point rect_to_polar(rect_point p);

fixed_t magnitude(rect_point p);
angle_t argument(rect_point p);
#endif

#endif /* !def GEOMETRY_H */
