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

/* Include debug bindings */
#define DEBUG

/* Size of hash map used to store resources */
#define ARCLIST_HASH_SIZE 1024

/* Compile system tests rather than the usual main() */
#define SYSTEM_TEST

/* Include "SDL/SDL_***.h" instead of "SDL_***.h" */
#define INCLUDE_SDL_PREFIX

/* Version string */
#define ENGINE_VERSION "0.0.0.1"

#endif /* !def COMPILE_H */
