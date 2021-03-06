/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * compile.h
 * Compile-time definitions.
 */

#ifndef COMPILE_H

#define COMPILE_H

/*
 * Edit these settings by commenting or uncommenting #defines as
 * needed for your compilation process.
 */

/* 
 * Use 64-bit optimizations and compile for 64-bit architectures
 * Currently has no effect
 */
/* #define SIXTYFOUR */

/* Deprecated: DEBUG - Now set by the makefile */

/* Verbose output */
/* #define VERBOSE_DEBUG */

/* Size of hash map used to store resources */
#define ARCLIST_HASH_SIZE 1024

/* Include "SDL/SDL_***.h" instead of "SDL_***.h", needed on e.g. Ubuntu */
#define INCLUDE_SDL_PREFIX

/* Version string */
#define ENGINE_VERSION "0.2"
/* padded with nulls to 16 chars */
#define ENGINE_VERSION_16 "0.2\0\0\0\0\0\0\0\0\0\0\0\0\0"

/* 
 * Some compilers don't provide these for some reason?
 * Also if you're using some alien version of C I suppose you could change
 * these to whatever your bizarre extraterrestrial compiler needs
 */
#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL  0
#endif

#endif /* !def COMPILE_H */
