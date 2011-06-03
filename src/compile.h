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
/* 
 * Paranoid hash map - accomodate hash collisions, but requires full
 * string checks whenever a hash match is detected
 * Our hashing algorithm has pretty even distribution over the
 * full 32-bit hash space, so this is likely not needed.
 */
/* #define PARANOID_HASH */

/* Compile system tests rather than the usual main() */
#define SYSTEM_TEST

/* Version string */
#define ENGINE_VERSION "prealpha"

#endif /* !def COMPILE_H */
