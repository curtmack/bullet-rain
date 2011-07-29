/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * menu.c
 * Contains code for handling graphical menus
 */

#include "debug.h"
#include "geometry.h"
#include "resource.h"
#include "menu.h"

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"
#else
#include "SDL.h"
#include "SDL_thread.h"
#endif

int _menu_runner(void *data);

brmenu *create_menu(SDL_Surface *surface, SDL_Surface *back,
                    SDL_Rect backsrc, SDL_Rect backdst,
                    resource *sm, resource *se)
{
    brmenu *brm;
    int r;
    
    brm = malloc(sizeof(brmenu));
    panic(brm != NULL, "Could not allocate memory for new menu");
    
    brm->_lock = SDL_CreateMutex();
    r = SDL_mutexP(brm->_lock);
    check_mutex(r);
    
    brm->surface     = surface;
    brm->running     = FALSE;
    brm->thread      = NULL;
    brm->end         = -1;
    brm->entries     = NULL;
    brm->selected    = NULL;
    brm->back        = back;
    brm->backsrc     = backsrc;
    brm->backdst     = backdst;
    brm->sound_move  = sm;
    brm->sound_enter = se;
    
    r = SDL_mutexV(brm->_lock);
    check_mutex(r);
    
    return brm;
}

int start_menu(brmenu *brm)
{
    int r;
    
    r = SDL_mutexP(brm->_lock);
    check_mutex(r);
    
    /* Make the thread */
    brm->thread = SDL_CreateThread(&_menu_runner, brm);
   
    
    /* Did it work? */
    if (brm->thread != NULL) {
        brm->running = TRUE;
        r = SDL_mutexV(brm->_lock);
        check_mutex(r);
        return 0;
    }
    else {
        brm->running = FALSE;
        r = SDL_mutexV(brm->_lock);
        check_mutex(r);
        return 1;
    }
}

brmenu_entry *menu_add_entry(brmenu *brm, Sint32 id, Sint32 idup,
                             Sint32 iddown, Sint32 idleft, Sint32 idright,
                             SDL_Surface *off, SDL_Rect offsrc,
                             SDL_Rect offdst, SDL_Surface *on,
                             SDL_Rect onsrc, SDL_Rect ondst)
{
    int r;
    brmenu_entry *newentry, *temp;
    
    /* Allocate memory */
    newentry = malloc(sizeof(brmenu_entry));
    panic(newentry != NULL, "Could not allocate memory for new menu entry");
    
    /* Create the lock */
    newentry->_lock = SDL_CreateMutex();
    
    /* We'll need these locked for the remainder of the function */
    r = SDL_mutexP(newentry->_lock);
    check_mutex(r);
    r = SDL_mutexP(brm->_lock);
    check_mutex(r);
        
    /* Set up the stuff we know */
    newentry->id     = id;
    newentry->off    = off;
    newentry->offsrc = offsrc;
    newentry->offdst = offdst;
    newentry->on     = on;
    newentry->onsrc  = onsrc;
    newentry->ondst  = ondst;

    newentry->selected = FALSE;
    
    /* Add in positional ids */
    newentry->idup    = idup;
    newentry->iddown  = iddown;
    newentry->idleft  = idleft;
    newentry->idright = idright;
    
    /* Null out all positional pointers and callbacks to start */
    newentry->up            = NULL;
    newentry->down          = NULL;
    newentry->left          = NULL;
    newentry->right         = NULL;
    newentry->on_selected   = NULL;
    newentry->on_deselected = NULL;
    newentry->on_enter      = NULL;
    newentry->on_up         = NULL;
    newentry->on_down       = NULL;
    newentry->on_left       = NULL;
    newentry->on_right      = NULL;
    
    /* Add entry to the list */
    temp = brm->entries;
    if (temp == NULL) {
        brm->entries = newentry;
    }
    else {
        for (; temp->next != NULL; temp = temp->next);
        temp->next = newentry;
    }
    newentry->next = NULL;
    
    /* Unlock stuff */
    r = SDL_mutexV(brm->_lock);
    check_mutex(r);
    r = SDL_mutexV(newentry->_lock);
    check_mutex(r);
    
    return newentry;
}

void menu_link_entries(brmenu *brm)
{
    int r;
    brmenu_entry *current, *temp;
    
    /* Lock up the menu */
    r = SDL_mutexP(brm->_lock);
    check_mutex(r);

    /* Set up the up, left, down, right references for every entry */
    for (current = brm->entries; current != NULL; current = current->next) {
        r = SDL_mutexP(current->_lock);
        check_mutex(r);
        if (current->idup >= 0) {
            for (temp = brm->entries;
                 temp != NULL && temp->id != current->idup;
                 temp = temp->next);
        
            if (temp != NULL) {
                current->up = temp;
            }
            else {
                current->up = NULL;
            }
        }
    
        if (current->iddown >= 0) {
            for (temp = brm->entries;
                 temp != NULL && temp->id != current->iddown;
                 temp = temp->next);
            
            if (temp != NULL) {
                current->down = temp;
            }
            else {
                current->down = NULL;
            }
        }
        
        if (current->idleft >= 0) {
            for (temp = brm->entries;
                 temp != NULL && temp->id != current->idleft;
                 temp = temp->next);
            
            if (temp != NULL) {
                current->left = temp;
            }
            else {
                current->left = NULL;
            }
        }
        
        if (current->idright >= 0) {
            for (temp = brm->entries;
                 temp != NULL && temp->id != current->idright;
                 temp = temp->next);
            
            if (temp != NULL) {
                current->right = temp;
            }
            else {
                current->right = NULL;
            }
        }
        
        r = SDL_mutexV(current->_lock);
        check_mutex(r);
    }
    
    /* Unlock the menu */
    r = SDL_mutexV(brm->_lock);
    check_mutex(r);
}

