/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * menu.h
 * Contains prototypes for handling graphical menus
 */

#ifndef MENU_H

#define MENU_H

#include "resource.h"

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"
#else
#include "SDL.h"
#include "SDL_thread.h"
#endif

/*
 * Don't want to take chances that "menu" isn't already used
 * by some compilers
 */
typedef struct brmenu_ brmenu;
typedef struct brmenu_entry_ brmenu_entry;

typedef enum {
    DO_NOTHING,
    DO_UP,
    DO_DOWN,
    DO_LEFT,
    DO_RIGHT,
    DO_CONFIRM
} brmenu_action;

struct brmenu_ {
    /* 
     * Linked list of entries
     * We start at the first entry on this list, regardless of id
     */
    brmenu_entry *entries;
    /* Which one is selected right now? */
    brmenu_entry *selected;
    
    /* Sound we should play when the menu cursor moves */
    resource *sound_move;
    
    /* Sound we should play when the user presses enter */
    resource *sound_enter;
    
    /* Threading stuff */
    SDL_Thread  *thread;
    SDL_mutex   *_lock;
    Sint32      running;
    
    /* Where to draw it */
    SDL_Surface *surface;
    
    /* Background? */
    SDL_Surface *back;
    SDL_Rect     backsrc;
    SDL_Rect     backdst;

    /* Where did we end? */
    Sint32 end;
    
    /* What do we do next? */
    brmenu_action action;
};

struct brmenu_entry_ {
    /* Next in linked list */
    brmenu_entry *next;
    
    /* Next in positional terms, when arrow keys are pressed */
    brmenu_entry *up;
    brmenu_entry *down;
    brmenu_entry *left;
    brmenu_entry *right;
    
    /* 
     * Positional ids, we need them so we can construct the pointers later
     * after all the menu entries are added
     */
    Sint32 idup;
    Sint32 iddown;
    Sint32 idleft;
    Sint32 idright;
    
    /* 
     * Used to encode menu result and identify entry for external access
     * Note: negative id's are taken to be "invalid," so if you give your
     * entry a negative id, you won't be able to access it. Don't do that.
     */
    Sint32 id;
    
    /* Drawing data, unselected and selected */
    SDL_Surface *off;
    SDL_Rect     offsrc;
    SDL_Rect     offdst;
    SDL_Surface *on;
    SDL_Rect     onsrc;
    SDL_Rect     ondst;
    
    /* Is this selected? */
    Sint32 selected;
    
    /* Some additional data, can be used as needed */
    Sint32 data[4];
    
    /*
     * Callbacks, if needed
     * Note that these aren't necessary for basic functionality.
     * Arguments are always the menu pointer and this entry's pointer
     */
    
    /* Called when the entry is selected */
    void (*on_selected)  (brmenu*, brmenu_entry*);

    /* Called when the entry is deselected */
    void (*on_deselected)(brmenu*, brmenu_entry*);

    /* 
    * Called if the confirm button is pressed while the entry is selected
    * If it returns FALSE, the menu doesn't stop like it normally does
    */
    int  (*on_enter)     (brmenu*, brmenu_entry*);

    /* 
    * Called when the cursor wants to move in the given direction, and
    * a normal pointer for that direction doesn't exist.
    * These all return a pointer to the new entry to select
    */
    brmenu_entry *(*on_up)    (brmenu*, brmenu_entry*);
    brmenu_entry *(*on_down)  (brmenu*, brmenu_entry*);
    brmenu_entry *(*on_left)  (brmenu*, brmenu_entry*);
    brmenu_entry *(*on_right) (brmenu*, brmenu_entry*);
    
    /* Lock */
    SDL_mutex *_lock;
};

extern brmenu *create_menu(SDL_Surface *surface, SDL_Surface *back,
                           SDL_Rect backsrc, SDL_Rect backdst,
                           resource *sm, resource *se);
extern int     start_menu(brmenu *brm);

extern brmenu_entry *menu_add_entry(brmenu *brm, Sint32 id, Sint32 idup,
                                    Sint32 iddown, Sint32 idleft,
                                    Sint32 idright,
                                    SDL_Surface *off, SDL_Rect offsrc,
                                    SDL_Rect offdst, SDL_Surface *on,
                                    SDL_Rect onsrc, SDL_Rect ondst);
extern void menu_link_entries(brmenu *brm);

extern void draw_menu   (brmenu *brm);
extern int  wait_on_menu(brmenu *brm);
extern void destroy_menu(brmenu *brm);

extern brmenu_action get_action (void);
extern void          menu_action(brmenu *brm, brmenu_action act);

#endif /* !def MENU_H */
