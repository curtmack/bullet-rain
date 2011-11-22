/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * resource.h
 * Contains code for loading resources from archives, unloading
 * them, and listing the files in an archive.
 * Uses libarchive.
 */

#include "compile.h"
#include "debug.h"
#include "resource.h"
#include <archive.h>
#include <archive_entry.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef INCLUDE_SDL_PREFIX
#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"
#include "SDL/SDL_image.h"
#else
#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_image.h"
#endif

/*
 * Clips any non-printing characters off the end of a string
 * Mainly for stripping newlines in user-input text
 */
void clip_string(char *a)
{
    int i = 0;
    for (; isprint((int)a[i]); ++i);
    /*
     * If a string falls in a forest, and we clip up to the terminating
     * null, do we care?
     */
    a[i] = '\0';
}

/*
 * Gets the extension of a filename
 * everything after and including the last dot, or everything if there is no dot
 * Used to detect filetype
 */
char *get_ext(char *a)
{
    char *ext = a;
    char *temp = a;
    while (*(++temp)) {
        if (*temp == '.') {
            ext = temp;
        }
    }
    
    verbose("get_ext result:");
    verbose2(a, ext);
    return ext;
}

/* 
 * Jenkins one-at-a-time hash 
 * NOTE TO SELF: If I ever change this algorithm, be sure
 * to change the precalculated hashes in resource.h!
 */
