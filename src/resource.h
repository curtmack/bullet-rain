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
#include <pthread.h>
#include <stdint.h>

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
 * Unloading a resource will free the memory used for its data and
 * resource structure. Unloading an archive will free the memory used
 * for all of its loaded resources, all of its arcentries, and its
 * arclist structure.
 */

typedef uint32_t sid_t;

typedef enum {
    RES_IMAGE,   /* image intended to be used as a GUI graphic - PNG */
    RES_TEXTURE, /*     image intended to be used in-game      - BIN */
    RES_SCRIPT,  /*                 Lua script                 - LUA */
    RES_STRING,  /*         some kind of string resource       - TXT */
    RES_MAP,     /*               level tile data              - MAP */
    RES_OTHER    /*  no idea, hope the calling function knows  - ??? */
} restype;

typedef struct resource resource;
struct resource {
    resource *next;
    char      name[16];
    sid_t     hash;
    int       size;
    restype   type;
    int       refcount;
    void     *data;
};

typedef struct arclist arclist;
struct arclist {
	struct archive *arc;
	
    arclist  *next;
    sid_t     id;
    char      arcname[16];
    int       refcount;
    resource *map[ARCLIST_HASH_SIZE];
    
    struct mutex_t *_lock;
};

extern void clip_string(char *a);
extern sid_t calculate_sid(char *string);

/* Start and stop resources thread */
extern void init_resources(pthread_t *t);
extern void stop_resources(void);

/* Get progress bar information for loading screen */
extern int get_progress(char *doing);

/*
 * All of these functions just add an action to the action queue,
 * except the get_ functions, which block until the arc or res is
 * loaded if necessary. The get_ functions also increase the refcount..
 * 
 * Garbage collect frees all resources and arclists that have a
 * refcount of zero.
 * 
 * Note that when a resource is freed, the struct itself will not
 * be destroyed. Only the data is unloaded. The structs are freed
 * when stop_resources is called.
 *
 * OTHER SYSTEMS SHOULD NOT MODIFY ARCLISTS OR RESOURCES. ONLY
 * THE FUNCTIONS IN THIS FILE SHOULD MODIFY THEM, AND ONLY AFTER
 * CHECKING THE LOCK.
 * Other systems may, however, freely access the data loaded in
 * a resource, which is guaranteed to not be modified unless the
 * memory is relinquished.
 */

extern void     load_arc(char *arcname);
extern void     free_arc(char *arcname);
extern arclist *get_arc(char *arcname);

extern void      load_res(char *arcname, char *resname);
extern void      free_res(char *arcname, char *resname);
extern resource *get_res(char *arcname, char *resname);

extern void garbage_collect(void);

#endif /* def RESOURCE_H */
