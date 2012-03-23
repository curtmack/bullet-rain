/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * systest.c
 * Contains an alternative main() that allows for testing
 * of the various subsystems used by the engine if SYSTEM_TEST
 * is set in compile.h
 */

#include "compile.h"

#ifdef SYSTEM_TEST

#include "bullet.h"
#include "coreship.h"
#include "debug.h"
#include "geometry.h"
#include "init.h"
#include "input.h"
#include "menu.h"
#include "player.h"
#include "resource.h"
#include "timer.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"
#include "SDL/SDL_ttf.h"
#else
#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_ttf.h"
#endif

void floatest(SDL_Surface *surface, TTF_Font *font);
void geom_test(SDL_Surface *surface, TTF_Font *font);
void hash_test(SDL_Surface *surface, TTF_Font *font);
void res_test(SDL_Surface *surface, TTF_Font *font);
void timer_test(SDL_Surface *surface, TTF_Font *font);
void input_test(SDL_Surface *surface, TTF_Font *font);
void bull_test_fake_proc(SDL_Surface *surface, TTF_Font *font);
void bull_test_collision(SDL_Surface *surface, TTF_Font *font);
void player_test(SDL_Surface *surface, TTF_Font *font);

#define TEST_MENU_SIZE 6

#define TEST_TIMER     0
#define TEST_INPUT     1
#define TEST_FAKEBULL  2
#define TEST_COLLISION 3
#define TEST_PLAYER    4
#define TEST_QUIT      5

const char menu[TEST_MENU_SIZE][32] = {
    "60 hz timer test",
    "Input test",
    "Bullet test",
    "Collision test",
    "Player test",
    "Quit the system test"
};
    
const SDL_Color off = {255, 255, 255}; /* white   */
const SDL_Color on  = {255,   0,   0}; /* red     */

#define rectset(rect,nx,ny,nw,nh) rect.x=nx;rect.y=ny;rect.w=nw;rect.h=nh

brmenu *construct_menu(SDL_Surface *surface, TTF_Font *font, resource *logo)
{
    brmenu *brm;
    
    int i, prev, next, menu_x, menu_y, menu_width, menu_height;
    
    char versionstring[64];
    
    SDL_Surface *back;
    SDL_Surface *logoimg;
    SDL_Surface *version;
    SDL_Surface *offs[TEST_MENU_SIZE];
    SDL_Surface *ons [TEST_MENU_SIZE];
    
    SDL_Rect rect, offsrc, offdst, onsrc, ondst;
    const Uint32 bg = SDL_MapRGB(surface->format, 0, 0, 32); /* dk.blue */
    
    for (i = 0; i < TEST_MENU_SIZE; ++i) {
        offs[i] = TTF_RenderText_Solid(font, menu[i], off);
        ons [i] = TTF_RenderText_Solid(font, menu[i], on );
    }
    
    /* 
     * This just makes an empty surface the same size and format as our screen
     * surface.
     * And yes, SDL does need a CreateSurfaceFromExistingSurface function.
     */
    back = SDL_CreateRGBSurface(SDL_SWSURFACE, surface->w, surface->h,
                                surface->format->BitsPerPixel,
                                surface->format->Rmask, surface->format->Gmask,
                                surface->format->Bmask, surface->format->Amask);
    /* Fill it with black */
    SDL_FillRect(back, NULL, bg);
    
    /* Draw the logos in */
    logoimg = (SDL_Surface*)(logo->data);
    rectset(rect, surface->w - logoimg->w, surface->h - logoimg->h, 0, 0);
    SDL_BlitSurface(logoimg, NULL, back, NULL); /* top-left corner */
    SDL_BlitSurface(logoimg, NULL, back, &rect); /* lower-right corner */
    
    /* Draw the engine version in */
    sprintf(versionstring, "Engine version %s", ENGINE_VERSION);
    version = TTF_RenderText_Solid(font, versionstring, off);
    rectset(rect, surface->w - version->w, 0, 0, 0);
    SDL_BlitSurface(version, NULL, back, &rect); /* top-right corner */
    rectset(rect, 0, surface->h - version->h, 0, 0);
    SDL_BlitSurface(version, NULL, back, &rect); /* lower-left corner */
    
    /* Figure out dimensions */
    menu_width = 0;
    menu_height = 0;
    /* Get maximum width and total height */
    for (i = 0; i < 5; ++i) {
        /* 
         * Given the circumstances, we can safely assume offs and ons have
         * the same dimensions
         */
        if (offs[i]->w > menu_width) {
            menu_width = offs[i]->w;
        }
        menu_height += offs[i]->h;
    }
    /* Calculate how to center menu on the screen */
    menu_x = (surface->w - menu_width)/2;
    menu_y = (surface->h - menu_height)/2;
    
    /* Make the menu */
    rectset(offsrc, 0, 0, back->w, back->h);
    rectset(offdst, 0, 0, surface->w, surface->h);
    brm = create_menu(surface, back, offsrc, offdst, NULL, NULL);
              
    /* Add all the entries */
    for (i = 0; i < TEST_MENU_SIZE; ++i) {
        prev = (i > 0 ? i-1 : TEST_MENU_SIZE - 1);
        next = (i < TEST_MENU_SIZE - 1 ? i+1 : 0);
        
        rectset(offsrc, 0, 0, offs[i]->w, offs[i]->h);
        rectset(offdst, menu_x, menu_y, 0, 0 );
        rectset(onsrc, 0, 0, ons[i]->w, ons[i]->h );
        rectset(ondst, menu_x, menu_y, 0, 0 );
        menu_add_entry(brm, i, prev, next, -1, -1, offs[i], offsrc, offdst,
                       ons[i], onsrc, ondst);
        menu_y += offs[i]->h;
    }
    
    /* Link all the entries together */
    menu_link_entries(brm);
    
    /* Clear surfaces */
    SDL_FreeSurface(logoimg);
    SDL_FreeSurface(version);
    
    return brm;
}

int draw_stop = FALSE;

/* 
 * This redraws the menu approximately 60 times a second
 * It's VERY rough, and would obviously only be used for something
 * simple like this
 */
