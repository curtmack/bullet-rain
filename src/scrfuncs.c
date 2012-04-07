/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * scrfuncs.c
 * Functions which will be provided to Lua to provide scripting functionality.
 */

#include "bullet.h"
#include "compile.h"
#include "debug.h"
#include "geometry.h"
#include "scrfuncs.h"
#include "scripts.h"
#include "./lua/lua.h"
#include "./lua/lauxlib.h"

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#else
#include "SDL.h"
#endif

#define check_null_context(luastate) if (context == NULL) \
    luaL_error(luastate, "Call to 'self' function from stage context.")

#define MAX_TYPES 256

/* The bullet currently in context, used for self-acting functions */
bullet *context = NULL;

/* First load? Use this to detect if we need to zero out the types registry */
int first_load = TRUE;

/* Bullet type registry */
int valid_type[MAX_TYPES];
bullet_type types[MAX_TYPES];

/* The format used for bullet images */
SDL_PixelFormat fmt;

static int set_bullet_context(lua_State *L)
{
    int id;
    
    if (lua_type(L, 1) == LUA_TNIL) {
        context = NULL;
    }
    else {
        id = (int)luaL_checknumber(L, 1);
    
        if (id < 0 || id >= 8192) {
            luaL_error(L, "Bullet context %d not in valid range.", id);
        }
        
        context = &(bullet_mem[id]);
    }
    return 0;
}

static int is_bullet_dead(lua_State *L)
{
    int id;
    
    id = (int)luaL_checknumber(L, 1);
    
    if (id < 0 || id >= 8192) {
        luaL_error(L, "Bullet ID %d not in valid range.", id);
    }
    
    lua_pushboolean(L, !is_alive(&(bullet_mem[id])));
    return 1;
}

static int kill_bullets(lua_State *L)
{
    int i;
    bullet *b;
    for (i = 0; i < 8192; ++i) {
        b = &(bullet_mem[i]);
        if (is_killed(b)) {
            destroy_bullet(b);
        }
    }
    
    return 0;
}

static int register_type(lua_State *L)
{
    /* OH HOLY CRAP LOTS OF ARGUMENTS */
    float rad, tlx, tly, lrx, lry, drawlocx, drawlocy;
    int idx, flags, gameflags, gfxx, gfxy, gfxw, gfxh;
    char *arcname;
    char *resname;
    
    SDL_Surface *temp;
    SDL_Surface *img;
    SDL_Rect rect;
    
    /* Bring them all in, one by one... */
    idx       = luaL_checkinteger(L, 1);
    rad       = luaL_checknumber (L, 2);
    flags     = luaL_checkinteger(L, 3);
    gameflags = luaL_checkinteger(L, 4);
    tlx       = luaL_checknumber (L, 5);
    tly       = luaL_checknumber (L, 6);
    lrx       = luaL_checknumber (L, 7);
    lry       = luaL_checknumber (L, 8);
    drawlocx  = luaL_checknumber (L, 9);
    drawlocy  = luaL_checknumber (L, 10);
    arcname   = (char*) luaL_checkstring (L, 11); /* these return const */
    resname   = (char*) luaL_checkstring (L, 12); /* by default         */
    gfxx      = luaL_checkinteger(L, 13);
    gfxy      = luaL_checkinteger(L, 14);
    gfxw      = luaL_checkinteger(L, 15);
    gfxh      = luaL_checkinteger(L, 16);
    
    /* Check if we're in valid range */
    if (idx < 0 || idx >= MAX_TYPES) {
        luaL_error(L, "Attempt to register type index %d out of range", idx);
    }
    
    /* Set the values we can set */
    types[idx].rad       = rad;
    types[idx].flags     = flags;
    types[idx].gameflags = gameflags;
    types[idx].tlx       = tlx;
    types[idx].tly       = tly;
    types[idx].lrx       = lrx;
    types[idx].lry       = lry;
    types[idx].drawlocx  = drawlocx;
    types[idx].drawlocy  = drawlocy;
    
    /* Now we need to load the SDL_Surface */
    load_arc(arcname);
    temp = (get_res(arcname, resname))->data;
    
    /* Create surface for img and blit it over */
    img = SDL_CreateRGBSurface(SDL_HWSURFACE, gfxw, gfxh, fmt.BitsPerPixel,
            fmt.Rmask, fmt.Gmask, fmt.Bmask, fmt.Amask);
    rect.x = gfxx;
    rect.y = gfxy;
    rect.w = gfxw;
    rect.h = gfxh;
    SDL_BlitSurface(temp, &rect, img, NULL);
    
    types[idx].img = img;
    
    return 0;
}

