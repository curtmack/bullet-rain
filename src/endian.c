/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * endian.c
 * Contains code for dealing with binary files in an endian-independent way
 */

#include "debug.h"
#include "endian.h"

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#else
#include "SDL.h"
#endif

/* Function pointers, which are used by outside code */
void (*endian_write_16)(FILE *f, Sint16 out);
void (*endian_write_32)(FILE *f, Sint32 out);
void (*endian_write_64)(FILE *f, Sint64 out);

Sint16 (*endian_read_16)(FILE *f);
Sint32 (*endian_read_32)(FILE *f);
Sint64 (*endian_read_64)(FILE *f);

/*
 * Essentially, files are always stored in little-endian. If we're running
 * in a big-endian environment, we switch bytes around as needed.
 */
void little_endian_write_16(FILE *f, Sint16 out)
{
    fwrite((void*)&out, 2, 1, f);
}
void little_endian_write_32(FILE *f, Sint32 out)
{
    fwrite((void*)&out, 4, 1, f);
}
void little_endian_write_64(FILE *f, Sint64 out)
{
    fwrite((void*)&out, 8, 1, f);
}

Sint16 little_endian_read_16(FILE *f)
{
    Sint16 in;
    fread((void*)&in, 2, 1, f);
    return in;
}
Sint32 little_endian_read_32(FILE *f)
{
    Sint32 in;
    fread((void*)&in, 4, 1, f);
    return in;
}
Sint64 little_endian_read_64(FILE *f)
{
    Sint64 in;
    fread((void*)&in, 8, 1, f);
    return in;
}

/* For the big-endian functions, we need to flip bytes around */
void big_endian_write_16(FILE *f, Sint16 out)
{
    Sint16 tmp;
    tmp = ((out&0x00FF) << 8) +
          ((out&0xFF00) >> 8);
    fwrite((void*)&tmp, 2, 1, f);
}
void big_endian_write_32(FILE *f, Sint32 out)
{
    Sint32 tmp;
    tmp = ((out&0x000000FF) << 24) +
          ((out&0x0000FF00) <<  8) +
          ((out&0x00FF0000) >>  8) +
          ((out&0xFF000000) >> 24);
    fwrite((void*)&tmp, 4, 1, f);
}
void big_endian_write_64(FILE *f, Sint64 out)
{
    Sint64 tmp;
    tmp = ((out&0x00000000000000FFL) << 56) +
          ((out&0x000000000000FF00L) << 40) +
          ((out&0x0000000000FF0000L) << 24) +
          ((out&0x00000000FF000000L) <<  8) +
          ((out&0x000000FF00000000L) >>  8) +
          ((out&0x0000FF0000000000L) >> 24) +
          ((out&0x00FF000000000000L) >> 40) +
          ((out&0xFF00000000000000L) >> 56);
    fwrite((void*)&tmp, 8, 1, f);
}

Sint16 big_endian_read_16(FILE *f)
{
    Sint16 in, tmp;
    fread((void*)&in, 2, 1, f);
    tmp = ((in&0x00FF) << 8) +
          ((in&0xFF00) >> 8);
    return tmp;
}
Sint32 big_endian_read_32(FILE *f)
{
    Sint32 in, tmp;
    fread((void*)&in, 4, 1, f);
    tmp = ((in&0x000000FF) << 24) +
          ((in&0x0000FF00) <<  8) +
          ((in&0x00FF0000) >>  8) +
          ((in&0xFF000000) >> 24);
    return tmp;
}
Sint64 big_endian_read_64(FILE *f)
{
    Sint64 in, tmp;
    fread((void*)&in, 8, 1, f);
    tmp = ((in&0x00000000000000FFL) << 56) +
          ((in&0x000000000000FF00L) << 40) +
          ((in&0x0000000000FF0000L) << 24) +
          ((in&0x00000000FF000000L) <<  8) +
          ((in&0x000000FF00000000L) >>  8) +
          ((in&0x0000FF0000000000L) >> 24) +
          ((in&0x00FF000000000000L) >> 40) +
          ((in&0xFF00000000000000L) >> 56);
    return tmp;
}

/* This sets up the whole thing */
void init_endian(void)
{
    Sint32 mark = 1;
    char *test = (char*)&mark;
    
    if (*test == '\x01') {
        /* little */
        debug("Determined this computer is little-endian");
        endian_write_16 = &little_endian_write_16;
        endian_write_32 = &little_endian_write_32;
        endian_write_64 = &little_endian_write_64;
        endian_read_16  = &little_endian_read_16;
        endian_read_32  = &little_endian_read_32;
        endian_read_64  = &little_endian_read_64;
    }
    else {
        /* big */
        debug("Determined this computer is big-endian");
        endian_write_16 = &big_endian_write_16;
        endian_write_32 = &big_endian_write_32;
        endian_write_64 = &big_endian_write_64;
        endian_read_16  = &big_endian_read_16;
        endian_read_32  = &big_endian_read_32;
        endian_read_64  = &big_endian_read_64;
    }
}