int _drawthread(void *data)
{
    brmenu *brm = (brmenu*)data;
    int r;
    
    draw_stop = FALSE;
    while (!draw_stop) {
        r = SDL_mutexP(brm->_lock);
        check_mutex(r);
        draw_menu(brm);
        SDL_Flip(brm->surface);
        r = SDL_mutexV(brm->_lock);
        check_mutex(r);
        SDL_Delay(16);
    }
    
    return 1;
}

int main(int argc, char *argv[])
{
    int finished = FALSE;
    brmenu *brm;
    arclist *core;
    resource *corner_logo;
    SDL_Surface *screen = NULL;
    SDL_Thread  *drawthread = NULL;
    TTF_Font *font = NULL;

    srand((int) time(NULL));
    
    panic(!init_all(), "Error initializing engine subsystems");
    
    font = TTF_OpenFont("res/monofont.ttf", 16);
    panic(font != NULL, "Couldn't load res/monofont.ttf");
    
    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
    panic(screen != NULL, "Error setting up video mode");
    SDL_WM_SetCaption("BULLET RAIN ENGINE TEST", NULL);
    
    core = load_arc("res/brcore.tgz");
    corner_logo = get_res("res/brcore.tgz", "logosmbk.png");
    
    brm = construct_menu(screen, font, corner_logo);

    while (!finished) {
        start_menu(brm);
        drawthread = SDL_CreateThread(&_drawthread, brm);
        while (brm->running == TRUE) {
            menu_action(brm, get_action());
            SDL_Delay(10);
        }
        
        draw_stop = TRUE;
        
        switch (brm->end) {
            case TEST_TIMER:
                timer_test(screen, font);
                break;
            case TEST_INPUT:
                input_test(screen, font);
                break;
            case TEST_FAKEBULL:
                bull_test_fake_proc(screen, font);
                break;
            case TEST_COLLISION:
                bull_test_collision(screen, font);
                break;
            case TEST_PLAYER:
                player_test(screen, font);
                break;
            case TEST_QUIT:
                finished = TRUE;
                break;
            default:
                debug("Oops! Something's wrong with the menu code!");
        }
    }
    
    destroy_menu(brm);
    TTF_CloseFont(font);
    SDL_FreeSurface(screen);
    
    stop_all();
    
    return 0;
}

void hash_test(SDL_Surface *surface, TTF_Font *font)
{
    char entry[32] = "";
    char prev[32]  = "";
    char hashbuf[12] = "";
    sid_t hash = (sid_t)0x00000000; /* sid of the empty string */
    int cursor = 0;
    int len = 0;
    int i;
    SDL_Surface *back, *tmp;
    SDL_Rect rect;
    SDL_Event event;

    const Uint32 bg = SDL_MapRGB(surface->format, 0, 0, 32); /* dk.blue */
    
    /* Set up the background */
    back = SDL_CreateRGBSurface(SDL_SWSURFACE, surface->w, surface->h,
                                surface->format->BitsPerPixel,
                                surface->format->Rmask, surface->format->Gmask,
                                surface->format->Bmask, surface->format->Amask);
    
    rectset(rect,0,0,surface->w,surface->h);
    SDL_FillRect(back,&rect,bg);
    
    /* Draw in instructions */
    tmp = TTF_RenderText_Solid(font, "Type in string to hash", off);
    rectset(rect,0,back->h-(3*tmp->h),0,0);
    SDL_BlitSurface(tmp,NULL,back,&rect);
    SDL_FreeSurface(tmp);
    
    tmp = TTF_RenderText_Solid(font, "Press enter to calculate hash", off);
    rectset(rect,0,back->h-(2*tmp->h),0,0);
    SDL_BlitSurface(tmp,NULL,back,&rect);
    SDL_FreeSurface(tmp);
    
    tmp = TTF_RenderText_Solid(font, "Escape: Exit to menu", off);
    rectset(rect,0,back->h-tmp->h,0,0);
    SDL_BlitSurface(tmp,NULL,back,&rect);
    SDL_FreeSurface(tmp);
    
    /* Need unicode enabled */
    SDL_EnableUNICODE(1);
    while (TRUE) {
        /* Update the screen */
        rectset(rect,0,0,0,0);
        SDL_BlitSurface(back, NULL, surface, &rect);
        
        /* Draw the cursor */
        /* This assumes the font is indeed monospaced */
        tmp = TTF_RenderText_Solid(font, "0", off);
        rectset(rect,(tmp->w)*cursor,0,tmp->w,tmp->h);
        /* red */
        SDL_FillRect(surface, &rect, SDL_MapRGB(surface->format, 255, 0, 0));
        SDL_FreeSurface(tmp);
        
        /* Draw the entry text */
        tmp = TTF_RenderText_Solid(font, entry, off);
        rectset(rect, 0, 0, 0, 0);
        SDL_BlitSurface(tmp, NULL, surface, &rect);
        SDL_FreeSurface(tmp);
        
        /* Draw the hash text */
        tmp = TTF_RenderText_Solid(font, prev, off);
        if (tmp == NULL) {
            /* This happens if the string is empty */
            tmp = TTF_RenderText_Solid(font, "(Empty string)", on);
        }
        rectset(rect, surface->w - tmp->w, 0, 0, 0);
        SDL_BlitSurface(tmp, NULL, surface, &rect);
        SDL_FreeSurface(tmp);
        
        /* Draw the hash */
        sprintf(hashbuf, "0x%08x", (int)hash);
        tmp = TTF_RenderText_Solid(font, hashbuf, on);
        rectset(rect, surface->w - tmp->w, tmp->h, 0, 0);
        SDL_BlitSurface(tmp, NULL, surface, &rect);
        SDL_FreeSurface(tmp);
        
        /* Update the screen */
        SDL_Flip(surface);
        
        /* Check input */
        SDL_WaitEvent(&event);
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                break;
            }
            else {
                /* Motion */
                switch(event.key.keysym.sym) {
                    case SDLK_LEFT:
                        if (cursor > 0) {
                            --cursor;
                        }
                        break;
                    case SDLK_RIGHT:
                        if (cursor < len && cursor < 31) {
                            ++cursor;
                        }
                        break;
                    
                    case SDLK_BACKSPACE:
                        /* tee hee i'm a clever pony */
                        if (cursor > 0) --cursor;
                        else break;
                    case SDLK_DELETE:
                        /* Bring everything over to the left */
                        for (i = cursor; i < len; ++i) {
                            entry[i] = entry[i+1];
                        }
                        break;
                    case SDLK_RETURN:
                        /* Update the hash */
                        hash = calculate_sid(entry);
                        strcpy(prev, entry);
                        entry[0] = '\0';
                        cursor = 0;
                        len = 0;
                        break;
                    default:
                        /* 
                         * Does it have a unicode component?
                         * We need to check for this last because some of
                         * the keys above have a unicode point (e.g.
                         * '\b' for SDLK_BACKSPACE)
                         */
                        if (event.key.keysym.unicode != 0) {
                            entry[cursor] = (char)event.key.keysym.unicode;
                            if (cursor < 31) ++cursor; /* need room for nul */
                            if (cursor > len) ++len;
                            entry[len] = '\0';
                        }
                        break;
                }
            }
        }
    }
    /* Disable unicode conversion */
    SDL_EnableUNICODE(0);
}