static int unregister_type(lua_State *L)
{
    int idx;
    
    idx = luaL_checkinteger(L, 1);
    
    /* Set type to invalid */
    valid_type[idx] = FALSE;
    
    /* Free surface if necessary */
    if (types[idx].img != NULL) {
        SDL_FreeSurface(types[idx].img);
        types[idx].img = NULL;
    }
    
    return 0;
}

static int clear_types(lua_State *L)
{
    int i;
    
    for (i = 0; i < MAX_TYPES; ++i) {
        /* Set type to invalid */
        valid_type[i] = FALSE;
        
        /* Free surface if necessary */
        if (types[i].img != NULL) {
            SDL_FreeSurface(types[i].img);
            types[i].img = NULL;
        }
    }
    
    return 0;
}

static int fire_bullet(lua_State *L)
{
    /* TODO: stub - testing for compilation */
    return 0;
}

static int create_enemy(lua_State *L)
{
    /* TODO: stub - testing for compilation */
    return 0;
}

static int get_velocity_self(lua_State *L)
{
    check_null_context(L);
    
    lua_pushnumber(L, context->velx);
    lua_pushnumber(L, context->vely);
    
    return 2;
}

static int get_velocity_other(lua_State *L)
{
    int id;
    bullet *b;
    
    id = luaL_checkinteger(L, 1);
    
    if (id < 0 || id >= 8192) {
        luaL_error(L, "Access to bullet %d out of range", id);
    }
    
    b = &(bullet_mem[id]);
    
    lua_pushnumber(L, context->velx);
    lua_pushnumber(L, context->vely);
    
    return 2;
}

static int accelerate_self_by_scale(lua_State *L)
{
    double scale;
    
    check_null_context(L);
    
    scale = luaL_checknumber(L, 1);
    
    context->velx *= scale;
    context->vely *= scale;
    context->vel_mag *= scale;
    
    return 0;
}

static int accelerate_self_by_rect(lua_State *L)
{
    double ax, ay;
    
    check_null_context(L);
    
    ax = luaL_checknumber(L, 1);
    ay = luaL_checknumber(L, 2);
    
    context->velx += ax;
    context->vely += ay;
    set_pinvalid(context, TRUE);
    
    return 0;
}

static int accelerate_other_by_scale(lua_State *L)
{
    int id;
    double scale;
    bullet *b;
    
    id    = luaL_checkinteger(L, 1);
    scale = luaL_checknumber(L, 2);
    
    if (id < 0 || id >= 8192) {
        luaL_error(L, "Bullet argument %d not in valid range.", id);
    }
    
    b = &(bullet_mem[id]);
    
    b->velx *= scale;
    b->vely *= scale;
    b->vel_mag *= scale;
    
    return 0;
}

static int accelerate_other_by_rect(lua_State *L)
{
    int id;
    double ax, ay;
    bullet *b;
    
    id = luaL_checkinteger(L, 1);
    ax = luaL_checknumber(L, 2);
    ay = luaL_checknumber(L, 3);
    
    if (id < 0 || id >= 8192) {
        luaL_error(L, "Bullet argument %d not in valid range.", id);
    }
    
    b = &(bullet_mem[id]);
    
    b->velx += ax;
    b->vely += ay;
    set_pinvalid(b, TRUE);
    
    return 0;
}

static int set_velocity_self(lua_State *L)
{
    double vx, vy;
    
    check_null_context(L);
    
    vx = luaL_checknumber(L, 1);
    vy = luaL_checknumber(L, 2);
    
    context->velx = (float) vx;
    context->vely = (float) vy;
    set_pinvalid(context, TRUE);
    
    return 0;
}

