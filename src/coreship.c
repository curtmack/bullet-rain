/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * coreship.c
 * Contains code for the coreship, a player built into the engine for use
 * in the system test
 */

#include "bullet.h"
#include "collmath.h"
#include "compile.h"
#include "coreship.h"
#include "geometry.h"
#include "input.h"
#include "player.h"
#include "resource.h"

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#else
#include "SDL.h"
#endif

/* Has init_coreship been called? */
int initialized = FALSE;

/* Bullet types */
pbullet_type main_shot;
pbullet_type left_shot;
pbullet_type right_shot;

/* The ship sprite sheet */
SDL_Surface *ship_sprites;

/* Actual sprites */
SDL_Surface *ship_main_sprite;
SDL_Surface *ship_destroy_anim[6];
SDL_Surface *main_shot_sprite;
SDL_Surface *left_shot_sprite;
SDL_Surface *right_shot_sprite;

/* 
 * Where things are on the sprite sheet 
 * Ostensibly these are const, but actually making them const
 * creates warnings about discarding qualifiers
 */
SDL_Rect ship_rect = {0, 0, 16, 16};

SDL_Rect main_shot_rect = {0, 16, 8, 8};
SDL_Rect left_shot_rect = {8, 16, 8, 8};
SDL_Rect right_shot_rect = {16, 16, 8, 8};

SDL_Rect ship_destroy_anim_rect[6] = {
    {16, 0, 16, 16},
    {32, 0, 16, 16},
    {48, 0, 16, 16},
    {64, 0, 16, 16},
    {80, 0, 16, 16},
    {96, 0, 16, 16}
};

/* 
 * Loads resources needed for the coreship
 * Note that this is called by init_player, NOT directly by init_all
 */