void draw_menu(brmenu *brm)
{
    int r;
    SDL_Surface *surface;
    brmenu_entry *temp;
    
    /* We assume the calling function locked the menu for us */
    
    surface = brm->surface;
    
    /* Draw background */
    SDL_BlitSurface(brm->back, &(brm->backsrc), surface, &(brm->backdst));
    
    /* Draw entries */
    for (temp = brm->entries; temp != NULL; temp = temp->next) {
        r = SDL_mutexP(temp->_lock);
        check_mutex(r);
        
        if (temp->selected) {
            SDL_BlitSurface(temp->on, &(temp->offsrc),
                            surface, &(temp->offdst));
        }
        else {
            SDL_BlitSurface(temp->off, &(temp->onsrc),
                            surface, &(temp->ondst));
        }
        
        r = SDL_mutexV(temp->_lock);
        check_mutex(r);
    }
}

int wait_on_menu(brmenu *brm)
{
    if (brm->thread == NULL || !(brm->running)) {
        return 1;
    }
    
    SDL_WaitThread(brm->thread, NULL);
    return 0;
}

void destroy_menu(brmenu *brm)
{
    int r;
    brmenu_entry *temp, *next;
    
    if (brm->running) {
        /* Politely tell the thread to stop */
        r = SDL_mutexP(brm->_lock);
        check_mutex(r);
        brm->running = 0;
        r = SDL_mutexV(brm->_lock);
        check_mutex(r);
        SDL_WaitThread(brm->thread, NULL);
    }
    
    temp = brm->entries;
    while (temp != NULL) {
        next = temp->next;
        SDL_FreeSurface(temp->off);
        SDL_FreeSurface(temp->on);
        SDL_DestroyMutex(temp->_lock);
        free(temp);
        temp = next;
    }
    
    /* 
     * We'll assume brm->surface is actually managed by the main thread
     * and that it would be quite peeved if we freed it here
     * We can probably free the background though
     */
    SDL_FreeSurface(brm->back);
    SDL_DestroyMutex(brm->_lock);
    free(brm);
}


/* Handle input and process it into a menu action */
brmenu_action get_action(void)
{
    int r;
    brmenu_action todo;
    SDL_Event event;
    
    r = SDL_PollEvent(&event);
    if (r != 0) {
        /* We really only care about key and joystick events */
        /* TODO: Tie this into the input system so it can be configured */
        switch(event.type) {
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                    /* Check for up, down, left, right, enter */
                    case SDLK_UP:
                        todo = DO_UP;
                        break;
                    case SDLK_DOWN:
                        todo = DO_DOWN;
                        break;
                    case SDLK_LEFT:
                        todo = DO_LEFT;
                        break;
                    case SDLK_RIGHT:
                        todo = DO_RIGHT;
                        break;
                    case SDLK_RETURN:
                        todo = DO_CONFIRM;
                        break;
                    /* Need to clear todo otherwise */
                    default:
                        todo = DO_NOTHING;
                        break;
                }
                break;
            default:
                todo = DO_NOTHING;
                break;
        }
    }
    else {
        todo = DO_NOTHING;
    }
    
    return todo;
}


/* Set the menu action in a menu */
void menu_action(brmenu *brm, brmenu_action act)
{
    int r;
    
    r = SDL_mutexP(brm->_lock);
    check_mutex(r);
    
    if (brm->action == DO_NOTHING) {
        brm->action = act;
    }
    
    r = SDL_mutexV(brm->_lock);
    check_mutex(r);
}


