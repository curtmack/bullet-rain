/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * resource.h
 * Contains function prototypes for loading resources from archives,
 * unloading them, and listing the files in an archive.
 */

#ifndef RESOURCE_H

#define RESOURCE_H

#include "compile.h"
#include <stdint.h>

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"
#else
#include "SDL.h"
#include "SDL_thread.h"
#endif

/*
 * Here, we're using a hash map to store the different resources
 * loaded from a single archive.
 * It's an open hash map, so it handles collisions gracefully.
 * Each entry in the map is essentially a pointer to a linked list.
 * We're not worried about dynamic allocation here, because resources
 * will only be loaded and unloaded during load times.
 *
 * The arclists themselves are stored in a linked list. This will
 * be searched linearly; there won't be enough archives loaded to
 * worry about this.
 *
 * Outside code will interface with this system using strings. Again,
 * no reason to worry about performance overhead. When a piece of
 * code needs to keep track of a resource for later retrieval, it will
 * store the pointer to the resource structure.
 *
 * Unloading a resource will free the memory used for its data.
 * Unloading an archive will free the memory used for all of its loaded
 * resources, all of its resource entries, and its arclist structure.
 */

typedef Uint32 sid_t;

/*
 * Enum for telling us what resource type we're looking at
 * Some of these might be handled differently when being loaded, so this
 * is necessary.
 */
typedef enum {
    RES_IMAGE,   /* image intended to be used as a GUI graphic - PNG */
    RES_BINARY,  /*    binary data - is this needed? No idea   - BIN */
    RES_MIDI,    /*               MIDI music file              - MID */
    RES_SOUND,   /*       Ogg Vorbis sound or music file       - OGG */
    RES_SCRIPT,  /*                 Lua script                 - LUA */
    RES_STRING,  /*         some kind of string resource       - TXT */
    RES_MAP,     /*               level tile data              - MAP */
    RES_OTHER    /*  no idea, hope the code that uses it knows - ??? */
} restype;

/* 
 * Precalculated hashes, used to detect filetype from extension
 * e.g. PNG_HASH is the result of calculate_sid(".png")
 */
#define PNG_HASH 0x5c4046f0
#define BIN_HASH 0x090e5a36
#define MID_HASH 0xefe92d02
#define OGG_HASH 0x3cb30fc1
#define LUA_HASH 0xf4af8b0f
#define TXT_HASH 0x1c6ca03e
#define MAP_HASH 0x0dc0b9a8

typedef struct resource resource;
struct resource {
    Sint64   size; /* yeah that's the form libarchive gives it to us in */
    
    resource *next;
    char      name[16];
    sid_t     id;
    restype   type;
    void     *data;
    
    SDL_mutex *_lock;
};

typedef struct arclist arclist;
struct arclist {
    arclist  *next;
    sid_t     id;
    char      name[16];
    int       loaded;
    resource *map[ARCLIST_HASH_SIZE];
    
    SDL_mutex *_lock;
};

extern void clip_string(char *a);
extern char *get_ext(char *a);
extern sid_t calculate_sid(char *string);

/* Start and stop resources thread */
extern void init_resources(void);
extern void stop_resources(void);

/* Get progress information for loading screen */
extern int get_progress(char *buf, size_t n);

/*
 * load_arc loads an archive,  The get_ functions block
 * until the arc or res is loaded if necessary.
 *
 * OTHER SYSTEMS SHOULD NOT MODIFY ARCLISTS OR RESOURCES. ONLY
 * THE FUNCTIONS IN THIS FILE SHOULD MODIFY THEM, AND ONLY AFTER
 * CHECKING THE LOCK.
 * Other systems may, however, freely access the data loaded in
 * a resource, which is guaranteed to not be modified unless the
 * memory is relinquished (in which case we're doing something wrong
 * anyway).
 */

extern arclist *load_arc(char *arcname);
extern void free_arc(char *arcname);

extern arclist *get_arc(char *arcname);
extern resource *get_res(char *arcname, char *resname);

#endif /* def RESOURCE_H */
