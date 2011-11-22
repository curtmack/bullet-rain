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
#include "fixed.h"
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

void fixed_test(SDL_Surface *surface, TTF_Font *font);
void geom_test(SDL_Surface *surface, TTF_Font *font);
void hash_test(SDL_Surface *surface, TTF_Font *font);
void res_test(SDL_Surface *surface, TTF_Font *font);
void timer_test(SDL_Surface *surface, TTF_Font *font);
void input_test(SDL_Surface *surface, TTF_Font *font);
void bull_test_fake_proc(SDL_Surface *surface, TTF_Font *font);
void bull_test_collision(SDL_Surface *surface, TTF_Font *font);
void player_test(SDL_Surface *surface, TTF_Font *font);

#define TEST_MENU_SIZE 10

#define TEST_FIXED     0
#define TEST_GEOM      1
#define TEST_HASH      2
#define TEST_RES       3
#define TEST_TIMER     4
#define TEST_INPUT     5
#define TEST_FAKEBULL  6
#define TEST_COLLISION 7
#define TEST_PLAYER    8
#define TEST_QUIT      9

const char menu[TEST_MENU_SIZE][32] = {
    "Fixed-point arithmetic test",
    "Trigonometry test",
    "String hashing test",
    "Resource loader test",
    "60 hz timer test",
    "Input test",
    "Bullet test - fake pipeline",
    "Bullet test - collision",
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
    fixed_t x = fixzero, y = fixzero;
    
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

#define INPUT_TEST_MOVESPEED tofixed(0,8192)
    
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
        rect.x = intpart(x) + 320;
        rect.y = intpart(y) + 240;
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
    
    rect_point vel, pt;
    
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
    
    /* Get the resources we need */
    smsprite = (SDL_Surface*)(get_res("res/brcore.tgz", "smbullet.png")->data);
    lgsprite = (SDL_Surface*)(get_res("res/brcore.tgz", "lgbullet.png")->data);
    
    /* Make the bullet types */
    for (i = 0; i < 12; ++i) {
        sm[i].rad       = tofixed(4, 0);
        sm[i].flags     = 0;
        sm[i].gameflags = 0;
        sm[i].lua       = NULL;
        sm[i].tl.x      = tofixed(-4,0);
        sm[i].tl.y      = tofixed(-4,0);
        sm[i].lr.x      = tofixed(4,0);
        sm[i].lr.y      = tofixed(4,0);
        sm[i].drawloc.x = tofixed(-4,0);
        sm[i].drawloc.y = tofixed(-4,0);
        
        lg[i].rad       = tofixed(12, 0);
        lg[i].flags     = 0;
        lg[i].gameflags = 0;
        lg[i].lua       = NULL;
        lg[i].tl.x      = tofixed(-12,0);
        lg[i].tl.y      = tofixed(-12,0);
        lg[i].lr.x      = tofixed(12,0);
        lg[i].lr.y      = tofixed(12,0);
        lg[i].drawloc.x = tofixed(-16,0);
        lg[i].drawloc.y = tofixed(-16,0);
        
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
                pt.x  = tofixed(rand()%640-320, rand()%65536-32768);
                pt.y  = tofixed(rand()%480-240, rand()%65536-32768);
                vel.x = tofixed(rand()%4 - 2,   rand()%65536-32768);
                vel.y = tofixed(rand()%4 - 2,   rand()%65536-32768);
                if (rand()%2) {
                    if (make_bullet(pt, vel, &sm[rand()%12]) != NULL) {
                        ++numbullets;
                        ++bullets_made;
                    }
                }
                else {
                    if (make_bullet(pt, vel, &lg[rand()%12]) != NULL) {
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
    
    rect_point vel, pt;
    
    int i, j, mouse_x, mouse_y;
    
    SDL_Rect rect;
    SDL_Surface *smsprite, *lgsprite, *stmp;
    SDL_Event event;
    
    const SDL_PixelFormat fmt = *(surface->format);
    const int center_x = 320;
    const int center_y = 240;
    const Uint32 bg = SDL_MapRGB(&fmt, 0, 0, 32); /* dk.blue */
    const Uint32 colorkey = SDL_MapRGBA(surface->format, 255, 0, 255, SDL_ALPHA_OPAQUE);
    
    /* Get the resources we need */
    smsprite = (SDL_Surface*)(get_res("res/brcore.tgz", "smbullet.png")->data);
    lgsprite = (SDL_Surface*)(get_res("res/brcore.tgz", "lgbullet.png")->data);
    
    /* Make the bullet types */
    
    miss[0].rad       = tofixed(4, 0);
    miss[0].flags     = 0;
    miss[0].gameflags = 0;
    miss[0].lua       = NULL;
    miss[0].tl.x      = tofixed(-4,0);
    miss[0].tl.y      = tofixed(-4,0);
    miss[0].lr.x      = tofixed(4,0);
    miss[0].lr.y      = tofixed(4,0);
    miss[0].drawloc.x = tofixed(-4,0);
    miss[0].drawloc.y = tofixed(-4,0);
    
    hit[0].rad       = tofixed(4, 0);
    hit[0].flags     = 0;
    hit[0].gameflags = 0;
    hit[0].lua       = NULL;
    hit[0].tl.x      = tofixed(-4,0);
    hit[0].tl.y      = tofixed(-4,0);
    hit[0].lr.x      = tofixed(4,0);
    hit[0].lr.y      = tofixed(4,0);
    hit[0].drawloc.x = tofixed(-4,0);
    hit[0].drawloc.y = tofixed(-4,0);
    
    miss[1].rad       = tofixed(12, 0);
    miss[1].flags     = 0;
    miss[1].gameflags = 1; /* we're cheating here, this means it's big */
    miss[1].lua       = NULL;
    miss[1].tl.x      = tofixed(-12,0);
    miss[1].tl.y      = tofixed(-12,0);
    miss[1].lr.x      = tofixed(12,0);
    miss[1].lr.y      = tofixed(12,0);
    miss[1].drawloc.x = tofixed(-16,0);
    miss[1].drawloc.y = tofixed(-16,0);
    
    hit[1].rad       = tofixed(12, 0);
    hit[1].flags     = 0;
    hit[1].gameflags = 1;
    hit[1].lua       = NULL;
    hit[1].tl.x      = tofixed(-12,0);
    hit[1].tl.y      = tofixed(-12,0);
    hit[1].lr.x      = tofixed(12,0);
    hit[1].lr.y      = tofixed(12,0);
    hit[1].drawloc.x = tofixed(-16,0);
    hit[1].drawloc.y = tofixed(-16,0);
        
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
            pt.x = fixmul(fixdiv(tofixed(640,0),tofixed(17,0)),tofixed(j+1,0));
            pt.x -= tofixed(center_x,0);
            pt.y = fixmul(fixdiv(tofixed(480,0),tofixed(13,0)),tofixed(i+1,0));
            pt.y -= tofixed(center_y,0);
            vel.x = fixzero;
            vel.y = fixzero;
            
            if ((i+j) % 2 == 1) {
                make_bullet(pt, vel, &miss[0]);
            }
            else {
                make_bullet(pt, vel, &miss[1]);
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
        pt.x = tofixed(mouse_x, 0);
        pt.y = tofixed(mouse_y, 0);
        
        /* We may as well process all the bullets, most of them are gone */
        for (i = 0; i < 8192; ++i) {
            tmp = &bullet_mem[i];
            if (is_alive(tmp)) {
                process_bullet(tmp);
                if(collide_bullet(tmp, pt, fixzero)) {
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
    pbullet *tmp;
    int i;
    
    const Uint32 bg = SDL_MapRGB(surface->format, 0, 0, 32); /* dk.blue */
    
    ship = make_coreship();
    setup_coreship(0);
    ship.center.x = fixzero;
    ship.center.y = fixzero;
    
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
                draw_pbullet(*tmp, surface, 320, 240);
            }
        }
        
        /* Draw the ship */
        draw_player(ship, surface, 320, 240);
        
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
