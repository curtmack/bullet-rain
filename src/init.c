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

#include "endian.h"
#include "init.h"
#include "resource.h"

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
    init_resources();
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    
    return 0;
}

int stop_all(void)
{
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    stop_resources();
    
    return 0;
}
