/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * bullet.c
 * Contains code for working with bullets
 */

#include "compile.h"
#include "bullet.h"
#include "collmath.h"
#include "debug.h"

/* Memory to use for bullets */
bullet bullet_mem[8192];

/* Memory to use for extended bullets */
bullet_ext extended_mem[1024];

bullet    *free_bullets_head;
bullet    *free_bullets_tail;
SDL_mutex *free_bullets_lock;

bullet_ext *free_extended_head;
bullet_ext *free_extended_tail;
SDL_mutex  *free_extended_lock;

/* 
 * Make a bullet 
 * TODO: So much missing...
 */
bullet *make_bullet(fixed_t locx, fixed_t locy,
                    fixed_t velx, fixed_t vely, bullet_type *type)
{
    bullet *newbullet;
    int r;
    
    /* Get next free bullet off the list */
    r = SDL_mutexP(free_bullets_lock);
    check_mutex(r);
    
    if (free_bullets_head == NULL) {
        warn(FALSE, "Out of bullet memory!");
        r = SDL_mutexV(free_bullets_lock);
        check_mutex(r);
        return NULL;
    }
    
    newbullet = free_bullets_head;
    free_bullets_head = free_bullets_head->next;
    
    r = SDL_mutexV(free_bullets_lock);
    check_mutex(r);
    
    /* Start making the new bullet */
    memcpy(&(newbullet->rad), type, sizeof(bullet_type));
    set_alive(newbullet, TRUE);
    
    /* Copy over all the other stuff we have */
    newbullet->velx = velx;
    newbullet->vely = vely;
    newbullet->centerx = locx;
    newbullet->centery = locy;
    newbullet->extend = FALSE;
    newbullet->next = NULL;
    
    return newbullet;
}

/* 
 * Process a single bullet to completion
 * TODO: This does not do anything with the extended block!
 */

inline int process_bullet(bullet *bul)
{
    /* Update various coordinates */
    bul->centerx += bul->velx;
    bul->centery += bul->vely;
    bul->tlx     += bul->velx;
    bul->tly     += bul->vely;
    bul->lrx     += bul->velx;
    bul->lry     += bul->vely;
    
    /* Is it gone? */
    if (bul->centerx > OUT_OF_BOUNDS || bul->centerx < -OUT_OF_BOUNDS ||
        bul->centery > OUT_OF_BOUNDS || bul->centery < -OUT_OF_BOUNDS) {
        destroy_bullet(bul);
        return 1;
    }
    
    return 0;
}

/* Check if the bullet is colliding with the given circle */
inline int collide_bullet(bullet *bul, fixed_t px, fixed_t py, fixed_t rad)
{
    fixed_t sors = fixmul(rad+bul->rad, rad+bul->rad); /* sum of radii squared */
    return circle_collide(bul->centerx, bul->centery, px, py, sors);
}

/* Destroy a bullet */
void destroy_bullet(bullet *bul)
{
    int r;
    
    /* All we really need to do is put it back on the free list */
    r = SDL_mutexP(free_bullets_lock);
    check_mutex(r);
    
    if (free_bullets_head == NULL) {
        free_bullets_head = bul;
        free_bullets_tail = bul;
    }
    else {
        free_bullets_tail->next = bul;
        free_bullets_tail = bul;
    }
    bul->next = NULL;
    set_alive(bul, FALSE);
    
    r = SDL_mutexV(free_bullets_lock);
    check_mutex(r);
}

/* Sets up the linked lists */
int init_bullets(void)
{
    free_bullets_lock = SDL_CreateMutex();
    free_extended_lock = SDL_CreateMutex();
    reset_bullets();
    
    return 0;
}

/* Clears all bullets */
void reset_bullets(void)
{
    int i, r;
    
    r = SDL_mutexP(free_bullets_lock);
    check_mutex(r);
    
    /*
     * This loop links the entire array to itself,
     * and sets all the bullets to be dead
     */
    free_bullets_head = bullet_mem;
    free_bullets_tail = free_bullets_head;
    for (i = 1; i < 8191; ++i) {
        free_bullets_tail->next = &bullet_mem[i];
        set_alive(free_bullets_tail, FALSE);
        free_bullets_tail = free_bullets_tail->next;
    }
    /* And this finishes it off */
    free_bullets_tail->next = NULL;
    set_alive(free_bullets_tail, FALSE);
    
    r = SDL_mutexV(free_bullets_lock);
    check_mutex(r);
    
    /* Now to do the same thing with the extended blocks */
    r = SDL_mutexP(free_extended_lock);
    check_mutex(r);
    
    /* This loop links the entire array to itself */
    free_extended_head = extended_mem;
    free_extended_tail = free_extended_head;
    for (i = 1; i < 1023; ++i) {
        free_extended_tail->next = &extended_mem[i];
        free_extended_tail = free_extended_tail->next;
    }
    /* And this finishes it off */
    free_extended_tail->next = NULL;
    
    r = SDL_mutexV(free_extended_lock);
    check_mutex(r);
}

/* Destroys all bullets and the linked lists */
void stop_bullets(void)
{
    /* 
     * TODO: At the moment we don't actually care if we destroy the bullets,
     * but later on it will matter since we have to stop scripts and destroy
     * image surfaces in registered bullet types.
     * Really we just have to destroy the mutexes for now
     */
    
    SDL_DestroyMutex(free_bullets_lock);
    SDL_DestroyMutex(free_extended_lock);
}

inline void draw_bullet(bullet *bul, SDL_Surface *screen, int center_x, int center_y)
{
    SDL_Rect drawdst;
    
    if (!is_alive(bul)) return;
    drawdst.x = intpart(bul->centerx + bul->drawlocx) + center_x;
    drawdst.y = intpart(bul->centery + bul->drawlocy) + center_y;
    /* w and h are immaterial */
    SDL_BlitSurface(bul->img, NULL, screen, &drawdst);
}
