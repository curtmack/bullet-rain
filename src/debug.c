/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * debug.c
 * Contains code for different debug tasks, like creating memory
 * dumps and producing warnings
 */

#include "compile.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"
#else
#include "SDL.h"
#include "SDL_thread.h"
#endif

SDL_mutex *debug_output_lock;

#ifdef DEBUG

void _debug (char *msg1, char *msg2)
{
    SDL_mutexP(debug_output_lock);
    printf("  --> DEBUG: %s %s\n", msg1, msg2);
    SDL_mutexV(debug_output_lock);
    fflush(stdout);
}
void _debugn (char *msg1, int num)
{    
    SDL_mutexP(debug_output_lock);
    printf("  --> DEBUG: %s %d\n", msg1, num);
    SDL_mutexV(debug_output_lock);
    fflush(stdout);
}

#endif /* def DEBUG */

/* These we want in non-debug builds */

void _warn (char *msg1, char *msg2, char *file, int line)
{
    SDL_mutexP(debug_output_lock);
    printf("  --> WARNING at %s line %d: %s %s\n", file, line, msg1, msg2);
    SDL_mutexV(debug_output_lock);
    fflush(stdout);
}
void _warnn (char *msg1, int num, char *file, int line)
{
    SDL_mutexP(debug_output_lock);
    printf("  --> WARNING at %s line %d: %s %d\n", file, line, msg1, num);
    SDL_mutexV(debug_output_lock);
    fflush(stdout);
}

void _memdump(void)
{
    /* TODO: Not much I can do at this point, there's nothing to dump! */
}

void _panic (char *msg1, char *msg2, char *file, int line)
{
    SDL_mutexP(debug_output_lock);
    printf("  --> PANIC at %s line %d: %s %s\n  Shutting down!\n",
                          file,  line,msg1,msg2);
    SDL_mutexV(debug_output_lock);
    fflush(stdout);

    _memdump();
    abort();
}

void _panicn (char *msg1, int num, char *file, int line)
{
    SDL_mutexP(debug_output_lock);
    printf("  --> PANIC at %s line %d: %s %d\n  Shutting down!\n",
                          file,  line,msg1,num);
    fflush(stdout);
    SDL_mutexV(debug_output_lock);

    _memdump();
    abort();
}

int init_debug()
{
    debug_output_lock = SDL_CreateMutex();
    
    return 0;
}
void stop_debug()
{
    SDL_DestroyMutex(debug_output_lock);
}
