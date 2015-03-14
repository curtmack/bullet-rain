# The graphical menu system - `menu.c` #

## At a glance ##

Graphical menus are implemented in `menu.c`. If a source file needs access to this code, it should include `menu.h`.

Graphical menus are implemented with two structs, `brmenu` and `brmenu_entry`. (These are actually typedefs to the real structs.) `brmenu` represents a menu, and `brmenu_entry` represents an entry in that menu.


---


### Creation ###

To create a graphical menu, use `create_menu`. The prototype of `create_menu` is:

```
brmenu *create_menu(SDL_Surface *surface, SDL_Surface *back, SDL_Rect backsrc, SDL_Rect backdst,
                    resource *sm, resource *se);
```

  * **`surface`** is the surface the menu should draw itself too.
  * **`back`** is the background displayed underneath the menu.
  * **`backsrc`** is the source rectangle the background is blitted from.
  * **`backdst`** is the destination rectangle the background is blitted to.
  * **`sm`** is a pointer to the sound resource that should be played when the cursor is moved.
  * **`se`** is a pointer to the sound resource that should be played when the "Confirm" button is pressed.

The function returns a pointer to the created menu. It automatically allocates memory for it (using `malloc`, if you care).

The background may be `NULL`, in which case it will not be drawn (i.e. it will be treated as a perfectly transparent rectangle).


---


### Adding entries ###

To add entries to a graphical menu, use `menu_add_entry`. The prototype of `menu_add_entry` is:

```
brmenu_entry *menu_add_entry(brmenu *brm, Sint32 id, Sint32 idup, Sint32 iddown, Sint32 idleft, Sint32 idright,
                             SDL_Surface *off, SDL_Rect offsrc, SDL_Rect offdst,
                             SDL_Surface *on, SDL_Rect onsrc, SDL_Rect ondst);
```

  * **`brm`** is the menu the entry should be added to.
  * **`id`** is the numeric ID of the entry you wish to add.
  * **`idup`** is the numeric ID of the entry in the "up" direction, or negative if there is no entry in that direction.
  * **`iddown`** is the numeric ID of the entry in the "down" direction, or negative if there is no entry in that direction.
  * **`idleft`** is the numeric ID of the entry in the "left" direction, or negative if there is no entry in that direction.
  * **`idright`** is the numeric ID of the entry in the "right" direction, or negative if there is no entry in that direction.
  * **`off`** is the surface drawn when the entry is not selected.
  * **`offsrc`** is the source rectangle the off surface is blitted from.
  * **`offdst`** is the destination rectangle the off surface is blitted to.
  * **`on`** is the surface drawn when the entry is selected.
  * **`onsrc`** is the source rectangle the on surface is blitted from.
  * **`ondst`** is the source rectangle the on surface is blitted to.

The function returns a pointer to the created menu entry.


---


### Linking ###

After adding all the entries, the menu must be "linked." Do this with `menu_link_entries`. The prototype of `menu_link_entries` is:

```
void menu_link_entries(brmenu *brm);
```

  * **`brm`** is the menu to link.

Linking the menu turns all of the IDs you provided for `idup`, `iddown`, etc. into pointers to the actual entries they correspond to. This must be done at the end because menu entries will almost always be linked circularly, so it is not possible to link entries as they are added.


---


### Starting ###

To start the menu, use `start_menu`. The prototype of `start_menu` is:

```
int start_menu(brmenu *brm);
```

  * **`brm`** is the menu to start.

The function returns 0 on success and 1 on failure. It creates a thread which runs the menu from then on.


---


### Drawing ###

For safety reasons, menu's don't draw themselves. Instead, the thread that manages the menu will have to somehow ensure that `draw_menu` is called periodically (preferably, in the same thread that redraws the rest of the screen). `draw_menu` is the function that actually draws the menu. The prototype of `draw_menu` is:

```
void draw_menu(brmenu *brm);
```

  * **`brm`** is the menu to draw.

All of the drawing information for a menu is self-contained, so `draw_menu` needs no other information than the menu itself; it just needs to know when to do it.


---


### Handling a simple menu ###

For a simple menu, you first have to wait for the menu to stop running. Do this with `wait_on_menu`. The prototype of `wait_on_menu` is:

```
int wait_on_menu(brmenu *brm);
```

  * **`brm`** is the menu to wait for.

The function returns 0 if the menu finished, and 1 if the menu wasn't running when you called the function. (In most cases, this can be safely ignored.)

Assuming the menu stopped normally, the ID of the last menu entry highlighted (i.e. the entry that was highlighted when the user pressed enter) is stored in the `brmenu`'s `end` element. So, for example:

```
switch (brm->end) {
    case 0:
        /* code for menu entry 0 */
        break;
    case 1:
        /* code for menu entry 1 */
        break;
    ...
    default:
        /* code for when something goes hideously wrong */
        break;
}
```

This is the basic formula for a menu. There is usually no reason to go fancier than this, but...


---


### Making advanced menus with callbacks ###

If you need something fancier than the simple, garden-variety menu, the system supports the use of callbacks to enhance its functionality. Callbacks are function pointers that are part of every `brmenu_entry` struct. If these callbacks are non-`NULL`, then whenever certain events happen, the callback will be called.

The arguments to each callback are the menu and the entry from which the callback originated. They are defined as follows:

```
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
```

They are set to `NULL` by default. To use them, you'll have to set them manually. For example:

```
brmenu_entry *temp = menu_add_entry(/* stuff here */);
temp->on_enter = &check_valid;

...

int check_valid (brmenu *brm, brmenu_entry *ent)
{
    /* Check to see if the menu can exit right now */
    return state_is_consistent();
}
```

If the user presses the confirm key on that particular entry (and we could easily set this for every entry in the menu), it will only stop the menu if `state_is_consistent()` (presumably another function we've written for this purpose) returns true. If it returns false, the menu will continue running.