static int set_velocity_other(lua_State *L)
{
    int id;
    double vx, vy;
    bullet *b;
    
    id = luaL_checkinteger(L, 1);
    vx = luaL_checknumber(L, 2);
    vy = luaL_checknumber(L, 3);
    
    if (id < 0 || id >= 8192) {
        luaL_error(L, "Bullet argument %d not in valid range.", id);
    }
    
    b = &(bullet_mem[id]);
    
    b->velx = (float) vx;
    b->vely = (float) vy;
    set_pinvalid(b, TRUE);
    
    return 0;
}

static int clear_bullets(lua_State *L)
{
    int i;
    
    for (i = 0; i < 8192; ++i) {
        if (&(bullet_mem[i]) != context) {
            set_killed(&(bullet_mem[i]), TRUE);
        }
    }
    
    return 0;
}

static int clear_bullets_rect(lua_State *L)
{
    /* TODO: stub - testing for compilation */
    return 0;
}

static int clear_bullets_circle(lua_State *L)
{
    /* TODO: stub - testing for compilation */
    return 0;
}

static int kill_me(lua_State *L)
{
    check_null_context(L);
    set_killed(context, TRUE);
    return 0;
}

static int kill_other(lua_State *L)
{
    int id;
    bullet *b;
    
    id = luaL_checkinteger(L, 1);
    
    if (id < 0 || id >= 8192) {
        luaL_error(L, "Bullet argument %d not in valid range.", id);
    }
    
    b = &(bullet_mem[id]);
    
    set_killed(context, TRUE);
    return 0;
}

/*
 * The translation table for Lua
 */

static const struct luaL_Reg bulletrain[] = {
    /* Used by the Lua script runner */
    {"set_bullet_context",         set_bullet_context},
    {"is_bullet_dead",             is_bullet_dead},
    {"kill_bullets",               kill_bullets},
    
    /* Bullet type functions */
    {"register_type",              register_type},
    {"unregister_type",            unregister_type},
    {"clear_types",                clear_types},
 
    /* Bullet creation functions */
    {"fire_bullet",                fire_bullet},
    {"create_enemy",               create_enemy},
    
    /* Bullet information functions */
    {"get_velocity_self",          get_velocity_self},
    {"get_velocity_other",         get_velocity_other},
 
    /* Bullet acceleration functions */
    {"accelerate_self_by_scale",   accelerate_self_by_scale},
    {"accelerate_self_by_rect",    accelerate_self_by_rect},
    {"accelerate_other_by_scale",  accelerate_other_by_scale},
    {"accelerate_other_by_rect",   accelerate_other_by_rect},
 
    /* Bullet velocity setting functions */
    {"set_velocity_self",          set_velocity_self},
    {"set_velocity_other",         set_velocity_other},
    
    /* Bullet clearing functions */
    {"clear_bullets",              clear_bullets},
    {"clear_bullets_rect",         clear_bullets_rect},
    {"clear_bullets_circle",       clear_bullets_circle},
 
    /* Bullet killing functions */
    {"kill_me",                    kill_me},
    {"kill_other",                 kill_other},
    
    /* sentinel */
    {NULL, NULL}
};

/* Opening the library in Lua */
int luaopen_bulletrain (lua_State *L)
{
    lua_newtable(L);
    luaL_newlib(L, bulletrain);
    lua_setglobal(L, "bulletrain");
    return 1;
}

/* Initialize some values */
int init_library(lua_State *L)
{
    int i;
    
    /* Only do this on first load, otherwise this will memory leak */
    if (first_load) {
        /* Set all of the types to have null SDL_Surfaces to ensure safety */
        for (i = 0; i < MAX_TYPES; ++i) {
            types[i].img = NULL;
        }
        
        first_load = FALSE;
    }
    
    /* This sets all the types to be invalid */
    clear_types(L);
    
    /* Make sure the context is sensible before any functions get run */
    context = NULL;
    
    /* Determine the pixel format */
    fmt = *((SDL_GetVideoInfo())->vfmt);
    
    return 0;
}
