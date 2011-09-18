/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * bullet.h
 * Contains structs, defines, and function prototypes for working with bullets
 */

#ifndef BULLET_H

#define BULLET_H

#include "compile.h"
#include "fixed.h"
#include "geometry.h"
#include "resource.h"

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"
#include "SDL/SDL_ttf.h"
#else
#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_ttf.h"
#endif


typedef struct bullet_     bullet;
typedef struct bullet_ext_ bullet_ext;

struct bullet_ {
       /* Pointer to next bullet in chain */
       bullet *next;

       /* Pointer to parent (NULL if none) */
       bullet *parent;

       /* Coordinates of bullet */
       rect_point center;

       /*
        * These next variables are in this order so they can be
        * memcpy'd from a bullet_type
        */

       /* Dimensions of bullet */
       fixed_t rad;

       /* Texture */
       SDL_Surface *img;
       
       /* Both engine and game-specific flags */
       Uint32 flags;
       Uint32 gameflags;
       
       /* Lua script resource */
       resource *lua_script;

       /* AABB information - relative from center */
       rect_point tl;
       rect_point lr;

       /* Display data */
       rect_point   drawloc;
       
       /* That's all we memcpy over */

       /* Velocity (rectangular and polar) */
       rect_point  rect_vel;
       polar_point polar_vel;

       /* Extended pointer */
       bullet_ext *extend;
};

struct bullet_ext_ {
    /* Next free block */
    bullet_ext *next;
    
	/* Animation data */
	SDL_Surface *frames[32];

	/* Scripting data */
	/* script_instance script; */

	/* Other data */
	int32_t hp;
	int32_t hp_max;

	/* Game-specific data */
	int32_t gamedata[16];
};

/* Bullet type information */
typedef struct bullet_type_ bullet_type;
struct bullet_type_ {
       /* Radius */
       fixed_t rad;

       /* Texture */
       SDL_Surface *img;

       /* Flags */
       Uint32 flags;
       Uint32 gameflags;

       /* Lua script */
       resource *lua;

       /* AABB information - relative from center */
       rect_point tl;
       rect_point lr;

       /* Draw location - relative from center */
       rect_point drawloc;
};

/* Defines for bitfield in bullet->flags */

/*
 * If player bullet PIERCE > BLOCK, player bullet pierces
 * through this enemy
 * Thus BLOCK of 15 blocks everything
 */
#define BLOCK 0x0000000F

#define P_INVALID 0x00000010 /* polar velocity needs update */
#define ENEMY     0x00000020 /* can be hurt by player bullets */
#define BOSS      0x00000040 /* draw HP bar, ignored if !ENEMY */
#define SCRIPTED  0x00000080 /* needs update in scripting step */

#define BOMBPROOF     0x00000100 /* immune to bombs */
#define ANCHOR_PARENT 0x00000200 /* moves with parent */
#define NO_COLLIDE    0x00000400 /* no collision with player */
#define WIDE_STOP     0x00000800 /* let live wider than offscreen */

#define KILL_ME   0x00001000 /* kill me now - set by script */
#define ROTATE    0x00002000 /* rotate to movement angle - defaults to off */

#define ALIVE     0x10000000 /* 
                              * Always true on bullets that are active
                              * In reality, dead bullets have flags == 0
                              */



/*
 * Defines for accessing bullet->flags
 * Virtually all compilers will optimize e.g. set_pinvalid(bul,1)
 * so we're not losing too much sleep over it
 */

#define get_block(bul)     (bul->flags & BLOCK)
#define set_block(bul,blk) (bul->flags = (bul->flags&!BLOCK) | blk)

#define is_pinvalid(bul) (bul->flags & P_INVALID)
#define set_pinvalid(bul,cond) \
(cond ? (bul->flags = bul->flags | P_INVALID) : \
        (bul->flags = bul->flags & (!P_INVALID)))

#define is_enemy(bul) (bul->flags & ENEMY)
#define set_enemy(bul,cond) \
(cond ? (bul->flags = bul->flags | ENEMY) : \
        (bul->flags = bul->flags & (!ENEMY)))

#define is_boss(bul) (bul->flags & BOSS)
#define set_boss(bul,cond) \
(cond ? (bul->flags = bul->flags | BOSS) : \
        (bul->flags = bul->flags & (!BOSS)))

#define is_scripted(bul) (bul->flags & SCRIPTED)
#define set_scripted(bul,cond) \
(cond ? (bul->flags = bul->flags | SCRIPTED) : \
        (bul->flags = bul->flags & (!SCRIPTED)))

#define is_bombproof(bul) (bul->flags & BOMBPROOF)
#define set_bombproof(bul,cond) \
(cond ? (bul->flags = bul->flags | BOMBPROOF) : \
        (bul->flags = bul->flags & (!BOMBPROOF)))

#define is_anchored(bul) (bul->flags & ANCHOR_PARENT)
#define set_anchored(bul,cond) \
(cond ? (bul->flags = bul->flags | ANCHOR_PARENT) : \
        (bul->flags = bul->flags & (!ANCHOR_PARENT)))

#define is_nocoll(bul) (bul->flags & NO_COLLIDE)
#define set_nocoll(bul,cond) \
(cond ? (bul->flags = bul->flags | NO_COLLIDE) : \
        (bul->flags = bul->flags & (!NO_COLLIDE)))

#define is_widestop(bul) (bul->flags & WIDE_STOP)
#define set_widestop(bul,cond) \
(cond ? (bul->flags = bul->flags | WIDE_STOP) : \
        (bul->flags = bul->flags & (!WIDE_STOP)))

#define is_killed(bul) (bul->flags & KILL_ME)
#define set_killed(bul,cond) \
(cond ? (bul->flags = bul->flags | KILL_ME) : \
        (bul->flags = bul->flags & (!KILL_ME)))

#define is_rotate(bul) (bul->flags & ROTATE)
#define set_rotate(bul,cond) \
(cond ? (bul->flags = bul->flags | ROTATE) : \
        (bul->flags = bul->flags & (!ROTATE)))

#define is_alive(bul) (bul->flags)
#define set_alive(bul,cond) \
(cond ? (bul->flags = bul->flags | ALIVE) : \
        (bul->flags = 0))



/* Memory to use for bullets */
extern bullet bullet_mem[8192];

/* Memory to use for extended bullets */
extern bullet_ext extended_mem[1024];

/* Linked list of free bullet / extended block space */
extern bullet    *free_bullets_head;
extern bullet    *free_bullets_tail;
extern SDL_mutex *free_bullets_lock;

extern bullet_ext *free_extended_head;
extern bullet_ext *free_extended_tail;
extern SDL_mutex  *free_extended_lock;

bullet *make_bullet(rect_point loc, rect_point vel, bullet_type *type);

int process_bullet(bullet *bul);
int collide_bullet(bullet *bul, rect_point p, fixed_t rad);

void destroy_bullet(bullet *bul);

/* Sets up the linked lists */
int init_bullets(void);

/* Clears all bullets */
void reset_bullets(void);

/* Destroys all bullets and the linked lists */
int stop_bullets(void);

/* This is only a few function calls, so it can be inline */
inline void draw_bullet(bullet *bul, SDL_Surface *screen,
                        int center_x, int center_y);

/* The extents of the squares at which bullets disappear */
#define OUT_OF_BOUNDS      tofixed(400,0)
#define OUT_OF_BOUNDS_WIDE tofixed(800,0)

#endif /* !def BULLET_H */
