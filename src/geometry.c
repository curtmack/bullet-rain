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

#include "fixed.h"
#include "geometry.h"

/* Precalculated tables */

const fixed_t sines[16] = {
    /*   0 */ tofixed(0,0),      /*  0.00000 */
    /*  16 */ tofixed(0,25080),  /*  0.38268 */
    /*  32 */ tofixed(0,46341),  /*  0.70710 */
    /*  48 */ tofixed(0,60547),  /*  0.92387 */
    /*  64 */ tofixed(1,0),      /*  1.00000 */
    /*  80 */ tofixed(0,60547),  /*  0.92387 */
    /*  96 */ tofixed(0,46341),  /*  0.70710 */
    /* 112 */ tofixed(0,25080),  /*  0.38268 */
    /* 128 */ tofixed(0,0),      /*  0.00000 */
    /* 144 */ tofixed(0,-25080), /* -0.38268 */
    /* 160 */ tofixed(0,-46341), /* -0.70710 */
    /* 176 */ tofixed(0,-60547), /* -0.92387 */
    /* 192 */ tofixed(-1,0),     /* -1.00000 */
    /* 208 */ tofixed(0,-60547), /* -0.92387 */
    /* 224 */ tofixed(0,-46341), /* -0.70710 */
    /* 240 */ tofixed(0,-25080), /* -0.38268 */
};

const fixed_t cosines[16] = {
    /*   0 */ tofixed(1,0),      /*  1.00000 */
    /*  16 */ tofixed(0,60547),  /*  0.92387 */
    /*  32 */ tofixed(0,46341),  /*  0.70710 */
    /*  48 */ tofixed(0,25080),  /*  0.38268 */
    /*  64 */ tofixed(0,0),      /*  0.00000 */
    /*  80 */ tofixed(0,-25080), /* -0.38268 */
    /*  96 */ tofixed(0,-46341), /* -0.70710 */
    /* 112 */ tofixed(0,-60547), /* -0.92387 */
    /* 128 */ tofixed(-1,0),     /* -1.00000 */
    /* 144 */ tofixed(0,-60547), /* -0.92387 */
    /* 160 */ tofixed(0,-46341), /* -0.70710 */
    /* 176 */ tofixed(0,-25080), /* -0.38268 */
    /* 192 */ tofixed(0,0),      /*  0.00000 */
    /* 208 */ tofixed(0,25080),  /*  0.38268 */
    /* 224 */ tofixed(0,46341),  /*  0.70710 */
    /* 240 */ tofixed(0,60547),  /*  0.92387 */
};

fixed_t lookup_sin(angle_t ang)
{
    fixed_t l,h,d;
    int idx;
    
    idx = (int)((ang & 0x00FFFFFF) >> 20);
    
    /* Takes a weighted average of the sines on either side */
    l = sines[idx];
    h = sines[(idx >= 15 ? 0 : idx+1)];
    d = (fixed_t)((ang & 0x000FFFFF) >> 4);
    
    return fixmul(h,d)+fixmul(l,((fixed_t)0x00010000)-d);
}

fixed_t lookup_cos(angle_t ang)
{
    fixed_t l,h,d;
    int idx;
    
    idx = (int)((ang & 0x00FFFFFF) >> 20);
    
    /* Takes a weighted average of the cosines on either side */
    l = cosines[idx];
    h = cosines[(idx >= 15 ? 0 : idx+1)];
    d = (fixed_t)((ang & 0x000FFFFF) >> 4);
    
    return fixmul(h,d)+fixmul(l,((fixed_t)0x00010000)-d);
}

rect_point polar_to_rect(polar_point p)
{
    rect_point r;
    /* Straightforward */
    r.x = fixmul(p.r, lookup_cos(p.t));
    r.y = fixmul(p.r, lookup_sin(p.t));
    return r;
}