int init_coreship (void)
{
    SDL_PixelFormat fmt;
    SDL_Surface *tmp;
    int i;
    
    Uint32 colorkey;
 
    /* Abort if we've done this before */
    if (initialized) return 0;
    
    /* Load the sprites */
    ship_sprites=(SDL_Surface*)(get_res("res/brcore.tgz", "coreship.png")->data);
    
    /* Get pixel format */
    fmt = *(ship_sprites->format);
    
    /* Get color key */
    colorkey = SDL_MapRGBA(&fmt, 255, 0, 255, SDL_ALPHA_OPAQUE);
    
    /* Copy over all of the individual sprites */
    tmp = SDL_CreateRGBSurface(SDL_HWSURFACE, 16, 16, 
        fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
    SDL_BlitSurface(ship_sprites, &ship_rect, tmp, NULL);
    SDL_SetColorKey(tmp, SDL_SRCCOLORKEY, colorkey);
    ship_main_sprite = tmp;
    
    for (i = 0; i < 6; ++i) {
        tmp = SDL_CreateRGBSurface(SDL_HWSURFACE, 16, 16, 
            fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
        SDL_BlitSurface(ship_sprites, &ship_destroy_anim_rect[i], tmp, NULL);
        SDL_SetColorKey(tmp, SDL_SRCCOLORKEY, colorkey);
        ship_destroy_anim[i] = tmp;
    }
    
    tmp = SDL_CreateRGBSurface(SDL_HWSURFACE, 8, 8,
        fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
    SDL_BlitSurface(ship_sprites, &main_shot_rect, tmp, NULL);
    SDL_SetColorKey(tmp, SDL_SRCCOLORKEY, colorkey);
    main_shot_sprite = tmp;
    
    tmp = SDL_CreateRGBSurface(SDL_HWSURFACE, 8, 8,
        fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
    SDL_BlitSurface(ship_sprites, &left_shot_rect, tmp, NULL);
    SDL_SetColorKey(tmp, SDL_SRCCOLORKEY, colorkey);
    left_shot_sprite = tmp;
    
    tmp = SDL_CreateRGBSurface(SDL_HWSURFACE, 8, 8,
        fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
    SDL_BlitSurface(ship_sprites, &right_shot_rect, tmp, NULL);
    SDL_SetColorKey(tmp, SDL_SRCCOLORKEY, colorkey);
    right_shot_sprite = tmp;
    
    /* Create the pbullet_types */
    (main_shot.tl).x       = tofixed(-4,0);
    (main_shot.tl).y       = tofixed(-3,0);
    (main_shot.lr).x       = tofixed(4,0);
    (main_shot.lr).y       = tofixed(4,0);
    (main_shot.drawloc).x  = tofixed(0,0);
    (main_shot.drawloc).y  = tofixed(-1,0);
    main_shot.img          = main_shot_sprite;
    main_shot.flags        = 0;
    main_shot.gameflags    = 0;
    main_shot.enemy_damage = 150;
    main_shot.boss_damage  = 150;
    
    (left_shot.tl).x       = tofixed(-3,0);
    (left_shot.tl).y       = tofixed(-3,0);
    (left_shot.lr).x       = tofixed(3,0);
    (left_shot.lr).y       = tofixed(3,0);
    (left_shot.drawloc).x  = tofixed(-1,0);
    (left_shot.drawloc).y  = tofixed(-1,0);
    left_shot.img          = left_shot_sprite;
    left_shot.flags        = 0;
    left_shot.gameflags    = 0;
    left_shot.enemy_damage = 65;
    left_shot.boss_damage  = 65;
    
    (right_shot.tl).x       = tofixed(-3,0);
    (right_shot.tl).y       = tofixed(-3,0);
    (right_shot.lr).x       = tofixed(3,0);
    (right_shot.lr).y       = tofixed(3,0);
    (right_shot.drawloc).x  = tofixed(-1,0);
    (right_shot.drawloc).y  = tofixed(-1,0);
    right_shot.img          = right_shot_sprite;
    right_shot.flags        = 0;
    right_shot.gameflags    = 0;
    right_shot.enemy_damage = 65;
    right_shot.boss_damage  = 65;
    
    /* And we're done! */
    initialized = TRUE;
    return 0;
}

/* Makes the coreship */
player make_coreship (void)
{
    player ship;
    int i;
    
    if (!initialized) {
        init_coreship();
    }
    
    ship.img = ship_main_sprite;
    ship.anim[0] = ship_main_sprite;
    
    (ship.drawloc).x = tofixed(-8,0);
    (ship.drawloc).y = tofixed(-9,0);
    
    ship.rad = fixone;
    
    for (i = 0; i < 5; ++i) {
        ship.gamedata[i] = 0;
    }
    
    for (i = 1; i <= 6; ++i) {
        ship.anim[i] = ship_destroy_anim[i-1];
    }
    
    ship.flags = 0;
    
    return ship;
}
                                  
/* Registers inputs needed for the coreship and gives them default binds */
void setup_coreship (int id)
{    
    /* Register all the inputs */
    input_register_boolean(CORESHIP_INPUT_UP    + id*5);
    input_register_boolean(CORESHIP_INPUT_DOWN  + id*5);
    input_register_boolean(CORESHIP_INPUT_LEFT  + id*5);
    input_register_boolean(CORESHIP_INPUT_RIGHT + id*5);
    input_register_boolean(CORESHIP_INPUT_SHOOT + id*5);
    
    /* Default binds */
    input_config_key(CORESHIP_INPUT_UP,    SDLK_UP);
    input_config_key(CORESHIP_INPUT_DOWN,  SDLK_DOWN);
    input_config_key(CORESHIP_INPUT_LEFT,  SDLK_LEFT);
    input_config_key(CORESHIP_INPUT_RIGHT, SDLK_RIGHT);
    input_config_key(CORESHIP_INPUT_SHOOT, SDLK_z);
}

/* Updates the coreship */
void update_coreship (int id, player *ship)
{
    /* Update timers */
    if (ship->gamedata[TIME_TO_MAIN_SHOT] > 0) {
        --(ship->gamedata[TIME_TO_MAIN_SHOT]);
    }
    if (ship->gamedata[TIME_TO_SIDE_SHOT] > 0) {
        --(ship->gamedata[TIME_TO_SIDE_SHOT]);
    }
    if (ship->gamedata[DEATH_ANIM] > 0) {
        --(ship->gamedata[DEATH_ANIM]);
    }
    if (ship->gamedata[DEATH_TIMER] > 0) {
        --(ship->gamedata[DEATH_TIMER]);
    }
    
    /* Check inputs and move ship/shoot accordingly */
    if (input_value(CORESHIP_INPUT_UP + id*5)) {
        (ship->center).y -= tofixed(3, 21845);
    }
    if (input_value(CORESHIP_INPUT_DOWN + id*5)) {
        (ship->center).y += tofixed(3, 21845);
    }
    if (input_value(CORESHIP_INPUT_LEFT + id*5)) {
        (ship->center).x -= tofixed(3, 21845);
    }
    if (input_value(CORESHIP_INPUT_RIGHT + id*5)) {
        (ship->center).x += tofixed(3, 21845);
    }
    if (input_value(CORESHIP_INPUT_SHOOT + id*5)) {
        if (ship->gamedata[TIME_TO_MAIN_SHOT] <= 0) {
            ship->gamedata[TIME_TO_MAIN_SHOT] = 5;
            make_pbullet(&main_shot, (ship->center.x),
                         (ship->center).y-tofixed(8,0),
                         fixzero, -tofixed(6,43691), FALSE);
        }
        if (ship->gamedata[TIME_TO_SIDE_SHOT] <= 0) {
            ship->gamedata[TIME_TO_SIDE_SHOT] = 10;
            make_pbullet(&left_shot, (ship->center).x-tofixed(6,0),
                         (ship->center).y-tofixed(6,0), -tofixed(3,21845),
                         -tofixed(5,50692), FALSE);
            make_pbullet(&right_shot, ship->center.x+tofixed(6,0),
                         (ship->center).y-tofixed(6,0), tofixed(3,21845),
                         -tofixed(5,50692), FALSE);
        }
    }
    
    /* That'll do it, we're skipping the whole "death" thing at the moment */
}
