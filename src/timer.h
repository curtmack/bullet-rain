/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * timer.h
 * Contains definitions and prototypes for the timer system.
 */

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#else
#include "SDL.h"
#endif

#ifndef TIMER_H

#define TIMER_H

/*
 * This system creates a thread that increments an int at 60hz.
 * It accumulates error to get as close to 60 hz as possible.
 * 
 * It will take well over 2 years of continuous running for the clock to
 * overflow, so I don't think it's a huge issue.
 * 
 * There's no need to synchronize access to the clock variable, as long as
 * the timer thread is the only thread that modifies it. So don't change it
 * yourself, you silly head.
 */

/* These are the only externally-facing parts of the system */
extern Uint32 clock_60hz(void);

/* Start/stop functions */
extern int  init_timer(void);
extern void stop_timer(void);

#endif /* !def TIMER_H */