/* The beast, this is the actual menu thread function */
int _menu_runner(void *data)
{
    int r;
    brmenu *me;
    brmenu_entry *temp;
    
    me = (brmenu*) data;
    
    /* Lock ourselves for some setup */
    r = SDL_mutexP(me->_lock);
    check_mutex(r);
    
    /* Finish some stuff */
    warn(me->entries != NULL, "Tried to start an empty menu");
    if (me->entries == NULL) {
        me->running = FALSE;
        return 0;
    }
    
    me->selected = me->entries; /* always start with first list entry */
    me->entries->selected = TRUE;
    
    /* Alright, let's get started */
    r = SDL_mutexV(me->_lock);
    check_mutex(r);
    
    debug("Menu started");
    
    while (me->running) {
        /*
         * We leave it to the external thread to redraw ourselves
         * that way they can sync it to the main drawing cycle
         * Furthermore, the main window thread has to handle input
         * events on some systems, so we let an external thread handle
         * that as well.
         * So, let's just see if we have something to do and handle it.
         */
        r = SDL_mutexP(me->_lock);
        check_mutex(r);
            
        switch (me->action) {
            case DO_NOTHING:
                /* we do nothing, obviously */
                break;                case DO_UP:
                if (me->selected->up != NULL) {
                    temp = me->selected->up;
                    me->selected->selected = FALSE;
                    temp->selected = TRUE;
                    
                    /* check callbacks */
                    if (me->selected->on_deselected != NULL) {
                        (*(me->selected->on_deselected))(me, me->selected);
                    }
                    if (temp->on_selected != NULL) {
                        (*(temp->on_selected))(me, temp);
                    }                        
                    me->selected = temp;                    
                }
                else if (me->selected->on_up != NULL) {
                    temp = (*(me->selected->on_up))(me, me->selected);
                    me->selected->selected = FALSE;
                    temp->selected = TRUE;
                    
                    /* check callbacks */
                    if (me->selected->on_deselected != NULL) {
                        (*(me->selected->on_deselected))(me, me->selected);
                    }
                    if (temp->on_selected != NULL) {
                        (*(temp->on_selected))(me, temp);
                    }                        
                    me->selected = temp;
                }
                break;
            
            case DO_DOWN:
                if (me->selected->down != NULL) {
                    temp = me->selected->down;
                    me->selected->selected = FALSE;
                    temp->selected = TRUE;
                    
                    /* check callbacks */
                    if (me->selected->on_deselected != NULL) {
                        (*(me->selected->on_deselected))(me, me->selected);
                    }
                    if (temp->on_selected != NULL) {
                        (*(temp->on_selected))(me, temp);
                    }
                        
                    me->selected = temp;
                }
                else if (me->selected->on_down != NULL) {
                    temp = (*(me->selected->on_down))(me, me->selected);
                    me->selected->selected = FALSE;
                    temp->selected = TRUE;
                        
                    /* check callbacks */
                    if (me->selected->on_deselected != NULL) {
                        (*(me->selected->on_deselected))(me, me->selected);
                    }
                    if (temp->on_selected != NULL) {
                        (*(temp->on_selected))(me, temp);
                    }
                        
                    me->selected = temp;
                }
                break;
            
            case DO_LEFT:
                if (me->selected->left != NULL) {
                    temp = me->selected->left;
                    me->selected->selected = FALSE;
                    temp->selected = TRUE;
                    
                    /* check callbacks */
                    if (me->selected->on_deselected != NULL) {
                        (*(me->selected->on_deselected))(me, me->selected);
                    }
                    if (temp->on_selected != NULL) {
                        (*(temp->on_selected))(me, temp);
                    }
                    
                    me->selected = temp;
                }
                else if (me->selected->on_left != NULL) {
                    temp = (*(me->selected->on_left))(me, me->selected);
                    me->selected->selected = FALSE;
                    temp->selected = TRUE;
                    
                    /* check callbacks */
                    if (me->selected->on_deselected != NULL) {
                        (*(me->selected->on_deselected))(me, me->selected);
                    }
                    if (temp->on_selected != NULL) {
                        (*(temp->on_selected))(me, temp);
                    }
                    
                    me->selected = temp;
                }
                break;
            
            case DO_RIGHT:
                if (me->selected->right != NULL) {
                    temp = me->selected->right;
                    me->selected->selected = FALSE;
                    temp->selected = TRUE;
                    
                    /* check callbacks */
                    if (me->selected->on_deselected != NULL) {
                        (*(me->selected->on_deselected))(me, me->selected);
                    }
                    if (temp->on_selected != NULL) {
                        (*(temp->on_selected))(me, temp);
                    }
                    
                    me->selected = temp;
                }
                else if (me->selected->on_right != NULL) {
                    temp = (*(me->selected->on_right))(me, me->selected);
                    me->selected->selected = FALSE;
                    temp->selected = TRUE;
                    
                    /* check callbacks */
                    if (me->selected->on_deselected != NULL) {
                        (*(me->selected->on_deselected))(me, me->selected);
                    }
                    if (temp->on_selected != NULL) {
                        (*(temp->on_selected))(me, temp);
                    }
                    
                    me->selected = temp;
                }
                break;
            
            case DO_CONFIRM:
                if (me->selected->on_enter != NULL) {
                    r = (*(me->selected->on_enter))(me, me->selected);
                }
                else r = TRUE;
                
                if (r) {
                    me->end = me->selected->id;
                    /* In case this brmenu is used again */
                    me->selected->selected = FALSE;
                    me->running = FALSE;
                }
                break;
            }
            
        /* Reset */
        me->action = DO_NOTHING;
        
        /* Unlock */
        r = SDL_mutexV(me->_lock);
        check_mutex(r);
        
        /* Delay a little while to avoid sucking up all the CPU cycles */
        SDL_Delay(20);
    }
    return 1;
}
