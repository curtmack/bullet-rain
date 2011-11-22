/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * player.c
 * Contains code for players and player bullets
 */

#include "bullet.h"
#include "compile.h"
#include "collmath.h"
#include "coreship.h"
#include "geometry.h"
#include "player.h"

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#else
#include "SDL.h"
#endif

/* Players, we allow 4 in case we want multiplayer some day */
player players[4];

/* Player bullets, 1024 is probably enough */
pbullet pbullet_mem[1024];

/* Linked list of free pbullets */
pbullet   *free_pbullets_head;
pbullet   *free_pbullets_tail;
SDL_mutex *free_pbullets_lock;

void register_player (int id, player plr)
{
    players[id] = plr;
}

player *get_player (int id)
{
    return &(players[id]);
}

void update_player (int id, player *plr)
{
    /* 
     * TODO: At some point we need to make this actually check the script,
     * at the moment we only have coreships so we'll just use that
     */
    update_coreship(id, plr);
}

void update_pbullet (pbullet *pbul)
{
    (pbul->tl).x += (pbul->rect_vel).x;
    (pbul->tl).y += (pbul->rect_vel).y;
    (pbul->lr).x += (pbul->rect_vel).x;
    (pbul->lr).y += (pbul->rect_vel).y;
    
    if ((pbul->tl).x < tofixed(-1024,0) || (pbul->tl).y < tofixed(-1024,0) ||
        (pbul->lr).x > tofixed( 1024,0) || (pbul->lr).y > tofixed( 1024,0)) {
        
        pset_alive(pbul, FALSE);
    }
}

pbullet *make_pbullet (pbullet_type *type, fixed_t x, fixed_t y,
                       fixed_t xvel, fixed_t yvel, int polar)
{
    pbullet *pbul;
    int r;
    
    r = SDL_mutexP(free_pbullets_lock);
    check_mutex(r);
    
    if (free_pbullets_head == NULL) {
        warn(FALSE, "Couldn't make new pbullet!");
        r = SDL_mutexV(free_pbullets_lock);
        check_mutex(r);
        return NULL;
    }
    
    pbul = free_pbullets_head;
    free_pbullets_head = free_pbullets_head->next;
    
    r = SDL_mutexV(free_pbullets_lock);
    check_mutex(r);
    
    /* Start making the new pbullet */
    memcpy(&(pbul->tl), type, sizeof(pbullet_type));
    pset_alive(pbul, TRUE);
    
    /* Copy over all the other stuff we have */
    (pbul->rect_vel).x = xvel;
    (pbul->rect_vel).y = yvel;
    (pbul->tl).x += x;
    (pbul->tl).y += y;
    (pbul->lr).x += x;
    (pbul->lr).y += y;
    pbul->next = NULL;
    
    return pbul;
}

int collide_pbullet (pbullet pbul, bullet bul)
{
    return aabb_collide(pbul.tl, pbul.lr, bul.tl, bul.lr);
}

void draw_player (player plr, SDL_Surface *surface,
                  int center_x, int center_y)
{
    SDL_Rect rect;
    
    rect.x = intpart((plr.center).x + (plr.drawloc).x) + center_x;
    rect.y = intpart((plr.center).y + (plr.drawloc).y) + center_y;
    
    SDL_BlitSurface(plr.img, NULL, surface, &rect);
}

void draw_pbullet (pbullet pbul, SDL_Surface *surface,
                   int center_x, int center_y)
{
    SDL_Rect rect;
    
    if (!pis_alive(&pbul)) return;
    rect.x = intpart((pbul.tl).x + (pbul.drawloc).x) + center_x;
    rect.y = intpart((pbul.tl).y + (pbul.drawloc).y) + center_y;
    
    SDL_BlitSurface(pbul.img, NULL, surface, &rect);
}

void reset_pbullets(void)
{
    int i, r;
    
    r = SDL_mutexP(free_pbullets_lock);
    check_mutex(r);
    
    /*
     * This loop links the entire array to itself,
     * and sets all the pbullets to be dead
     */
    free_pbullets_head = pbullet_mem;
    pset_alive(free_pbullets_head, FALSE);
    
    free_pbullets_tail = free_pbullets_head;
    for (i = 1; i < 1023; ++i) {
        free_pbullets_tail->next = &pbullet_mem[i];
        pset_alive(free_pbullets_tail, FALSE);
        free_pbullets_tail = free_pbullets_tail->next;
    }
    /* And this finishes it off */
    free_pbullets_tail->next = NULL;
    pset_alive(free_pbullets_tail, FALSE);
    
    r = SDL_mutexV(free_pbullets_lock);
    check_mutex(r);
}

int init_player(void)
{    
    free_pbullets_lock = SDL_CreateMutex();
    reset_pbullets();
    
    return 0;   
}

void stop_player(void)
{
    SDL_DestroyMutex(free_pbullets_lock);
}