void timer_test(SDL_Surface *surface, TTF_Font *font)
{
    Uint32 start_clock = clock_60hz();
    Uint32 start_ticks = SDL_GetTicks();
    Sint32 clock, ticks;
    float realhz;
    char buffer[64];
    
    SDL_Surface *tmp;
    SDL_Event event;
    SDL_Rect rect;

    const Uint32 bg = SDL_MapRGB(surface->format, 0, 0, 32); /* dk.blue */
    
    while (TRUE) {
        /* Update calculations */
        clock = clock_60hz()   - start_clock;
        ticks = SDL_GetTicks() - start_ticks;
        
        realhz = (ticks==0 ? 60.0F : clock / (ticks/1000.0F)); /* ms to sec */
        
        /* Draw background */
        SDL_FillRect(surface, NULL, bg);
        
        /* Draw in text */
        sprintf(buffer, "%u clock ticks in %u ms", clock, ticks);
        tmp = TTF_RenderText_Solid(font, buffer, off);
        rectset(rect, 0, 0, 0, 0);
        SDL_BlitSurface(tmp, NULL, surface, &rect);
        
        sprintf(buffer, "% g hz error", realhz - 60.0);
        if (fabs(realhz - 60.0) <= 0.2) {
            tmp = TTF_RenderText_Solid(font, buffer, off);
        }
        else {
            tmp = TTF_RenderText_Solid(font, buffer, on);
        }
        rectset(rect, 0, tmp->h, 0, 0);
        SDL_BlitSurface(tmp, NULL, surface, &rect);

        SDL_Flip(surface);
        
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    break;
                }
            }
        }
    }
}