sid_t calculate_sid(char *string)
{
    Uint32 hash, i;

    /*
     * We assume here string is properly null-terminated,
     * this saves time having to run strlen
     */

    /* Not much to say here. This is the algorithm. */
    for (hash = i = 0; string[i] != '\0'; ++i) {
        hash += string[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return (sid_t)hash;
}

/* arclist chain */
arclist *arc_head = NULL;
SDL_mutex *arc_lock;

/* For creating human-readable description of what I'm doing */
SDL_mutex *load_lock;
int progress;
char doing[64];

/* Return the human-readable description of what I'm doing */
int get_progress(char *buf, size_t n)
{
    int r;
    
    r = SDL_mutexP(load_lock);
    check_mutex(r);
    strncpy(buf, doing, n);
    return progress;
    r = SDL_mutexV(load_lock);
    check_mutex(r);
}

/* Reset the progress spinner */
void reset_progress(void)
{
    int r;
    
    r = SDL_mutexP(load_lock);
    check_mutex(r);
    progress = 0;
    r = SDL_mutexV(load_lock);
    check_mutex(r);
}

/* 
 * "Doctor" a resource to its finished format
 * Mainly, this converts PNGs to SDL_Surfaces
 */
void _doctor_resource(resource *res)
{
    SDL_Surface *img, *opt;
    SDL_RWops   *rwop;
    
    /* We assume the calling function has already locked the resource */
    switch (res->type) {
        case RES_IMAGE:
            /*  Need to go through SDL_RWops to load an image from memory */
            debug2("Doctoring image:", res->name);
            verbose("Creating SDL_RWops");
            rwop = SDL_RWFromMem(res->data, res->size);
            panic(rwop != NULL, "Failed to set up SDL_RWops");
            verbose("Loading PNG from SDL_RWops");
            img  = IMG_LoadPNG_RW(rwop);
            verbose("Cleaning up");
            SDL_FreeRW(rwop);
            /* Now we need to optimize the surface */
            opt = SDL_DisplayFormat(img);
            SDL_FreeSurface(img);
            free(res->data);
            res->data = (void*)opt;
            break;
        default:
            break;
    }
}

/* Get an archive from the arclist chain */
inline arclist *_get_arc_from_chain(sid_t id, char *arcname)
{
    arclist *temparc;
    int r;
    
    r = SDL_mutexP(arc_lock);
    check_mutex(r);
    for(temparc = arc_head; temparc != NULL && temparc->id != id;
        temparc = temparc->next);
    /* Check the strings too, because I'm paranoid */
    /* for(; temparc != NULL && (r = strcmp(temparc->name, arcname)) < 0;
        temparc = temparc->next); */
    /* Did we miss it? */
    if (r > 0) temparc = NULL;
    r = SDL_mutexV(arc_lock);
    check_mutex(r);
    return temparc;
}

/* Load an archive */
arclist *load_arc(char *arcname)
{
    int i, r;
    arclist *newarclist, *curr, *prev;
    resource *newresource, *tempres, *prevres;
    sid_t reshash, temphash;
    void *tempdat;
    char *tempname;
    
    struct archive *newarc;
    struct archive_entry *entry;
    
    debug2("Starting to load archive", arcname);
    
    temphash = calculate_sid(arcname);
    /* Do we already have this arclist in memory? */
    if ((newarclist = _get_arc_from_chain(temphash, arcname))) {
        debug("Archive found in arc chain");
        /* And is it loaded? */
        r = SDL_mutexP(newarclist->_lock);
        check_mutex(r);
        if(newarclist->loaded) {
            /* Then we're golden */
            debug("Archive is loaded, we're done.");
            r = SDL_mutexV(newarclist->_lock);
            check_mutex(r);
            return newarclist;
        }
        /* if not, we sill don't need to make a new one at least */
    }
    else {
        /* Make an arclist */
        r = SDL_mutexP(load_lock);
        check_mutex(r);
        sprintf(doing, "Initializing archive %s", arcname);
        ++progress;
        debug(doing);
        r = SDL_mutexV(load_lock);
        check_mutex(r);
        
        newarclist = malloc(sizeof(arclist));
        panic2(newarclist, "Could not allocate memory for new arclist:", arcname);
        
        /* Prepare the lock */
        newarclist->_lock = SDL_CreateMutex();
                                        
        /* Lock it */
        r = SDL_mutexP(newarclist->_lock);
        check_mutex(r);
        
        /* Prepare other stuff */
        strcpy(newarclist->name, arcname);
        newarclist->id = temphash;
        for (i = 0; i < ARCLIST_HASH_SIZE; ++i) {
            newarclist->map[i] = NULL;
        }
        
        /* Add it to the arclist chain */
        r = SDL_mutexP(arc_lock);
        check_mutex(r);
        if (arc_head) {
            /* 
             * List is not currently empty - add it
             * We need it sorted to make the string checks more efficient
             */
            curr = arc_head;
            prev = curr;
            for (; curr != NULL && curr->id < temphash;
                prev = curr, curr = curr->next);
            if (curr->id == temphash) {
                /* Settle hash ties via strcmp */
                for (; curr != NULL && strcmp(curr->name, arcname) < 0;
                    prev = curr, curr = curr->next);
            }
            /* Now we should be at the right place */
            prev->next = newarclist;
            newarclist->next = curr;
        }
        else {
            /* List is empty - make it */
            arc_head = newarclist;
            newarclist->next = NULL;
        }
        r = SDL_mutexV(arc_lock);
        check_mutex(r);
    }
    
    /* Start loading the archive */
    r = SDL_mutexP(load_lock);
    check_mutex(r);
    sprintf(doing, "Opening up archive %s", arcname);
    ++progress;
    debug(doing);
    r = SDL_mutexV(load_lock);
    check_mutex(r);
    
    newarc = archive_read_new();
    /* All archives are gzipped tarballs */
    
    /* 
     * "libarchive version 3 is just around the corner! Honest!"
     *  -- Libarchive developers, Mar 2010
     */
#if ARCHIVE_VERSION_NUMBER < 3000000
    archive_read_support_compression_gzip(newarc);
#else
    archive_read_support_filter_gzip(newarc);
#endif

    archive_read_support_format_tar(newarc);
    
    /* Open it up */
    r = archive_read_open_filename(newarc, arcname, 10240);
    panic2(r==ARCHIVE_OK, "Couldn't open archive", arcname);
    
    /*
     * Start reading
     * Yes, we're sending a pointer to the pointer to the entry.
     * This is by design. I have no idea why.
     */
    while (archive_read_next_header(newarc, &entry) == ARCHIVE_OK) {
        tempname = (char*) archive_entry_pathname(entry);
        
        r = SDL_mutexP(load_lock);
        check_mutex(r);
        sprintf(doing, "Reading in archive entry %s", tempname);
        ++progress;
        debug(doing);
        r = SDL_mutexV(load_lock);
        check_mutex(r);
        
        /* Set up new resource entry */
        newresource = malloc(sizeof(resource));
        panic2(newresource, "Couldn't allocate memory for new resource",
                                                                    tempname);
                                                                    
        newresource->_lock = SDL_CreateMutex();
        
        /* Lock it */
        r = SDL_mutexP(newresource->_lock);
        check_mutex(r);
        
        /* 
         * Copy over some stuff
         * Note: libarchive doesn't guarantee that the size is
         * known, but the tar format does, so we'll assume that
         * if archive_entry_size returns 0, then the file really
         * does have a size of zero. Although I'm not sure why
         * we would ever need that...
         */
        strncpy(newresource->name, archive_entry_pathname(entry), 15);
        (newresource->name)[15] = '\0';
        reshash = calculate_sid(newresource->name);
        newresource->id = reshash;
        newresource->size = archive_entry_size(entry);
        newresource->data = NULL;
        
        /* Good to know! */
        debug2("Copied over filepath:", newresource->name);
        /* Less good to know */
        verbosen("File is size", newresource->size);
        verbosen("File has SID", (int)newresource->id);
        verbosen("File will be stored in map slot", reshash%ARCLIST_HASH_SIZE);
        
        /* Try and determine filetype */
        temphash = calculate_sid(get_ext(newresource->name));
        switch (temphash) {
            case PNG_HASH:
                debug("Filetype is PNG");
                newresource->type = RES_IMAGE;
                break;
            case BIN_HASH:
                debug("Filetype is BIN");
                newresource->type = RES_BINARY;
                break;
            case MID_HASH:
                debug("Filetype is MID");
                newresource->type = RES_MIDI;
                break;
            case OGG_HASH:
                debug("Filetype is OGG");
                newresource->type = RES_SOUND;
                break;
            case LUA_HASH:
                debug("Filetype is LUA");
                newresource->type = RES_SCRIPT;
                break;
            case TXT_HASH:
                debug("Filetype is TXT");
                newresource->type = RES_STRING;
                break;
            case MAP_HASH:
                debug("Filetype is MAP");
                newresource->type = RES_MAP;
                break;
            default:
                debug("Filetype is unrecognized (this is not an error)");
                newresource->type = RES_OTHER;
        }
        
        r = SDL_mutexP(load_lock);
        check_mutex(r);
        sprintf(doing, "Mapping resource %s", newresource->name);
        ++progress;
        debug(doing);
        r = SDL_mutexV(load_lock);
        check_mutex(r);
        
        /* Store it in arclist */
        tempres = newarclist->map[reshash%ARCLIST_HASH_SIZE];
        if (tempres) {
            /* 
             * List is not empty - add it 
             * Again, need to sort it to make hash collisions less painful
             * via string checks
             *
             * Thanks to the if, we know tempres != NULL on the first
             * iteration, so we don't need to worry about intializing prevres
             */
            for (; tempres != NULL && tempres->id < reshash;
                prevres = tempres, tempres = tempres->next);
            if (tempres->id == reshash) {
                /* String checks ahoy! */
                for (; tempres != NULL &&
                    strcmp(tempres->name, newresource->name) < 0;
                    prevres = tempres, tempres = tempres->next);
            }
            /* And now we're in position */
            prevres->next = newresource;
            newresource->next = tempres;
        }
        else {
            /* list is empty - make it */
            newarclist->map[reshash%ARCLIST_HASH_SIZE] = newresource;
            newresource->next = NULL;
        }
        
        /* Now we need to load this entry, pretty standard stuff */
        
        r = SDL_mutexP(load_lock);
        check_mutex(r);
        sprintf(doing, "Loading resource %s into memory", newresource->name);
        ++progress;
        debug(doing);
        r = SDL_mutexV(load_lock);
        check_mutex(r);
        
        tempdat = malloc((size_t)newresource->size);
        panic2(tempdat, "Couldn't allocate memory to load resource",
                newresource->name);
        archive_read_data(newarc, tempdat, (size_t)newresource->size);
        
        newresource->data = tempdat;
        
        /* Convert from file format to internal format, if needed */
        _doctor_resource(newresource);
        
        /* Finally, unlock */
        r = SDL_mutexV(newresource->_lock);
        check_mutex(r);
    }
    /* Everything's loaded, just need to clean some stuff */
    
    r = SDL_mutexP(load_lock);
    check_mutex(r);
    sprintf(doing, "Cleaning up internal copy of %s", arcname);
    ++progress;
    debug(doing);
    r = SDL_mutexV(load_lock);
    check_mutex(r);
    
    /* 
     * "libarchive version 3 is just around the corner! Honest!"
     *  -- Libarchive developers, Mar 2010
     */
#if ARCHIVE_VERSION_NUMBER < 3000000
    archive_read_finish(newarc);
#else
    archive_read_free(newarc);
#endif

    newarclist->loaded = 1;
    r = SDL_mutexV(newarclist->_lock);
    check_mutex(r);
    
    /* clear this out */
    r = SDL_mutexP(load_lock);
    check_mutex(r);
    doing[0] = '\0';
    debug2("Done loading archive", arcname);
    r = SDL_mutexV(load_lock);
    check_mutex(r);
    return newarclist;
}

/* Internal function to free an archive */
void _free_arc(arclist *arc)
{
    resource *tempres, *nextres;
    int i, r;
    
    /* Is it freed already? */
    if (!(arc->loaded)) {
    }
    else {
        /* Lock it */
        r = SDL_mutexP(arc->_lock);
        check_mutex(r);
        
        /* Free all resources */
        debug2("Freeing resources in arclist", arc->name);
        for (i = 0; i < ARCLIST_HASH_SIZE; ++i) {
            tempres = arc->map[i];
            while (tempres) {
                /* Free everything */
                debug2("Freeing resource", tempres->name);
                /* Wait, how DO we free it? */
                switch (tempres->type) {
                    case RES_IMAGE:
                        SDL_FreeSurface((SDL_Surface*)tempres->data);
                        break;
                    default:
                        free(tempres->data);
                        break;
                }
                SDL_DestroyMutex(tempres->_lock);
                
                /* Free the resource itself, need to shuffle around a bit */
                nextres = tempres->next;
                free(tempres);
                tempres = nextres;
            }
            arc->map[i] = NULL;
        }
    
        /* Clean up */
        arc->loaded = 0;
        r = SDL_mutexV(arc->_lock);
        check_mutex(r);
        
        debug2("Done freeing archive", arc->name);
    }
} 

/*
 * Free an archive
 * Note that the arclist struct STAYS IN MEMORY, but all the resources therein
 * are freed.
 */
void free_arc(char *arcname)
{
    _free_arc(_get_arc_from_chain(calculate_sid(arcname), arcname));
}

/* Retrieve something that's been loaded */
arclist *get_arc(char *arcname)
{
    arclist *temp;
    sid_t archash;
    int r;
    
    archash = calculate_sid(arcname);
    temp = _get_arc_from_chain(archash, arcname);
    if (temp) {
        /* If it's being worked on, wait */
        r = SDL_mutexP(temp->_lock);
        check_mutex(r);
        r = SDL_mutexV(temp->_lock);
        check_mutex(r);
        return temp;
    }
    else {
        /* We need to load it */
        warn2(0, "Requested arclist was not loaded, loading it now:", arcname);
        return load_arc(arcname);
    }
}

resource *get_res(char *arcname, char *resname)
{
    arclist *temparc;
    resource *tempres;
    sid_t reshash;
    int r;
    
    temparc = get_arc(arcname); 
    /* Was the arclist freed? */
    if (!(temparc->loaded)) {
        warn2(FALSE, "Arclist was loaded but has since been freed, reloading:",
                arcname);
        temparc = load_arc(arcname);
    }
    
    reshash = calculate_sid(resname);
    /* Look for the resource we want */
    r = SDL_mutexP(temparc->_lock);
    check_mutex(r);
    for (tempres = temparc->map[reshash%ARCLIST_HASH_SIZE];
         tempres != NULL && tempres->id < reshash; tempres = tempres->next);
    
    /* Did we reach the end of the list? */
    if (tempres == NULL) {
        r = SDL_mutexV(temparc->_lock);
        check_mutex(r);
        warn2(FALSE, "Arclist didn't have the requested resource:", resname);
        return NULL;
    }
    
    /* If there's a hash tie, check for strings */
    if (tempres->id == reshash) {
        for(; tempres != NULL && strcmp(tempres->name, resname) < 0;
            tempres = tempres->next);
        /* Did we go too far? */
        if (strcmp(tempres->name, resname) > 0) {
            tempres = NULL;
        }
    }
    /* Did we go too far? */
    else if (tempres->id > reshash) {
        tempres = NULL;
    }
    
    r = SDL_mutexV(temparc->_lock);
    check_mutex(r);
    
    /* Check again for NULL, since we may have set NULL since then */
    if (tempres == NULL) {
        warn2(FALSE, "Arclist didn't have the requested resource:", resname);
        return NULL;
    }
    
    /* If it's being worked on, wait */
    r = SDL_mutexP(tempres->_lock);
    check_mutex(r);
    r = SDL_mutexV(tempres->_lock);
    check_mutex(r);
    
    return tempres;
}

/* Initialize stuff needed by all resource functions */
void init_resources(void)
{
    arc_lock = SDL_CreateMutex();
    load_lock = SDL_CreateMutex();
    debug("Resource loader initialized");
}

void stop_resources(void)
{
    arclist *temparc, *nextarc;
    int r;
    
    /*
     * Clear EVERYTHING
     * We can safely assume that this will be called at a time when
     * nothing else needs anything we have loaded
     * Thus, we may hack away with impunity
     * Just to be safe, though, we'll still lock/unlock the arclist chain
     */
    
    r = SDL_mutexP(arc_lock);
    check_mutex(r);
    temparc = arc_head;
    while (temparc) {
        debug2("Clearing arclist", temparc->name);
        
        /* Is it still loaded? If so, free it */
        if (temparc->loaded) {
            debug2("Arclist still loaded, freeing it:", temparc->name);
            _free_arc(temparc);
        }
        
        /* Now go to work on its guts */
        debug2("Freeing arclist memory:", temparc->name);
        SDL_DestroyMutex(temparc->_lock);
        nextarc = temparc->next;
        free(temparc);
        temparc = nextarc;
        
        debug("Arclist freed");
    }
    r = SDL_mutexV(arc_lock);
    check_mutex(r);
    
    debug("Freeing miscellaneous memory");
    SDL_DestroyMutex(arc_lock);
    SDL_DestroyMutex(load_lock);
    
    debug("Resources stopped and ready for engine closure.");
}
