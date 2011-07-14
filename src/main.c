/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * main.c
 * Includes the main() function, which starts the engine and
 * game loop.
 * Also includes an alternative main() that allows for testing
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
    "Geometry test",
    "String hashing test",
    "Resource loader test",
    "Quit the system test"
};

#define rectset(rect,nx,ny,nw,nh) rect.x=nx;rect.y=ny;rect.w=nw;rect.h=nh

brmenu *construct_menu(SDL_Surface *surface, TTF_Font *font, resource *logo)
{
    brmenu *brm;
    
    int i, menu_x, menu_y, menu_width, menu_height;
    
    const SDL_Color off = {255, 255, 255}; /* white   */
    const SDL_Color on  = {255,   0,   0}; /* red     */
    
    Uint32 bg = SDL_MapRGB(surface->format, 0, 0, 32); /* dk.blue */
    
    char versionstring[64];
    
    SDL_Surface *back;
    SDL_Surface *logoimg;
    SDL_Surface *version;
    SDL_Surface *offs[5];
    SDL_Surface *ons [5];
    
    SDL_Rect rect, offsrc, offdst, onsrc, ondst;
    
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
    
    return brm;
}

/* 
 * This redraws the menu approximately 60 times a second
 * It's VERY rough, and would obviously only be used for something
 * simple like this
 */
int _drawthread(void *data)
{
    brmenu *brm = (brmenu*)data;
    int r;
    
    while (brm->running) {
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

int main(int nargs, char *args[])
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
    
    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
    panic(screen != NULL, "Error setting up video mode");
    SDL_WM_SetCaption("BULLET RAIN ENGINE TEST", NULL);
    
    logos = load_arc("res/logo.tgz");
    corner_logo = get_res("res/logo.tgz", "logosm.png");
    
    brm = construct_menu(screen, font, corner_logo);

    while (!finished) {
        start_menu(brm);
        drawthread = SDL_CreateThread(&_drawthread, brm);
        wait_on_menu(brm);
        
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
    
    SDL_WaitThread(drawthread, NULL);
    
    destroy_menu(brm);
    TTF_CloseFont(font);
    SDL_FreeSurface(screen);
    
    panic(!stop_all(), "Error stopping engine subsystems!");
    
    return 0;
}

inline void print_fixed(fixed_t a)
{
    printf("%d and %d/65536\n", intpart(a), fracpart(a));
}

#define FIXED_TESTS 20
void fixed_test(SDL_Surface *surface, TTF_Font *font)
{
    fixed_t a,b;
    int i;

    for (i=0; i<FIXED_TESTS; ++i) {
        /* generate random a,b */
        a = tofixed(rand()%1024-512,rand()%65536);
        b = tofixed(rand()%1024-512,rand()%65536);

        printf("  A: ");
        print_fixed(a);
        printf("  B: ");
        print_fixed(b);
        printf("A+B: ");
        print_fixed(a+b);
        printf("A-B: ");
        print_fixed(a-b);
        printf("A*B: ");
        print_fixed(fixmul(a,b));
        printf("A/B: ");
        print_fixed(fixdiv(a,b));
        puts("");
    }
}

#define DIAMETER 760
#define WIDTH 800
void geom_test(SDL_Surface *surface, TTF_Font *font)
{
    char img[WIDTH][WIDTH];
    int i,j,x,y;
    double mx, my, dangle;
    angle_t currang;
    polar_point p;
    rect_point r;
    FILE *out;
    
    const int center = WIDTH/2;
    const fixed_t radius = tofixed(DIAMETER/2,0);
    const double dradius = intpart(radius) + (fracpart(radius)/65536.0);
    const angle_t step = makeangle(0,4096); /* so 4096 steps */
    
    puts("Drawing a big circle and outputting it to geometry.txt");
    
    debug("Filling memory buffer with spaces");
    for (i = 0; i < WIDTH; ++i) {
        for (j = 0; j < WIDTH; ++j) {
            img[i][j] = ' ';
        }
    }
    
    debug("Beginning to draw circle");
    p.r = radius;
    for (currang = zeroangle; currang < maxangle; currang += step) {
        p.t = currang;
        r = polar_to_rect(p);
        x = intpart(r.x + tofixed(center,0));
        y = intpart(r.y + tofixed(center,0));
        img[y][x] = '#';
        
        dangle = intpart(currang) + (fracpart(currang)/65536.0);
        mx = dradius * cos(dangle);
        my = dradius * sin(dangle);
        x = (int)(mx + center);
        y = (int)(my + center);
        img[y][x] = 'C';
    }
    
    out = fopen("geometry.txt", "w");
    for (i = 0; i < WIDTH; i++) {
        for (j = 0; j < WIDTH; j++) {
            fputc(img[i][j], out);
        }
        fputc('\n', out);
    }
    fclose(out);
}

void hash_test(SDL_Surface *surface, TTF_Font *font)
{
    char a[64];
    sid_t hash;
    
    while (1) {
        puts("Enter a string to hash (blank line quits):");
        fgets(a, 64, stdin);
        clip_string(a);
        if (a[0] == '\0') {
            puts("Okay, exiting hash test.");
            break;
        }
        
        hash = calculate_sid(a);
        
        /* 
         * note: this may cause warnings if Uint32 is not vanilla int
         * but it won't hurt anything, god willing
         */
        printf("SID of '%s' is %08x\n", a, (Uint32)hash);
    }
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

#else /* def SYSTEM_TEST */

void main(int nargs, char *args[])
{
    /* TODO: Should probably write something here? */
}

#endif /* def SYSTEM_TEST */