void input_test(SDL_Surface *surface, TTF_Font *font)
{
    float x = 0.0F, y = 0.0F;
    
    int temp_x, temp_y, i;
    
    SDL_Event event;
    SDL_Surface *lgsprite, *oursprite, *text;
    SDL_PixelFormat *fmt;
    SDL_Rect rect;
    
    const Uint32 bg = SDL_MapRGB(surface->format, 0, 0, 32); /* dk.blue */
    
#define INPUT_TEST_UP    0
#define INPUT_TEST_DOWN  1
#define INPUT_TEST_LEFT  2
#define INPUT_TEST_RIGHT 3

#define INPUT_TEST_MOVESPEED 0.25F
    
    /* Get the sprite */
    lgsprite = (SDL_Surface*)(get_res("res/brcore.tgz", "lgbullet.png")->data);
    fmt = lgsprite->format;
    /* copy over the one we actually want */
    rectset(rect, 32, 0, 32, 32);
    oursprite = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY, 32, 32,
                                     fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask,
                                     fmt->Bmask, fmt->Amask);
    SDL_BlitSurface(lgsprite, &rect, oursprite, NULL);
    /* set the color key */
    SDL_SetColorKey(oursprite, SDL_SRCCOLORKEY, SDL_MapRGB(fmt, 255, 0, 255));
    
    /* Get keys from user */
    SDL_FillRect(surface, NULL, bg);
    text = TTF_RenderText_Solid(font, "Press key for up", off);
    temp_x = (surface->w - text->w)/2;
    temp_y = (surface->h - text->h)/2;
    rectset(rect,temp_x,temp_y,0,0);
    SDL_BlitSurface(text, NULL, surface, &rect);
    SDL_Flip(surface);
    while (TRUE) {
        if (SDL_WaitEvent(&event) && event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                /* Abandon thread! */
                SDL_FreeSurface(text);
                SDL_FreeSurface(oursprite);
                return;
            }
            else {
                /* Register and bind the input */
                input_register_boolean(INPUT_TEST_UP);
                input_config_key(INPUT_TEST_UP, event.key.keysym.sym);
                /* Get out of the loop */
                break;
            }
        }
    }
    SDL_FreeSurface(text);
    
    SDL_FillRect(surface, NULL, bg);
    text = TTF_RenderText_Solid(font, "Press key for down", off);
    temp_x = (surface->w - text->w)/2;
    temp_y = (surface->h - text->h)/2;
    rectset(rect,temp_x,temp_y,0,0);
    SDL_BlitSurface(text, NULL, surface, &rect);
    SDL_Flip(surface);
    while (TRUE) {
        if (SDL_WaitEvent(&event) && event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                /* Abandon thread! */
                SDL_FreeSurface(text);
                SDL_FreeSurface(oursprite);
                return;
            }
            else {
                /* Register and bind the input */
                input_register_boolean(INPUT_TEST_DOWN);
                input_config_key(INPUT_TEST_DOWN, event.key.keysym.sym);
                /* Get out of the loop */
                break;
            }
        }
    }
    SDL_FreeSurface(text);
    
    SDL_FillRect(surface, NULL, bg);
    text = TTF_RenderText_Solid(font, "Press key for left", off);
    temp_x = (surface->w - text->w)/2;
    temp_y = (surface->h - text->h)/2;
    rectset(rect,temp_x,temp_y,0,0);
    SDL_BlitSurface(text, NULL, surface, &rect);
    SDL_Flip(surface);
    while (TRUE) {
        if (SDL_WaitEvent(&event) && event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                /* Abandon thread! */
                SDL_FreeSurface(text);
                SDL_FreeSurface(oursprite);
                return;
            }
            else {
                /* Register and bind the input */
                input_register_boolean(INPUT_TEST_LEFT);
                input_config_key(INPUT_TEST_LEFT, event.key.keysym.sym);
                /* Get out of the loop */
                break;
            }
        }
    }
    SDL_FreeSurface(text);
    
    SDL_FillRect(surface, NULL, bg);
    text = TTF_RenderText_Solid(font, "Press key for right", off);
    temp_x = (surface->w - text->w)/2;
    temp_y = (surface->h - text->h)/2;
    rectset(rect,temp_x,temp_y,0,0);
    SDL_BlitSurface(text, NULL, surface, &rect);
    SDL_Flip(surface);
    while (TRUE) {
        if (SDL_WaitEvent(&event) && event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                /* Abandon thread! */
                SDL_FreeSurface(text);
                SDL_FreeSurface(oursprite);
                return;
            }
            else {
                /* Register and bind the input */
                input_register_boolean(INPUT_TEST_RIGHT);
                input_config_key(INPUT_TEST_RIGHT, event.key.keysym.sym);
                /* Get out of the loop */
                break;
            }
        }
    }
    SDL_FreeSurface(text);
    
    while (TRUE) {
        /* Do we have events? */
        while (SDL_PollEvent(&event)) {
            /* Run through the input system */
            if (!update_input(event)) {
                /* Check for escape */
                if (event.type == SDL_KEYDOWN && 
                    event.key.keysym.sym == SDLK_ESCAPE) {
    
                    /* Free our sprite */
                    SDL_FreeSurface(oursprite);
                    
                    /* Unregister inputs */
                    for (i = 0; i < 4; ++i) {
                        unregister_input(i);
                    }
                    
                    /* Exit the test */
                    return;
                }
            }
        }
        
        /* Move the sprite around */
        if (input_value(INPUT_TEST_UP)) {
            y -= INPUT_TEST_MOVESPEED;
        }
        if (input_value(INPUT_TEST_DOWN)) {
            y += INPUT_TEST_MOVESPEED;
        }
        if (input_value(INPUT_TEST_LEFT)) {
            x -= INPUT_TEST_MOVESPEED;
        }
        if (input_value(INPUT_TEST_RIGHT)) {
            x += INPUT_TEST_MOVESPEED;
        }
        
        /* Draw the sprite */
        SDL_FillRect(surface, NULL, bg);
        rect.x = (int)(x + 320);
        rect.y = (int)(y + 240);
        SDL_BlitSurface(oursprite, NULL, surface, &rect);
        SDL_Flip(surface);
        
        /* Keep looping */
    }
    
    /* We never get out of here */
}

