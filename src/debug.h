/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * debug.h
 * Contains macros for assertions and performing different debug
 * tasks, like creating memory dumps and such
 */

#include "compile.h"

#ifndef DEBUG_H

#define DEBUG_H

#ifdef DEBUG

/*
 * debug
 * Produce output to stdout only if DEBUG is defined
 */
#define debug(msg1) _debug(msg1, "")
#define debug2(msg1,msg2) _debug(msg1, msg2)

/*
 * warn
 * If cond is false, output a message to stderr, but resume.
 */
#define warn(cond,msg1) \
						if(cond) {} \
                        else { \
                          _warn(msg1, "", __FILE__, __LINE__); \
                        }
#define warn2(cond,msg1,msg2) \
						if(cond) {} \
                        else { \
                          _warn(msg1, msg2, __FILE__, __LINE__); \
                        }

/* Declarations of all those functions used earlier */
extern void _debug (char *msg1, char *msg2);
extern void _warn (char *msg1, char *msg2, char *file, int line);

#else   /* def DEBUG */

/* No DEBUG, so evaluate these to nothing */
#define debug(msg1)
#define debug2(msg1,msg2)
#define warn(cond,msg1)
#define warn2(cond,msg1,msg2)

#endif /* def DEBUG */

/* This we want in non-debug builds */

/* 
 * panic
 * If cond is false, create a memory dump and quit the engine.
 */
#define panic(cond,msg1) \
						 if(cond) {} \
                         else { \
                           _panic(msg1, "", __FILE__, __LINE__); \
                         }
#define panic2(cond,msg1,msg2) \
						 if(cond) {} \
                         else { \
                           _panic(msg1, msg2, __FILE__, __LINE__); \
                         }
                         
extern void _panic (char *msg1, char *msg2, char *file, int line);

#endif /* !def DEBUG_H */
