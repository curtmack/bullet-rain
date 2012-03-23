/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * geometry.c
 * Contains code for working with angles and points
 */

#include "compile.h"
#include "geometry.h"
#include <math.h>

void polar_to_rect(float pr, float pt, float *x, float *y)
{
    /* Straightforward */
    if (x != NULL) *x = pr * cos(pt);
    if (y != NULL) *y = pr * sin(pt);
}
