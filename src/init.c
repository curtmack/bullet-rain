/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * init.c
 * Contains code for initializing and stopping all subsystems
 */

#include "debug.h"
#include "init.h"
#include "input.h"
#include "player.h"
#include "resource.h"
#include "timer.h"
#include "bullet.h"
#include "scripts.h"

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_image.h"
#else
#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#endif

int init_all(void)
{
    init_debug();
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    init_timer();
    init_resources();
    init_inputs();
    init_bullets();
    init_player();
    init_scripts();
    
    return 0;
}

void stop_all(void)
{
    stop_scripts();
    stop_player();
    stop_bullets();
    stop_inputs();
    stop_resources();
    stop_timer();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    stop_debug();
}