void bull_test_fake_proc(SDL_Surface *surface, TTF_Font *font)
{
    bullet *tmp;
    
    bullet_type sm[12];
    bullet_type lg[12];
    
    float velx, vely, px, py;
    
    int i, bullets_made = 0, numbullets = 0;
    Uint32 lasttime = SDL_GetTicks(), newtime, frametotal = 0;
    Uint32 frames[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    float fps;
    char fpsbuf[24];
    
    SDL_Rect rect;
    SDL_Surface *smsprite, *lgsprite, *smtmp, *lgtmp, *fpstmp;
    SDL_Event event;
    
    const SDL_PixelFormat fmt = *(surface->format);
    const int center_x = 320;
    const int center_y = 240;
    const Uint32 bg = SDL_MapRGB(&fmt, 0, 0, 32); /* dk.blue */
    const Uint32 colorkey = SDL_MapRGBA(surface->format, 255, 0, 255, SDL_ALPHA_OPAQUE);
    
    /* Clear all the bullets */
    reset_bullets();
    
    /* Get the resources we need */
    smsprite = (SDL_Surface*)(get_res("res/brcore.tgz", "smbullet.png")->data);
    lgsprite = (SDL_Surface*)(get_res("res/brcore.tgz", "lgbullet.png")->data);
    
    /* Make the bullet types */
    for (i = 0; i < 12; ++i) {
        sm[i].rad       = 4.0F;
        sm[i].flags     = 0;
        sm[i].gameflags = 0;
        sm[i].lua       = NULL;
        sm[i].tlx       = -4.0F;
        sm[i].tly       = -4.0F;
        sm[i].lrx       = 4.0F;
        sm[i].lry       = 4.0F;
        sm[i].drawlocx  = -4.0F;
        sm[i].drawlocy  = -4.0F;
        
        lg[i].rad       = 12.0F;
        lg[i].flags     = 0;
        lg[i].gameflags = 0;
        lg[i].lua       = NULL;
        lg[i].tlx       = -12.0F;
        lg[i].tly       = -12.0F;
        lg[i].lrx       = 12.0F;
        lg[i].lry       = 12.0F;
        lg[i].drawlocx  = -16.0F;
        lg[i].drawlocy  = -16.0F;
        
        /* Now we need to get the images */
        switch (i) {
            case 0:
                smtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,8,8,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,8,0,8,8);
                SDL_BlitSurface(smsprite,&rect, smtmp, NULL);
                SDL_SetColorKey(smtmp, SDL_SRCCOLORKEY, colorkey);
                sm[i].img = smtmp;
                
                lgtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,32,32,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,32,0,32,32);
                SDL_BlitSurface(lgsprite,&rect, lgtmp, NULL);
                SDL_SetColorKey(lgtmp, SDL_SRCCOLORKEY, colorkey);
                lg[i].img = lgtmp;
                
                break;
            case 1:
                smtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,8,8,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,16,0,8,8);
                SDL_BlitSurface(smsprite,&rect, smtmp, NULL);
                SDL_SetColorKey(smtmp, SDL_SRCCOLORKEY, colorkey);
                sm[i].img = smtmp;
                
                lgtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,32,32,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,64,0,32,32);
                SDL_BlitSurface(lgsprite,&rect, lgtmp, NULL);
                SDL_SetColorKey(lgtmp, SDL_SRCCOLORKEY, colorkey);
                lg[i].img = lgtmp;
                
                break;
            case 2:
                smtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,8,8,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,24,0,8,8);
                SDL_BlitSurface(smsprite,&rect, smtmp, NULL);
                SDL_SetColorKey(smtmp, SDL_SRCCOLORKEY, colorkey);
                sm[i].img = smtmp;
                
                lgtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,32,32,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,96,0,32,32);
                SDL_BlitSurface(lgsprite,&rect, lgtmp, NULL);
                SDL_SetColorKey(lgtmp, SDL_SRCCOLORKEY, colorkey);
                lg[i].img = lgtmp;
                
                break;
            case 3:
                smtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,8,8,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,0,8,8,8);
                SDL_BlitSurface(smsprite,&rect, smtmp, NULL);
                SDL_SetColorKey(smtmp, SDL_SRCCOLORKEY, colorkey);
                sm[i].img = smtmp;
                
                lgtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,32,32,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,0,32,32,32);
                SDL_BlitSurface(lgsprite,&rect, lgtmp, NULL);
                SDL_SetColorKey(lgtmp, SDL_SRCCOLORKEY, colorkey);
                lg[i].img = lgtmp;
                
                break;
            case 4:
                smtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,8,8,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,8,8,8,8);
                SDL_BlitSurface(smsprite,&rect, smtmp, NULL);
                SDL_SetColorKey(smtmp, SDL_SRCCOLORKEY, colorkey);
                sm[i].img = smtmp;
                
                lgtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,32,32,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,32,32,32,32);
                SDL_BlitSurface(lgsprite,&rect, lgtmp, NULL);
                SDL_SetColorKey(lgtmp, SDL_SRCCOLORKEY, colorkey);
                lg[i].img = lgtmp;
                
                break;
            case 5:
                smtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,8,8,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,16,8,8,8);
                SDL_BlitSurface(smsprite,&rect, smtmp, NULL);
                SDL_SetColorKey(smtmp, SDL_SRCCOLORKEY, colorkey);
                sm[i].img = smtmp;
                
                lgtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,32,32,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,64,32,32,32);
                SDL_BlitSurface(lgsprite,&rect, lgtmp, NULL);
                SDL_SetColorKey(lgtmp, SDL_SRCCOLORKEY, colorkey);
                lg[i].img = lgtmp;
                
                break;
            case 6:
                smtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,8,8,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,8,16,8,8);
                SDL_BlitSurface(smsprite,&rect, smtmp, NULL);
                SDL_SetColorKey(smtmp, SDL_SRCCOLORKEY, colorkey);
                sm[i].img = smtmp;
                
                lgtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,32,32,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,32,64,32,32);
                SDL_BlitSurface(lgsprite,&rect, lgtmp, NULL);
                SDL_SetColorKey(lgtmp, SDL_SRCCOLORKEY, colorkey);
                lg[i].img = lgtmp;
                
                break;
            case 7:
                smtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,8,8,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,16,16,8,8);
                SDL_BlitSurface(smsprite,&rect, smtmp, NULL);
                SDL_SetColorKey(smtmp, SDL_SRCCOLORKEY, colorkey);
                sm[i].img = smtmp;
                
                lgtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,32,32,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,64,64,32,32);
                SDL_BlitSurface(lgsprite,&rect, lgtmp, NULL);
                SDL_SetColorKey(lgtmp, SDL_SRCCOLORKEY, colorkey);
                lg[i].img = lgtmp;
                
                break;
            case 8:
                smtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,8,8,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,24,16,8,8);
                SDL_BlitSurface(smsprite,&rect, smtmp, NULL);
                SDL_SetColorKey(smtmp, SDL_SRCCOLORKEY, colorkey);
                sm[i].img = smtmp;
                
                lgtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,32,32,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,96,64,32,32);
                SDL_BlitSurface(lgsprite,&rect, lgtmp, NULL);
                SDL_SetColorKey(lgtmp, SDL_SRCCOLORKEY, colorkey);
                lg[i].img = lgtmp;
                
                break;
            case 9:
                smtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,8,8,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,0,24,8,8);
                SDL_BlitSurface(smsprite,&rect, smtmp, NULL);
                SDL_SetColorKey(smtmp, SDL_SRCCOLORKEY, colorkey);
                sm[i].img = smtmp;
                
                lgtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,32,32,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,0,96,32,32);
                SDL_BlitSurface(lgsprite,&rect, lgtmp, NULL);
                SDL_SetColorKey(lgtmp, SDL_SRCCOLORKEY, colorkey);
                lg[i].img = lgtmp;
                
                break;
            case 10:
                smtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,8,8,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,8,24,8,8);
                SDL_BlitSurface(smsprite,&rect, smtmp, NULL);
                SDL_SetColorKey(smtmp, SDL_SRCCOLORKEY, colorkey);
                sm[i].img = smtmp;
                
                lgtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,32,32,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,32,96,32,32);
                SDL_BlitSurface(lgsprite,&rect, lgtmp, NULL);
                SDL_SetColorKey(lgtmp, SDL_SRCCOLORKEY, colorkey);
                lg[i].img = lgtmp;
                
                break;
            case 11:
                smtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,8,8,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,16,24,8,8);
                SDL_BlitSurface(smsprite,&rect, smtmp, NULL);
                SDL_SetColorKey(smtmp, SDL_SRCCOLORKEY, colorkey);
                sm[i].img = smtmp;
                
                lgtmp = SDL_CreateRGBSurface(SDL_HWSURFACE,32,32,
                    fmt.BitsPerPixel,fmt.Rmask,fmt.Gmask,fmt.Bmask,fmt.Amask);
                rectset(rect,64,96,32,32);
                SDL_BlitSurface(lgsprite,&rect, lgtmp, NULL);
                SDL_SetColorKey(lgtmp, SDL_SRCCOLORKEY, colorkey);
                lg[i].img = lgtmp;
                
                break;
        }
    }
    
    while (TRUE) {
        /* Blank the screen */
        SDL_FillRect(surface, NULL, bg);
        
        /* Process ALL the bullets! */
        for (i = 0; i < 8192; ++i) {
            tmp = &bullet_mem[i];
            if (is_alive(tmp)) {
                if (process_bullet(tmp)) {
                    --numbullets;
                }
            }
            /* flooding screen with bullets is bad, hence the limit */
            else if (bullets_made < 12) {
                px   = (rand()%40960-20480) / 64.0F;
                py   = (rand()%30720-15360) / 64.0F;
                velx = (rand()%256-128) / 64.0F;
                vely = (rand()%256-128) / 64.0F;
                if (rand()%2) {
                    if (make_bullet(px, py, velx, vely, &sm[rand()%12]) != NULL) {
                        ++numbullets;
                        ++bullets_made;
                    }
                }
                else {
                    if (make_bullet(px, py, velx, vely, &lg[rand()%12]) != NULL) {
                        ++numbullets;
                        ++bullets_made;
                    }
                }
            }
            
            draw_bullet(tmp, surface, center_x, center_y);
        }
        
        /* Update time information */
        bullets_made = 0;
        
        newtime = SDL_GetTicks();
        
        frametotal += newtime - lasttime;
        frametotal -= frames[0];
        
        for (i = 0; i < 11; ++i) {
            frames[i] = frames[i+1];
        }
        
        frames[11] = newtime - lasttime;
        lasttime = newtime;
        
        /* Display FPS */
        if (frametotal == 0) {
            fps = 999.9F; /* arbitrary number */
        }
        else {
            fps = 12.0F / (frametotal/1000.0F); /* time is in milliseconds */
        }
        sprintf(fpsbuf, "%d @ %.2f fps", numbullets, fps);
        fpstmp = TTF_RenderText_Solid(font, fpsbuf, off);
        SDL_BlitSurface(fpstmp, NULL, surface, NULL);
        
        SDL_Flip(surface);
        
        SDL_Delay(1);
        
        /* Should we quit? */
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    break;
                }
            }
        }
    }
    reset_bullets();
}

