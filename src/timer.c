/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * timer.c
 * Contains code for the timer system.
 */

#include "compile.h"
#include "timer.h"
#include "debug.h"
#include <stdlib.h> /* for abs */

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"
#else
#include "SDL.h"
#include "SDL_thread.h"
#endif

/* The value in question */
Uint32 _clock_60hz;
SDL_mutex *_clock_lock;

SDL_Thread *_timer_thread;
SDL_mutex *_timer_kill_lock;
int _timer_kill = FALSE;

SDL_sem *_timer_pulse_sem;

/* This function gets the current clock value */
Uint32 clock_60hz(void)
{
    int r;
    Uint32 ret;
    r = SDL_mutexP(_clock_lock);
    check_mutex(r);
    ret = _clock_60hz;
    r = SDL_mutexV(_clock_lock);
    check_mutex(r);
    return ret;
}

/* 
 * For error calculation 
 * We keep track in nanoseconds, but between the actual resolution of the clock
 * and multithreading delays, we can only guarantee a safe interval of about
 * 10 milliseconds, conservatively
 * Whenever the accumulated error is more than this resolution (absolutely),
 * we add or subtract 10 ms from the next interval length, depending on
 * what the sign of the error is
 */
#define NANO (1000000)
#define INTERVAL_RESOLUTION (10000000)
/* better to be slightly under 60hz than slightly over, so round up */
#define DESIRED_INTERVAL (16666667)
#define NEXT_INTERVAL(error) \
    (abs(error) < INTERVAL_RESOLUTION ?                                   \
     (DESIRED_INTERVAL/INTERVAL_RESOLUTION) * INTERVAL_RESOLUTION :       \
     ((error) < 0 ?                                                       \
        (DESIRED_INTERVAL/INTERVAL_RESOLUTION+1) * INTERVAL_RESOLUTION :  \
        (DESIRED_INTERVAL/INTERVAL_RESOLUTION-1) * INTERVAL_RESOLUTION)   \
    )
#define UPDATE_ERROR(interval,error) ((error) += ((interval)-DESIRED_INTERVAL))

/* 
 * This function updates with the SDL timer and messes with the pulse
 * semaphore so that the timer can update
 */
Uint32 _catch_timer_pulse(Uint32 interval, void *param)
{
    
    int *error = (int*)param;
    Uint32 next_int;
    
    /* Post the semaphore, this unblocks the timer */
    SDL_SemPost(_timer_pulse_sem);
    
    /* Figure out the next interval */
    next_int = NEXT_INTERVAL(*error);
    UPDATE_ERROR(next_int,*error);
    
    /* Tell SDL to start the next timer */
    return (next_int / NANO);
    
}

/* The thread function that does it all */
int timer_thread(void *unused)
{
    int error = 0, r;
    
    Uint32 init_interval;
    
    SDL_TimerID timer;
    
    /* Create the pulse semaphore, this is part of how we get clock ticks */
    _timer_pulse_sem = SDL_CreateSemaphore(0);
    
    /* Find the initial interval we should use */
    init_interval = NEXT_INTERVAL(error);
    UPDATE_ERROR(init_interval,error);
    /* Start the timer */
    timer = SDL_AddTimer((init_interval)/NANO, _catch_timer_pulse,
                         (void*) &error);
    
    r = SDL_mutexP(_timer_kill_lock);
    check_mutex(r);
    
    while (!_timer_kill) {
        r = SDL_mutexV(_timer_kill_lock);
        check_mutex(r);
        /* Wait for the pulse semaphore to be updated by the timer callback */
        SDL_SemWait(_timer_pulse_sem);
        r = SDL_mutexP(_clock_lock);
        check_mutex(r);
        ++_clock_60hz;
        r = SDL_mutexV(_clock_lock);
        check_mutex(r);
        r = SDL_mutexP(_timer_kill_lock);
        check_mutex(r);
    }
    
    /* Destroy the semaphore */
    SDL_DestroySemaphore(_timer_pulse_sem);
    
    /* Stop the timer */
    SDL_RemoveTimer(timer);
    
    return 0;
    
}

/* Start the timer */
int init_timer()
{
    _timer_kill_lock = SDL_CreateMutex();
    _clock_lock = SDL_CreateMutex();
    _clock_60hz = 0;
    _timer_thread = SDL_CreateThread(timer_thread, NULL);
    
    return 0;
}

/* Stop the timer */
void stop_timer()
{
    SDL_mutexP(_timer_kill_lock);
    _timer_kill = TRUE;
    SDL_mutexV(_timer_kill_lock);
    SDL_WaitThread(_timer_thread, NULL);
    SDL_DestroyMutex(_timer_kill_lock);
}
