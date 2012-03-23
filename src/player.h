/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * player.h
 * Contains structs and function prototypes for players and player bullets
 */

#include "bullet.h"
#include "compile.h"
#include "collmath.h"
#include "geometry.h"

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#else
#include "SDL.h"
#endif

#ifndef PLAYER_H

#define PLAYER_H

typedef struct player_ player;
struct player_ {
    /* Location data */
    float centerx;
    float centery;
    float rad;

    /* Drawing data */
    float drawlocx;
    float drawlocy;
    SDL_Surface *img;

    /* Flags */
    Sint32 flags;

    /* TODO: Need something to do with scripting */

    /* Animation data */
    SDL_Surface *anim[32];

    /* Game-specific data */
    Sint32 gamedata[32];
};

typedef struct pbullet_ pbullet;
struct pbullet_ {
    /* Next pbullet in free chain */
    pbullet *next;
    
    /* Movement data */
    float velx;
    float vely;
    float vel_mag;
    float vel_dir;
    
    /* Hitbox data - note that pbullets use AABBs, not circles */
    float tlx;
    float tly;
    float lrx;
    float lry;
    
    /* Drawing data, drawloc is relative to tl */
    float drawlocx;
    float drawlocy;
    SDL_Surface *img;

    /* Flags */
    Uint32 flags;
    Uint32 gameflags;

    /* Damage data */
    Sint32 enemy_damage;
    Sint32 boss_damage;
    
    /* TODO: Script stuff */
};

typedef struct pbullet_type_ pbullet_type;
struct pbullet_type_ {
    /* We memcpy this over when we make a new bullet */
    
    /* 
     * These are copied over, and then the specified starting position is
     * added to them - thus, they serve as offsets from the "center point"
     * even though the center point isn't technically a real thing
     */
    /* Hitbox data - note that pbullets use AABBs, not circles */
    float tlx;
    float tly;
    float lrx;
    float lry;
    
    /* Drawing data, drawloc is relative to tl */
    float drawlocx;
    float drawlocy;
    SDL_Surface *img;

    /* Flags */
    Uint32 flags;
    Uint32 gameflags;

    /* Damage data */
    Sint32 enemy_damage;
    Sint32 boss_damage;
};

/* These are the only flags we have right now */
/*
 * If PIERCE > enemy BLOCK, player bullet pierces
 * through that enemy
 * Thus BLOCK of 15 blocks everything
 */
#define P_PIERCE    0x0000000F

#define P_P_INVALID 0x00000010 /* polar velocity needs update */

#define P_ROTATE    0x00002000 /* rotate to movement angle - defaults to off */

#define P_ALIVE     0x10000000 /* 
                                * Always true on bullets that are active
                                * In reality, dead bullets have flags == 0
                                */

#define pget_pierce(bul)     ((bul)->flags & P_PIERCE)
#define pset_pierce(bul,prc) ((bul)->flags = ((bul)->flags&!P_PIERCE) | prc)

#define pis_pinvalid(bul) ((bul)->flags & P_P_INVALID)
#define pset_pinvalid(bul,cond) \
(cond ? ((bul)->flags = (bul)->flags | P_P_INVALID) : \
        ((bul)->flags = (bul)->flags & (!P_P_INVALID)))

#define pis_rotate(bul) ((bul)->flags & P_ROTATE)
#define pset_rotate(bul,cond) \
(cond ? ((bul)->flags = (bul)->flags | P_ROTATE) : \
        ((bul)->flags = (bul)->flags & (!P_ROTATE)))

#define pis_alive(bul) ((bul)->flags)
#define pset_alive(bul,cond) \
(cond ? ((bul)->flags = (bul)->flags | P_ALIVE) : \
        ((bul)->flags = 0))

/* Players, we allow 4 in case we want multiplayer some day */
extern player players[4];

/* Player bullets, 1024 is probably enough */
extern pbullet pbullet_mem[1024];

/* Linked list of free pbullets */
extern pbullet   *free_pbullets_head;
extern pbullet   *free_pbullets_tail;
extern SDL_mutex *free_pbullets_lock;

extern void register_player (int id, player plr);

extern player *get_player (int id);

extern inline void update_player (int id, player *plr);
extern inline void update_pbullet (pbullet *pbul);
extern inline int  collide_pbullet (pbullet *pbul, bullet *bul);
extern void destroy_pbullet (pbullet *pbul);

extern pbullet *make_pbullet (pbullet_type *type, float x, float y,
                              float xvel, float yvel, int polar);

extern inline void draw_player (player *plr, SDL_Surface *surface,
                                int center_x, int center_y);
extern inline void draw_pbullet (pbullet *pbul, SDL_Surface *surface,
                                 int center_x, int center_y);

extern void reset_pbullets(void);

extern int init_player(void);
extern void stop_player(void);

#endif /* !def PLAYER_H */