void bull_test_collision(SDL_Surface *surface, TTF_Font *font)
{
    bullet *tmp;
    
    bullet_type miss[2];
    bullet_type hit[2];
    
    float px, py;
    
    int i, j, mouse_x, mouse_y;
    
    SDL_Rect rect;
    SDL_Surface *smsprite, *lgsprite, *stmp;
    SDL_Event event;
    
    const SDL_PixelFormat fmt = *(surface->format);
    const int center_x = 320;
    const int center_y = 240;
    const Uint32 bg = SDL_MapRGB(&fmt, 0, 0, 32); /* dk.blue */
    const Uint32 colorkey = SDL_MapRGBA(surface->format, 255, 0, 255, SDL_ALPHA_OPAQUE);
    
    /* Clear all the bullets */
    reset_bullets();
    
    /* Get the resources we need */
    smsprite = (SDL_Surface*)(get_res("res/brcore.tgz", "smbullet.png")->data);
    lgsprite = (SDL_Surface*)(get_res("res/brcore.tgz", "lgbullet.png")->data);
    
    /* Make the bullet types */
    
    miss[0].rad       = 4.0F;
    miss[0].flags     = 0;
    miss[0].gameflags = 0;
    miss[0].lua       = NULL;
    miss[0].tlx       = -4.0F;
    miss[0].tly       = -4.0F;
    miss[0].lrx       = 4.0F;
    miss[0].lry       = 4.0F;
    miss[0].drawlocx  = -4.0F;
    miss[0].drawlocy  = -4.0F;
    
    hit[0].rad       = 4.0F;
    hit[0].flags     = 0;
    hit[0].gameflags = 0;
    hit[0].lua       = NULL;
    hit[0].tlx       = -4.0F;
    hit[0].tly       = -4.0F;
    hit[0].lrx       = 4.0F;
    hit[0].lry       = 4.0F;
    hit[0].drawlocx  = -4.0F;
    hit[0].drawlocy  = -4.0F;
    
    miss[1].rad       = 12.0F;
    miss[1].flags     = 0;
    miss[1].gameflags = 1; /* we're cheating here, this means it's big */
    miss[1].lua       = NULL;
    miss[1].tlx       = -12.0F;
    miss[1].tly       = -12.0F;
    miss[1].lrx       = 12.0F;
    miss[1].lry       = 12.0F;
    miss[1].drawlocx  = -16.0F;
    miss[1].drawlocy  = -16.0F;
    
    hit[1].rad       = 12.0F;
    hit[1].flags     = 0;
    hit[1].gameflags = 1;
    hit[1].lua       = NULL;
    hit[1].tlx       = -12.0F;
    hit[1].tly       = -12.0F;
    hit[1].lrx       = 12.0F;
    hit[1].lry       = 12.0F;
    hit[1].drawlocx  = -16.0F;
    hit[1].drawlocy  = -16.0F;
        
    /* Now we need to get the images */
    
    stmp = SDL_CreateRGBSurface(SDL_HWSURFACE, 8, 8, fmt.BitsPerPixel,
                        fmt.Rmask, fmt.Gmask, fmt.Bmask, fmt.Amask);
    rectset(rect,24,24,8,8);
    SDL_BlitSurface(smsprite,&rect, stmp, NULL);
    SDL_SetColorKey(stmp, SDL_SRCCOLORKEY, colorkey);
    miss[0].img = stmp;
    
    stmp = SDL_CreateRGBSurface(SDL_HWSURFACE, 32, 32, fmt.BitsPerPixel,
                        fmt.Rmask, fmt.Gmask, fmt.Bmask, fmt.Amask);
    rectset(rect,0,0,32,32);
    SDL_BlitSurface(lgsprite,&rect, stmp, NULL);
    SDL_SetColorKey(stmp, SDL_SRCCOLORKEY, colorkey);
    miss[1].img = stmp;
    
    stmp = SDL_CreateRGBSurface(SDL_HWSURFACE, 8, 8, fmt.BitsPerPixel,
                        fmt.Rmask, fmt.Gmask, fmt.Bmask, fmt.Amask);
    rectset(rect,16,0,8,8);
    SDL_BlitSurface(smsprite,&rect, stmp, NULL);
    SDL_SetColorKey(stmp, SDL_SRCCOLORKEY, colorkey);
    hit[0].img = stmp;
    
    stmp = SDL_CreateRGBSurface(SDL_HWSURFACE, 32, 32, fmt.BitsPerPixel,
                        fmt.Rmask, fmt.Gmask, fmt.Bmask, fmt.Amask);
    rectset(rect,64,64,32,32);
    SDL_BlitSurface(lgsprite,&rect, stmp, NULL);
    SDL_SetColorKey(stmp, SDL_SRCCOLORKEY, colorkey);
    hit[1].img = stmp;
    
    /* 
     * For this test, we're going to create a matrix of stationary bullets
     * makes it easier to test all the hitboxes
     */
    for (i = 0; i < 12; ++i) {
        for (j = 0; j < 16; ++j) {
            px   = (640.0F/17.0F)*(j+1) - center_x;
            py   = (480.0F/13.0F)*(i+1) - center_y;
            
            if ((i+j) % 2 == 1) {
                make_bullet(px, py, 0.0F, 0.0F, &miss[0]);
            }
            else {
                make_bullet(px, py, 0.0F, 0.0F, &miss[1]);
            }
        }
    }
    
    while (TRUE) {
        /* Blank the screen */
        SDL_FillRect(surface, NULL, bg);
        
        /* Update mouse */
        SDL_GetMouseState(&mouse_x, &mouse_y);
        /* need this relative to the center */
        mouse_x -= center_x;
        mouse_y -= center_y;
        px = (float) mouse_x;
        py = (float) mouse_y;
        
        /* We may as well process all the bullets, most of them are gone */
        for (i = 0; i < 8192; ++i) {
            tmp = &bullet_mem[i];
            if (is_alive(tmp)) {
                process_bullet(tmp);
                if(collide_bullet(tmp, px, py, 0.0F)) {
                    if(tmp->gameflags) {
                        tmp->img = hit[1].img;
                    }
                    else {
                        tmp->img = hit[0].img;
                    }
                }
                else {
                    if(tmp->gameflags) {
                        tmp->img = miss[1].img;
                    }
                    else {
                        tmp->img = miss[0].img;
                    }
                }
            }
            
            draw_bullet(tmp, surface, center_x, center_y);
        }
        
        SDL_Flip(surface);
        
        /* Should we quit? */
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    break;
                }
            }
        }
    }
    reset_bullets();
}

