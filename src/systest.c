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

#include "debug.h"
#include "fixed.h"
#include "geometry.h"
#include "init.h"
#include "menu.h"
#include "resource.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"
#include "SDL/SDL_ttf.h"
#else
#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_ttf.h"
#endif

void fixed_test(SDL_Surface *surface, TTF_Font *font);
void geom_test(SDL_Surface *surface, TTF_Font *font);
void hash_test(SDL_Surface *surface, TTF_Font *font);
void res_test(SDL_Surface *surface, TTF_Font *font);

#define TEST_MENU_SIZE 5

#define TEST_FIXED 0
#define TEST_GEOM  1
#define TEST_HASH  2
#define TEST_RES   3
#define TEST_QUIT  4

const char menu[TEST_MENU_SIZE][32] = {
    "Fixed-point arithmetic test",
    "Trigonometry test",
    "String hashing test",
    "Resource loader test",
    "Quit the system test"
};
    
const SDL_Color off = {255, 255, 255}; /* white   */
const SDL_Color on  = {255,   0,   0}; /* red     */

#define rectset(rect,nx,ny,nw,nh) rect.x=nx;rect.y=ny;rect.w=nw;rect.h=nh

brmenu *construct_menu(SDL_Surface *surface, TTF_Font *font, resource *logo)
{
    brmenu *brm;
    
    int i, menu_x, menu_y, menu_width, menu_height;
    
    char versionstring[64];
    
    SDL_Surface *back;
    SDL_Surface *logoimg;
    SDL_Surface *version;
    SDL_Surface *offs[5];
    SDL_Surface *ons [5];
    
    SDL_Rect rect, offsrc, offdst, onsrc, ondst;
    const Uint32 bg = SDL_MapRGB(surface->format, 0, 0, 32); /* dk.blue */
    
    offs[0] = TTF_RenderText_Solid(font, menu[0], off);
    ons [0] = TTF_RenderText_Solid(font, menu[0], on );
    offs[1] = TTF_RenderText_Solid(font, menu[1], off);
    ons [1] = TTF_RenderText_Solid(font, menu[1], on );
    offs[2] = TTF_RenderText_Solid(font, menu[2], off);
    ons [2] = TTF_RenderText_Solid(font, menu[2], on );
    offs[3] = TTF_RenderText_Solid(font, menu[3], off);
    ons [3] = TTF_RenderText_Solid(font, menu[3], on );
    offs[4] = TTF_RenderText_Solid(font, menu[4], off);
    ons [4] = TTF_RenderText_Solid(font, menu[4], on );
    
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
    rectset(offsrc, 0, 0, offs[0]->w, offs[0]->h);
    rectset(offdst, menu_x, menu_y, 0, 0 );
    rectset(onsrc, 0, 0, ons[0]->w, ons[0]->h );
    rectset(ondst, menu_x, menu_y, 0, 0 );
    menu_add_entry(brm, TEST_FIXED, TEST_QUIT, TEST_GEOM, -1, -1,
                   offs[0], offsrc, offdst, ons[0], onsrc, ondst);
    menu_y += offs[0]->h;

    rectset(offsrc, 0, 0, offs[1]->w, offs[1]->h);
    rectset(offdst, menu_x, menu_y, 0, 0 );
    rectset(onsrc, 0, 0, ons[1]->w, ons[1]->h );
    rectset(ondst, menu_x, menu_y, 0, 0 );
    menu_add_entry(brm, TEST_GEOM, TEST_FIXED, TEST_HASH, -1, -1,
                   offs[1], offsrc, offdst, ons[1], onsrc, ondst);
    menu_y += offs[1]->h;
    
    rectset(offsrc, 0, 0, offs[2]->w, offs[2]->h);
    rectset(offdst, menu_x, menu_y, 0, 0 );
    rectset(onsrc, 0, 0, ons[2]->w, ons[2]->h );
    rectset(ondst, menu_x, menu_y, 0, 0 );
    menu_add_entry(brm, TEST_HASH, TEST_GEOM, TEST_RES, -1, -1,
                   offs[2], offsrc, offdst, ons[2], onsrc, ondst);
    menu_y += offs[2]->h;
    
    rectset(offsrc, 0, 0, offs[3]->w, offs[3]->h);
    rectset(offdst, menu_x, menu_y, 0, 0 );
    rectset(onsrc, 0, 0, ons[3]->w, ons[3]->h );
    rectset(ondst, menu_x, menu_y, 0, 0 );
    menu_add_entry(brm, TEST_RES, TEST_HASH, TEST_QUIT, -1, -1,
                   offs[3], offsrc, offdst, ons[3], onsrc, ondst);
    menu_y += offs[3]->h;
    
    rectset(offsrc, 0, 0, offs[4]->w, offs[4]->h);
    rectset(offdst, menu_x, menu_y, 0, 0 );
    rectset(onsrc, 0, 0, ons[4]->w, ons[4]->h );
    rectset(ondst, menu_x, menu_y, 0, 0 );
    menu_add_entry(brm, TEST_QUIT, TEST_RES, TEST_FIXED, -1, -1,
                   offs[4], offsrc, offdst, ons[4], onsrc, ondst);
    
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
    arclist *logos;
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
    
    logos = load_arc("res/brcore.tgz");
    corner_logo = get_res("res/brcore.tgz", "logosm.png");
    
    brm = construct_menu(screen, font, corner_logo);

    while (!finished) {
        start_menu(brm);
        draw_stop = FALSE;
        drawthread = SDL_CreateThread(&_drawthread, brm);
        while (brm->running == TRUE) {
            menu_action(brm, get_action());
            SDL_Delay(10);
        }
        
        /* I'm sorry, it's just not working out. Friends? */
        draw_stop = TRUE;
        
        switch (brm->end) {
            case TEST_FIXED:
                fixed_test(screen, font);
                break;
            case TEST_GEOM:
                geom_test(screen, font);
                break;
            case TEST_HASH:
                hash_test(screen, font);
                break;
            case TEST_RES:
                res_test(screen, font);
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
    
    panic(!stop_all(), "Error stopping engine subsystems!");
    
    return 0;
}

void fixed_test(SDL_Surface *surface, TTF_Font *font)
{
    int values[4] = {0, 0, 0, 0};
    fixed_t a = fixzero, b = fixzero, result = fixzero;
    
    enum {
        FIXED_TEST_AI,
        FIXED_TEST_AF,
        FIXED_TEST_BI,
        FIXED_TEST_BF,
        FIXED_TEST_OP
    } selected = FIXED_TEST_AI;
    
    enum {
        FIXED_TEST_ADD,
        FIXED_TEST_SUB,
        FIXED_TEST_MUL,
        FIXED_TEST_DIV
    } op = FIXED_TEST_ADD;
    
    char buffer[32];
    char numbuf[8] = "\0\0\0\0\0\0\0\0";
    char entry[8] = "0\0\0\0\0\0\0\0";
    int cursor = 1;
    int len    = 1;
    int i;
    int update = TRUE;
    int divzero = FALSE;
    SDL_Surface *back, *tmp;
    SDL_Rect rect;
    SDL_Event event;
    
    const Uint32 bg = SDL_MapRGB(surface->format, 0, 0, 32); /* dk.blue */
    
    /* Start setting up background */
    back = SDL_CreateRGBSurface(SDL_SWSURFACE, surface->w, surface->h,
                                surface->format->BitsPerPixel,
                                surface->format->Rmask, surface->format->Gmask,
                                surface->format->Bmask, surface->format->Amask);
    
    rectset(rect,0,0,surface->w,surface->h);
    SDL_FillRect(back,&rect,bg);
    
    /* Draw in instructions */
    tmp = TTF_RenderText_Solid(font, "Up/Down: Select number", off);
    rectset(rect,0,back->h-(4*tmp->h),0,0);
    SDL_BlitSurface(tmp,NULL,back,&rect);
    SDL_FreeSurface(tmp);
    
    tmp = TTF_RenderText_Solid(font, "Type selected number with keyboard", off);
    rectset(rect,0,back->h-(3*tmp->h),0,0);
    SDL_BlitSurface(tmp,NULL,back,&rect);
    SDL_FreeSurface(tmp);
    
    tmp = TTF_RenderText_Solid(font, "Choose operation with left/right", off);
    rectset(rect,0,back->h-(2*tmp->h),0,0);
    SDL_BlitSurface(tmp,NULL,back,&rect);
    SDL_FreeSurface(tmp);
    
    tmp = TTF_RenderText_Solid(font, "Escape: Exit to menu", off);
    rectset(rect,0,back->h-tmp->h,0,0);
    SDL_BlitSurface(tmp,NULL,back,&rect);
    SDL_FreeSurface(tmp);
    
    /*
     * This isn't a menu because it would be difficult to get a brmenu
     * to do what we need it to here
     */
    SDL_EnableUNICODE(1);
    while (TRUE) {
        /* Draw everything */
        rectset(rect, 0, 0, 0, 0);
        SDL_BlitSurface(back, NULL, surface, &rect);
        
        for (i = 0; i < 4; ++i) {
            if (i == selected) {
                /* Draw the cursor */
                tmp = TTF_RenderText_Solid(font, "0", off);
                rectset(rect,(tmp->w)*(cursor+4),i*tmp->h,tmp->w,tmp->h);
                SDL_FillRect(surface, &rect,
                        SDL_MapRGB(surface->format, 255, 0, 0)); /* red */
                
                /* Now draw the text */
                switch (i) {
                    case 0:
                        sprintf(buffer, "ai: %s", entry);
                        break;
                    case 1:
                        sprintf(buffer, "af: %s", entry);
                        break;
                    case 2:
                        sprintf(buffer, "bi: %s", entry);
                        break;
                    case 3:
                        sprintf(buffer, "bf: %s", entry);
                        break;
                }
                SDL_FreeSurface(tmp);
                tmp = TTF_RenderText_Solid(font, buffer, off);
                rectset(rect, 0, i*tmp->h, 0, 0);
                SDL_BlitSurface(tmp, NULL, surface, &rect);
                SDL_FreeSurface(tmp);
            }
            else {
                /* Just draw it */
                itoa(values[i], numbuf, 10);
                switch (i) {
                    case 0:
                        sprintf(buffer, "ai: %s", numbuf);
                        break;
                    case 1:
                        sprintf(buffer, "af: %s", numbuf);
                        break;
                    case 2:
                        sprintf(buffer, "bi: %s", numbuf);
                        break;
                    case 3:
                        sprintf(buffer, "bf: %s", numbuf);
                        break;
                }
                tmp = TTF_RenderText_Solid(font, buffer, off);
                rectset(rect, 0, i*tmp->h, 0, 0);
                SDL_BlitSurface(tmp, NULL, surface, &rect);
                SDL_FreeSurface(tmp);
            }
        }
        
        /* Draw the op */
        if (selected == FIXED_TEST_OP) {
            /* Draw a selection rectangle */
            tmp = TTF_RenderText_Solid(font, "0", off);
            rectset(rect,(tmp->w)*4,4*tmp->h,(tmp->w)*3,tmp->h);
            SDL_FillRect(surface, &rect,
                        SDL_MapRGB(surface->format, 255, 0, 0)); /* red */
            SDL_FreeSurface(tmp);
        }
        switch (op) {
            case FIXED_TEST_ADD:
                tmp = TTF_RenderText_Solid(font, "op:  +", off);
                break;
            case FIXED_TEST_SUB:
                tmp = TTF_RenderText_Solid(font, "op:  -", off);
                break;
            case FIXED_TEST_MUL:
                tmp = TTF_RenderText_Solid(font, "op:  *", off);
                break;
            case FIXED_TEST_DIV:
                tmp = TTF_RenderText_Solid(font, "op:  /", off);
                break;
        }
        rectset(rect,0,4*tmp->h,0,0);
        SDL_BlitSurface(tmp, NULL, surface, &rect);
        SDL_FreeSurface(tmp);
        
        /* Draw the result */
        sprintf(buffer, "result: (%d + %d/65536)", intpart(result), fracpart(result));
        tmp = TTF_RenderText_Solid(font, buffer, off);
        rectset(rect,surface->w - tmp->w, 0, 0, 0);
        SDL_BlitSurface(tmp, NULL, surface, &rect);
        SDL_FreeSurface(tmp);
        
        /* Draw division by zero warning */
        if (divzero) {
            tmp = TTF_RenderText_Solid(font, "Division by zero", on);
            rectset(rect,surface->w - tmp->w, tmp->h, 0, 0);
            SDL_BlitSurface(tmp, NULL, surface, &rect);
            SDL_FreeSurface(tmp);
        }
        
        /* Update the screen */
        SDL_Flip(surface);
        
        SDL_WaitEvent(&event);
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                break;
            }
            /* It's just simpler to check this first */
            if (selected == FIXED_TEST_OP) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        selected = FIXED_TEST_BF;
                        /* Set up the new selection */
                        itoa(values[selected], entry, 10);
                        len = strlen(entry);
                        if (len > 5) {
                            entry[5] = '\0';
                            len = 5;
                            cursor = 5;
                        } 
                        else {
                            cursor = len;
                        }
                        update = TRUE;
                        break;
                    case SDLK_DOWN:
                        selected = FIXED_TEST_AI;
                        itoa(values[selected], entry, 10);
                        len = strlen(entry);
                        if (len > 5) {
                            entry[5] = '\0';
                            len = 5;
                            cursor = 5;
                        } 
                        else {
                            cursor = len;
                        }
                        update = TRUE;
                        break;
                    case SDLK_LEFT:
                        if (op == FIXED_TEST_ADD) {
                            op = FIXED_TEST_DIV;
                        }
                        else {
                            --op;
                        }
                        break;
                    case SDLK_RIGHT:
                        if (op == FIXED_TEST_DIV) {
                            op = FIXED_TEST_ADD;
                        }
                        else {
                            ++op;
                        }
                        break;
                    case SDLK_RETURN:
                        /* Update without moving */
                        update = TRUE;
                        break;
                    default:
                        /* do nothing */
                        break;
                }
            }
            else {
                /* We're in typing mode */
                switch(event.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_DOWN:
                        /* 
                         * These are mostly the same, set the new value
                         * and then change selected
                         * Up or down only matters at the end
                         */
                        /* Set new value */
                        values[selected] = atoi(entry);
                        
                        /* Clear out entry */
                        for (i = 0; i < 8; ++i) {
                            entry[i] = '\0';
                        }
                        
                        /* Go to next entry */
                        if (event.key.keysym.sym == SDLK_UP) {
                            if (selected == FIXED_TEST_AI) {
                                selected = FIXED_TEST_OP;
                            }
                            else --selected;
                        }
                        else {
                            /* If we had op selected, we wouldn't be here */
                            ++selected;
                        }
                        
                        /* Are we on an input row? */
                        if (selected != FIXED_TEST_OP) {
                            /* Set up the new selection */
                            itoa(values[selected], entry, 10);
                            len = strlen(entry);
                            if (len > 5) {
                                entry[5] = '\0';
                                len = 5;
                                cursor = 5;
                            } 
                            else {
                                cursor = len;
                            }
                        }
                        /* We're ready now */
                        update = TRUE;
                        break;
                        
                    case SDLK_RETURN:
                        /* Update without moving */
                        update = TRUE;                  
                        break;
                        
                    case SDLK_LEFT:
                        if (cursor > 0) {
                            --cursor;
                        }
                        break;
                    case SDLK_RIGHT:
                        if (cursor < len && cursor < 4) {
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
                    default:
                        /* 
                         * Does it have a unicode component?
                         * We need to check for this last because some
                         * of the above characters have a unicode point
                         * (e.g. '\b' for SDLK_BACKSPACE)
                         */
                        if (event.key.keysym.unicode != 0) {
                            /* Only allow digits */
                            if ((char)event.key.keysym.unicode >= '0' &&
                                (char)event.key.keysym.unicode <= '9') {
                                entry[cursor] = (char)event.key.keysym.unicode;
                                if (cursor < 5) ++cursor;
                                if (cursor > len) ++len;
                                entry[len] = '\0';
                            }
                        }
                        break;
                }
            }
        }
            
        /* Do we need to update the math? */
        if (update) {
            a = tofixed(values[0], values[1]);
            b = tofixed(values[2], values[3]);
            switch (op) {
                case FIXED_TEST_ADD:
                    result = a+b;
                    divzero = FALSE;
                    break;
                case FIXED_TEST_SUB:
                    result = a-b;
                    divzero = FALSE;
                    break;
                case FIXED_TEST_MUL:
                    result = fixmul(a,b);
                    divzero = FALSE;
                    break;
                case FIXED_TEST_DIV:
                    result = fixdiv(a,b);
                    if (b == fixzero) {
                        divzero = TRUE;
                    }
                    else {
                        divzero = FALSE;
                    }
                    break;
            }
            /* don't need to update again */
            update = FALSE;
        }
        
        /* Wait a bit */
        SDL_Delay(20);
        
        /* And continue looping */
    }
    /* Disable unicode conversion */
    SDL_EnableUNICODE(0);
}

