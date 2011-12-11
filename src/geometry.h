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

/* Angles run from 0 to 256 (like 0 to 2pi) */
#define zeroangle (angle_t)(0x00000000)
#define maxangle (angle_t)(0x01000000)
#define makeangle(in,fr) (angle_t)((((in)<<16) + (fr)) & 0x00FFFFFF);

extern fixed_t lookup_sin(angle_t ang);
extern fixed_t lookup_cos(angle_t ang);

extern void polar_to_rect(fixed_t pr, angle_t pt, fixed_t *x, fixed_t *y);

#if 0 /* deferred */
extern void rect_to_polar(fixed_t px, fixed_t py, fixed_t *r, angle_t *t);

extern fixed_t magnitude(fixed_t px, fixed_t py);
extern angle_t argument(fixed_t px, fixed_t py);
#endif

#endif /* !def GEOMETRY_H */
