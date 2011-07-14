/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * endian.h
 * Contains prototypes for functions for dealing with binary files in an
 * endian-independent way
 */

#ifndef ENDIAN_H

#define ENDIAN_H

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#else
#include "SDL.h"
#endif

/*
 * These are the functions other code will actually use
 * They'll be curried to the appropriate functions when the user's endianness
 * is determined
 */
extern void (*endian_write_16)(FILE *f, Sint16 out);
extern void (*endian_write_32)(FILE *f, Sint32 out);
extern void (*endian_write_64)(FILE *f, Sint64 out);

extern Sint16 (*endian_read_16)(FILE *f);
extern Sint32 (*endian_read_32)(FILE *f);
extern Sint64 (*endian_read_64)(FILE *f);

/* Determine the user's endianness */
extern void init_endian(void);

#endif /* !def ENDIAN_H */
