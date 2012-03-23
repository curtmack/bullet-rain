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

extern void polar_to_rect(float pr, float pt, float *x, float *y);

#if 0 /* deferred */
extern void rect_to_polar(float px, float py, float *r, float *t);

extern float magnitude(float px, float py);
extern float argument(float px, float py);
#endif

#endif /* !def GEOMETRY_H */