void player_test(SDL_Surface *surface, TTF_Font *font)
{
    player ship;
    SDL_Event event;
    Uint32 last_clock_tick;
    SDL_Surface *temp, *tempsrc;
    SDL_PixelFormat *fmt;
    SDL_Rect rect;
    bullet_type enemy;
    bullet_type shot_a;
    bullet_type shot_b;
    pbullet *tmp;
    bullet *tmpb;
    int i,j;
    int player_died = FALSE;
    int deaths = 0;
    float xvel, yvel;
    float dir;
    char deathstring[20];
    
#define ENEMY_TIMER  120
#define SHOT_A_TIMER 120
#define SHOT_B_TIMER 60

    int next_enemy_timer = ENEMY_TIMER;
    int next_shot_a_timer = SHOT_A_TIMER;
    int next_shot_b_timer = SHOT_B_TIMER;

    const Uint32 bg = SDL_MapRGB(surface->format, 0, 0, 32); /* dk.blue */
    const Uint32 colorkey = SDL_MapRGBA(surface->format, 255, 0, 255, SDL_ALPHA_OPAQUE);
    
    reset_pbullets();
    reset_bullets();
    
    ship = make_coreship();
    setup_coreship(0);
    ship.centerx = 0.0F;
    ship.centery = 0.0F;
    
    tempsrc = (SDL_Surface*)(get_res("res/brcore.tgz", "enemy.png")->data);
    SDL_SetColorKey(tempsrc, SDL_SRCCOLORKEY, colorkey);
    enemy.img = tempsrc;
    enemy.drawlocx  = -16.0F;
    enemy.drawlocy  = -16.0F;
    enemy.rad       = 16.0F;
    enemy.tlx       = -16.0F;
    enemy.tly       = -16.0F;
    enemy.lrx       = 16.0F;
    enemy.lry       = 16.0F;
    enemy.flags     = ENEMY;
    enemy.gameflags = 0;
    
    tempsrc = (SDL_Surface*)(get_res("res/brcore.tgz", "lgbullet.png")->data);
    fmt = tempsrc->format;
    temp = SDL_CreateRGBSurface(SDL_SWSURFACE,32,32,fmt->BitsPerPixel,
                                fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask);
    rectset(rect, 64, 32, 32, 32);
    SDL_BlitSurface(tempsrc, &rect, temp, NULL);
    SDL_SetColorKey(temp, SDL_SRCCOLORKEY, colorkey);
    shot_a.img = temp;
    shot_a.drawlocx  = -16.0F;
    shot_a.drawlocy  = -16.0F;
    shot_a.rad       = 12.0F;
    shot_a.tlx       = -12.0F;
    shot_a.tly       = -12.0F;
    shot_a.lrx       = 12.0F;
    shot_a.lry       = 12.0F;
    shot_a.flags     = 0;
    shot_a.gameflags = 0;
    
    tempsrc = (SDL_Surface*)(get_res("res/brcore.tgz", "smbullet.png")->data);
    fmt = tempsrc->format;
    temp = SDL_CreateRGBSurface(SDL_SWSURFACE,8,8,fmt->BitsPerPixel,
                                fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask);
    rectset(rect, 0, 24, 8, 8);
    SDL_BlitSurface(tempsrc, &rect, temp, NULL);
    SDL_SetColorKey(temp, SDL_SRCCOLORKEY, colorkey);
    shot_b.img = temp;
    shot_b.drawlocx  = -4.0F;
    shot_b.drawlocy  = -4.0F;
    shot_b.rad       = 4.0F;
    shot_b.tlx       = -4.0F;
    shot_b.tly       = -4.0F;
    shot_b.lrx       = 4.0F;
    shot_b.lry       = 4.0F;
    shot_b.flags     = 0;
    shot_b.gameflags = 0;
    
    last_clock_tick = clock_60hz();
    
    while (TRUE) {
        /* Check for events */
        if (SDL_PollEvent(&event)) {
            /* Give escape priority */
            if (event.type == SDL_KEYDOWN &&
                event.key.keysym.sym == SDLK_ESCAPE) {
                break;
            }
            else {
                /* Run through input system */
                update_input(event);
            }
        }
        
        /* Blank out the screen */
        SDL_FillRect(surface, NULL, bg);
        
        /* Update all the pbullets */
        for (i = 0; i < 1024; ++i) {
            tmp = &pbullet_mem[i];
            if (pis_alive(tmp)) {
                update_pbullet(tmp);
            }
        }
        
        /* Update the ship */
        update_coreship(0, &ship);
        
        /* Draw all the pbullets (including new ones) */
        for (i = 0; i < 1024; ++i) {
            tmp = &pbullet_mem[i];
            if (pis_alive(tmp)) {
                draw_pbullet(tmp, surface, 320, 240);
            }
        }
        
        /* Draw the ship */
        draw_player(&ship, surface, 320, 240);
        
        /* Now handle enemy stuff */
        /* Update all the timers */
        --next_enemy_timer;
        if (next_enemy_timer < 0) next_enemy_timer = ENEMY_TIMER;
        --next_shot_a_timer;
        if (next_shot_a_timer < 0) next_shot_a_timer = SHOT_A_TIMER;
        --next_shot_b_timer;
        if (next_shot_b_timer < 0) next_shot_b_timer = SHOT_B_TIMER;
        
        player_died = FALSE;
        /* Update all the bullets */
        for (i = 0; i < 8192; ++i) {
            tmpb = &bullet_mem[i];
            if (is_alive(tmpb)) {
                process_bullet(tmpb);
                if (is_enemy(tmpb)) {
                    /* Check if it's colliding with a pbullet */
                    for (j = 0; j < 1024; ++j) {
                        tmp = &pbullet_mem[j];
                        if (pis_alive(tmp)) {
                            if (collide_pbullet(tmp, tmpb)) {
                                destroy_pbullet(tmp);
                                destroy_bullet(tmpb);
                                break;
                            }
                        }
                    }
                    /* We need to check if we're still alive now */
                    /* Check if we're supposed to fire */
                    if (is_alive(tmpb) && tmpb->velx > 0 && next_shot_a_timer == 0) {
                        /* Shooting off a bullet in all 8 directions */
                        make_bullet(tmpb->centerx, tmpb->centery + 8.0F,
                                     1.5F,  0.0F, &shot_a);
                        make_bullet(tmpb->centerx, tmpb->centery + 8.0F,
                                    -1.5F,  0.0F, &shot_a);
                        make_bullet(tmpb->centerx, tmpb->centery + 8.0F,
                                     0.0F,  1.5F, &shot_a);
                        make_bullet(tmpb->centerx, tmpb->centery + 8.0F,
                                     0.0F, -1.5F, &shot_a);
                                    
                        make_bullet(tmpb->centerx, tmpb->centery + 8.0F,
                                     1.064F,  1.064F, &shot_a);
                        make_bullet(tmpb->centerx, tmpb->centery + 8.0F,
                                    -1.064F,  1.064F, &shot_a);
                        make_bullet(tmpb->centerx, tmpb->centery + 8.0F,
                                     1.064F, -1.064F, &shot_a);
                        make_bullet(tmpb->centerx, tmpb->centery + 8.0F,
                                    -1.064F, -1.064F, &shot_a);
                    }
                    if (is_alive(tmpb) && tmpb->velx < 0 && next_shot_b_timer == 0) {
                        /* Shooting off 6 bullets in random directions */
                        for (j = 0; j < 6; ++j) {
                            /* Gives a random angle in the valid range */
                            dir = (rand()%92160)/256.0F;
                            polar_to_rect(1.0F, dir, &xvel, &yvel);
                            make_bullet(tmpb->centerx, tmpb->centery + 8.0F,
                                        xvel, yvel, &shot_b);
                        }
                    }
                }
                
                /* Did we hit the player? */
                if (!player_died && is_alive(tmpb) &&
                    collide_bullet(tmpb, ship.centerx, ship.centery, ship.rad)) {
                    
                    player_died = TRUE;
                    ++deaths;
                    ship.centerx = 0.0F;
                    ship.centery = 0.0F;
                    /* Kill the bullet so we don't respawn on it */
                    set_alive(tmpb,FALSE);
                }
                
                /* Draw */
                if (is_alive(tmpb)) {
                    draw_bullet(tmpb, surface, 320, 240);
                }
            }
        }
        
        /* Check if we need to make more enemies */
        if (next_enemy_timer == 0) {
            make_bullet(-360.0F, -120.0F, 1.25F, 0, &enemy);
            make_bullet(360.0F, -180.0F, -1.25F, 0, &enemy);
        }
        
        /* Display death counter */
        sprintf(deathstring, "Deaths: %d", deaths);
        temp = TTF_RenderText_Solid(font, deathstring, off);
        SDL_BlitSurface(temp, NULL, surface, NULL);
        
        /* Flip the screen */
        SDL_Flip(surface);
        
        /* Wait for next clock tick */
        while (last_clock_tick == clock_60hz()) {
            SDL_Delay(1);
        }
        last_clock_tick = clock_60hz();
    }
}

#endif /* def SYSTEM_TEST */
