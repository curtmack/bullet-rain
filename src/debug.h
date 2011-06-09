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
 * For each of these, the suffix means the following
 * 
 * (nothing): Accepts one string message
 * 2:         Accepts two string messages, printed in the given order with a
 *            space between them
 * n:         Accepts a string and a number, printed in that order with a
 *            space between them
 */

/*
 * debug
 * Produce output to stdout only if DEBUG is defined
 */
#define debug(msg1) _debug(msg1, "")
#define debug2(msg1,msg2) _debug(msg1, msg2)
#define debugn(msg1,num) _debugn(msg1, num)

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
#define warnn(cond,msg1,num) \
                        if(cond) {} \
                        else { \
                          _warnn(msg1, num, __FILE__, __LINE__); \
                        }

/* Declarations of all those functions used earlier */
extern void _debug (char *msg1, char *msg2);
extern void _debugn (char *msg1, int num);
extern void _warn (char *msg1, char *msg2, char *file, int line);
extern void _warnn (char *msg1, int num, char *file, int line);

#else   /* def DEBUG */

/* No DEBUG, so evaluate these to nothing */
#define debug(msg1)
#define debug2(msg1,msg2)
#define debugn(msg1,num)
#define warn(cond,msg1)
#define warn2(cond,msg1,msg2)
#define warnn(cond,msg1,num)

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
#define panicn(cond,msg1,num) \
                         if(cond) {} \
                         else { \
                           _panicn(msg1, num, __FILE__, __LINE__); \
                         }
                         
extern void _panic (char *msg1, char *msg2, char *file, int line);
extern void _panicn (char *msg1, int num, char *file, int line);

#endif /* !def DEBUG_H */