void geom_test(SDL_Surface *surface, TTF_Font *font)
{
    angle_t ang;
    fixed_t res;
    double angd, resd, persec;
    char buffer[40];
    Uint32 starttime, geomtime, libctime;
    int i, x, y, r;
    SDL_Surface *back, *tmp;
    SDL_Rect rect;
    SDL_Event event;
    
    const Uint32 bg = SDL_MapRGB(surface->format, 0, 0, 32); /* dk.blue */
    const Uint32 axes = SDL_MapRGB(surface->format, 255, 255, 255); /* white */
    const Uint32 graph = SDL_MapRGB(surface->format, 255, 0, 0); /* red */
    
    const angle_t step = makeangle(0, 4096);
    
    /* Why doesn't cmath have this? */
    const double pi = 3.141592653589793;
    
    /* Set up the background */
    back = SDL_CreateRGBSurface(SDL_SWSURFACE, surface->w, surface->h,
                                surface->format->BitsPerPixel,
                                surface->format->Rmask, surface->format->Gmask,
                                surface->format->Bmask, surface->format->Amask);
    
    rectset(rect,0,0,surface->w,surface->h);
    SDL_FillRect(back,&rect,bg);
    
    /* Draw in instructions */    
    tmp = TTF_RenderText_Solid(font, "Escape: Exit to menu", off);
    rectset(rect,0,back->h-tmp->h,0,0);
    SDL_BlitSurface(tmp,NULL,back,&rect);
    SDL_FreeSurface(tmp);
    
    /* Draw in labels */
    tmp = TTF_RenderText_Solid(font, "geometry.h", off);
    rectset(rect,0,0,0,0);
    SDL_BlitSurface(tmp,NULL,back,&rect);
    SDL_FreeSurface(tmp);
    
    tmp = TTF_RenderText_Solid(font, "cmath", off);
    rectset(rect,back->w-tmp->w,0,0,0);
    SDL_BlitSurface(tmp,NULL,back,&rect);
    SDL_FreeSurface(tmp);
    
    /* Working on the graphs now */
#define LEFT_GRAPH_START_X  10
#define LEFT_GRAPH_START_Y  50
#define RIGHT_GRAPH_START_X 330
#define RIGHT_GRAPH_START_Y 50
#define GRAPH_WIDTH   300
#define GRAPH_HEIGHT  300

    /* Draw axes */
    rectset(rect, LEFT_GRAPH_START_X, LEFT_GRAPH_START_Y, 1, GRAPH_HEIGHT);
    SDL_FillRect(back, &rect, axes);
    rectset(rect, LEFT_GRAPH_START_X, LEFT_GRAPH_START_Y + GRAPH_HEIGHT/2,
            GRAPH_WIDTH, 1);
    SDL_FillRect(back, &rect, axes);
    
    rectset(rect, RIGHT_GRAPH_START_X, RIGHT_GRAPH_START_Y, 1, GRAPH_HEIGHT);
    SDL_FillRect(back, &rect, axes);
    rectset(rect, RIGHT_GRAPH_START_X, RIGHT_GRAPH_START_Y + GRAPH_HEIGHT/2,
            GRAPH_WIDTH, 1);
    SDL_FillRect(back, &rect, axes);
    
    /* Draw the graphs themselves */
    for (ang = zeroangle; ang < maxangle; ang += step) {
        res = lookup_sin(ang);
        x = intpart(
            fixmul((fixed_t)ang,
            fixdiv(tofixed(GRAPH_WIDTH,0),(fixed_t)maxangle))
            ) + LEFT_GRAPH_START_X;
        y = intpart(
            fixmul((fixed_t)res, tofixed(GRAPH_HEIGHT/2,0))
            ) + LEFT_GRAPH_START_Y + GRAPH_HEIGHT/2;
        /* No DrawPixel function, but a rect of size 1 is close enough */
        rectset(rect, x, y, 1, 1);
        SDL_FillRect(back, &rect, graph);
        
        /* 
         * When we're timing, this lengthy conversion calculation will be
         * unfair, so we'll generate double angles separately.
         * But here, we don't really care.
         */
        angd = (double)(intpart(ang) + (fracpart(ang)/65536.0));
        angd = angd * (2*pi) / 256.0;
        resd = sin(angd);
        x = angd * (GRAPH_WIDTH / (2*pi)) + RIGHT_GRAPH_START_X;
        y = resd * (GRAPH_HEIGHT/2) + RIGHT_GRAPH_START_Y + GRAPH_HEIGHT/2;
        rectset(rect,x,y,1,1);
        SDL_FillRect(back, &rect, graph);
    }
    
    /* Draw to the screen */
    rectset(rect, 0, 0, 0, 0);
    SDL_BlitSurface(back, NULL, surface, &rect);
    SDL_Flip(surface);

#define NUM_CALCULATIONS_INT    4000000
#define NUM_CALCULATIONS_DOUBLE 4000000.0
    
    while (1) {
        /* Start drawing */
        rectset(rect, 0, 0, 0, 0);
        SDL_BlitSurface(back, NULL, surface, &rect);
        
        /* Faster to count to a number than wait to a time */
        starttime = SDL_GetTicks();
        for (i = 0; i < NUM_CALCULATIONS_INT; ++i) {
            ang = makeangle(rand()%1024,rand()%65536);
            res = lookup_sin(ang);
        }
        geomtime = SDL_GetTicks() - starttime;
        
        /* 1000 milliseconds in a second */
        persec = (NUM_CALCULATIONS_DOUBLE/geomtime) * 1000.0;
        
        sprintf(buffer, "%g per second", persec);
        tmp = TTF_RenderText_Solid(font, buffer, off);
        rectset(rect,0,tmp->h,0,0);
        SDL_BlitSurface(tmp,NULL,surface,&rect);
        SDL_FreeSurface(tmp);
        
        /* Same timing process for libc's sin */
        starttime = SDL_GetTicks();
        for (i = 0; i < NUM_CALCULATIONS_INT; ++i) {
            /* This picks an angle in a reasonable range */
            angd = (double)rand();
            resd = sin(angd);
        }
        libctime = SDL_GetTicks() - starttime;
        
        persec = (NUM_CALCULATIONS_DOUBLE/libctime) * 1000.0;
        
        sprintf(buffer, "%g per second", persec);
        tmp = TTF_RenderText_Solid(font, buffer, off);
        rectset(rect,back->w-tmp->w,tmp->h,0,0);
        SDL_BlitSurface(tmp,NULL,surface,&rect);
        SDL_FreeSurface(tmp);
        
        /* Draw it */
        SDL_Flip(surface);
        
        /* Should we quit? */
        r = SDL_PollEvent(&event);
        if (r && event.type == SDL_KEYDOWN &&
            event.key.keysym.sym == SDLK_ESCAPE)
            break;
    }
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

void print_res(resource *res)
{
    /*
     * We're going to assume we'll never call this unless we know
     * it's actually printable data
     */
    char *a = (char*) res->data;
    Sint64 num = res->size;
    int i, r;
    
    puts("-------BEGIN-------");
    for (i = 0; i <= num; ++i) {
        r = (a[i] <= 127);
        warnn(r, "Character found that was not printable:", (int)a[i]);
        warnn(r, "It was at location:", i);
        if (r) putchar(a[i]);
    }
    puts("\n--------END--------");
}

void res_test(SDL_Surface *surface, TTF_Font *font)
{
    arclist *arc;
    resource *res;
    
    puts("Loading res/test.tgz");
    arc = load_arc("res/test.tgz");
    puts("Loading successful!");
    
    puts("Printing contents of file chocolat.txt");
    res = get_res("res/test.tgz", "chocolat.txt");
    print_res(res);
    
    puts("Freeing archive");
    free_arc("res/test.tgz");
    puts("Archive freed!");
    
    puts("Getting resource at pocky.txt in a way that should cause a warning");
    res = get_res("res/test.tgz", "pocky.txt");
    
    puts("Printing contents of file pocky.txt");
    print_res(res);
    
    puts("Test done!");
    puts("Leaving archive in memory.");
}

#endif /* def SYSTEM_TEST */
