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
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
 * everything after the last dot, or everything if there is no dot
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
    
    debug("get_ext result:");
    debug2(a, ext);
    return ext;
}

/* 
 * Jenkins one-at-a-time hash 
 * NOTE TO SELF: If I ever change this algorithm, be sure
 * to change the precalculated hashes in resource.h!
 */
sid_t calculate_sid(char *string)
{
    uint32_t hash, i;

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
arclist *arc_tail = NULL;
pthread_mutex_t arc_lock = PTHREAD_MUTEX_INITIALIZER;

/* For creating human-readable description of what I'm doing */
pthread_mutex_t load_lock = PTHREAD_MUTEX_INITIALIZER;
char doing[40];

/* Get an archive from the arclist chain */
inline arclist *_get_arc_from_chain(sid_t id)
{
    arclist *temparc;
    
    pthread_mutex_lock(&arc_lock);
    for(temparc = arc_head; temparc != NULL && temparc->id != id;
        temparc = temparc->next);
    pthread_mutex_unlock(&arc_lock);
    return temparc;
}

/* Load an archive */
arclist *load_arc(char *arcname)
{
    int i, r;
    arclist *newarclist;
    resource *newresource, *tempres;
    sid_t reshash, temphash;
    void *tempdat;
    char *tempname;
    
    struct archive *newarc;
    struct archive_entry *entry;
    
    debug2("Starting to load archive", arcname);
    
    pthread_mutex_lock(&load_lock);
    
    temphash = calculate_sid(arcname);
    /* Do we already have this arclist in memory? */
    if ((newarclist = _get_arc_from_chain(temphash))) {
        debug("Archive found in arc chain");
        /* And is it loaded? */
        pthread_mutex_lock(&(newarclist->_lock));
        if(newarclist->loaded) {
            debug("Archive is loaded, we're done.");
            /* Then we're golden */
            pthread_mutex_unlock(&(newarclist->_lock));
            return newarclist;
        }
        /* still don't need to make a new one at least */
    }
    else {
        /* Make an arclist */
        sprintf(doing, "Initializing archive %s", arcname);
        debug(doing);
        
        newarclist = malloc(sizeof(arclist));
        panic2(newarclist, "Could not allocate memory for new arclist:", arcname);
        
        /* Prepare the lock */
        newarclist->_lock = PTHREAD_MUTEX_INITIALIZER;
                                        
        /* Lock it */
        pthread_mutex_lock(&(newarclist->_lock));
        
        /* Prepare other stuff */
        strcpy(newarclist->name, arcname);
        newarclist->id = temphash;
        /* wherever it goes, this is correct */
        newarclist->next = NULL;
        for (i = 0; i < ARCLIST_HASH_SIZE; ++i) {
            newarclist->map[i] = NULL;
        }
        
        /* Add it to the arclist chain */
        pthread_mutex_lock(&arc_lock);
        if (arc_head) {
            /* 
             * List is not currently empty - add it
             * We COULD sort these, but there are only like seven
             * archives, so why bother?
             */
             arc_tail->next = newarclist;
             arc_tail = newarclist;
        }
        else {
            /* List is empty - make it */
            arc_head = newarclist;
            arc_tail = newarclist;
        }
        pthread_mutex_unlock(&arc_lock);
    }
    /* Start loading the archive */
    sprintf(doing, "Opening up archive %s", arcname);
    debug(doing);
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
        
        sprintf(doing, "Reading in archive entry %s", tempname);
        debug(doing);
        
        /* Set up new resource entry */
        newresource = malloc(sizeof(resource));
        panic2(newresource, "Couldn't allocate memory for new resource",
                                                                    tempname);
                                                                    
        newresource->_lock = PTHREAD_MUTEX_INITIALIZER;
        
        /* Lock it */
        pthread_mutex_lock(&(newresource->_lock));
        
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
        
        debug2("Copied over filepath:", newresource->name);
        debugn("File is size", newresource->size);
        debugn("File has SID", (int)newresource->id);
        debugn("File will be stored in map slot", reshash%ARCLIST_HASH_SIZE);
        
        /* Try and determine filetype */
        temphash = calculate_sid(get_ext(newresource->name));
        switch (temphash) {
            case PNG_HASH:
                debug("Filetype is PNG");
                newresource->type = RES_IMAGE;
                break;
            case BIN_HASH:
                debug("Filetype is BIN");
                newresource->type = RES_TEXTURE;
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
        
        /* Store it in arclist */
        tempres = newarclist->map[reshash%ARCLIST_HASH_SIZE];
        if (tempres) {
            /* List is not empty - add it */
            /* We don't care about sorting this one either */
            for (; tempres != NULL; tempres = tempres->next);
            
            newresource->next = tempres->next;
            tempres->next = newresource;
        }
        else {
            /* list is empty - make it */
            newarclist->map[reshash%ARCLIST_HASH_SIZE] = newresource;
            newresource->next = NULL;
        }
        
        /* Now we need to load this entry, pretty standard stuff */
        tempdat = malloc((size_t)newresource->size);
        panic2(tempdat, "Couldn't allocate memory to load resource",
                newresource->name);
        archive_read_data(newarc, tempdat, (size_t)newresource->size);
        
        newresource->data = tempdat;
        
        /* Finally, unlock */
        pthread_mutex_unlock(&(newresource->_lock));
    }
    /* Everything's loaded, just need to clean some stuff */
    sprintf(doing, "Cleaning up internal copy of %s", arcname);
    debug(doing);
    
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
    pthread_mutex_unlock(&(newarclist->_lock));
    
    /* clear this out */
    doing[0] = '\0';
    debug2("Done loading archive", arcname);
    pthread_mutex_unlock(&load_lock);
    return newarclist;
}

/* Internal function to free an archive */
void _free_arc(arclist *arc)
{
    int i;
    resource *tempres, *nextres;
    
    /* Is it freed already? */
    if (!(arc->loaded)) {
    }
    else {
        /* Lock it */
        pthread_mutex_lock(&(arc->_lock));
        
        /* Free all resources */
        debug2("Freeing resources in arclist", arc->name);
        for (i = 0; i < ARCLIST_HASH_SIZE; ++i) {
            tempres = arc->map[i];
            while (tempres) {
                /* Free everything */
                debug2("Freeing resource", tempres->name);
                free(tempres->data);
                
                /* Free the resource itself, need to shuffle around a bit */
                nextres = tempres->next;
                free(tempres);
                tempres = nextres;
            }
            arc->map[i] = NULL;
        }
    
        /* Clean up */
        arc->loaded = 0;
        pthread_mutex_unlock(&(arc->_lock));
        
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
    _free_arc(_get_arc_from_chain(calculate_sid(arcname)));
}

/* Retrieve something that's been loaded */
arclist *get_arc(char *arcname)
{
    arclist *temp;
    sid_t archash;
    
    archash = calculate_sid(arcname);
    temp = _get_arc_from_chain(archash);
    if (temp) {
        /* If it's being worked on, wait */
        pthread_mutex_lock(&(temp->_lock));
        pthread_mutex_unlock(&(temp->_lock));
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
    
    temparc = get_arc(arcname); 
    /* Was the arclist freed? */
    if (!(temparc->loaded)) {
        /* We know we want to warn here */
        warn2(0, "Arclist was loaded but has since been freed, reloading:",
                arcname);
        temparc = load_arc(arcname);
    }
    
    reshash = calculate_sid(resname);
    /* Look for the resource we want */
    pthread_mutex_lock(&(temparc->_lock));
    for (tempres = temparc->map[reshash%ARCLIST_HASH_SIZE];
         tempres != NULL && tempres->id != reshash; tempres = tempres->next);
    pthread_mutex_unlock(&(temparc->_lock));
    
    if (tempres) {
        /* If it's being worked on, wait */
        pthread_mutex_lock(&(tempres->_lock));
        pthread_mutex_unlock(&(tempres->_lock));
    }
    warn2(tempres,
            "Arclist was loaded but it didn't have the requested resource:",
            resname);
    return tempres;
}

/* Initialize stuff needed by all resource functions */
void init_resources(void)
{
    /*
     * ... We don't actually do anything here
     * This is just here for legacy, and because having stop_ without init_
     * feels weird
     */
    
    debug("Resource loader initialized");
}

void stop_resources(void)
{
    arclist *temparc, *nextarc;
    
    /*
     * Clear EVERYTHING
     * We can safely assume that this will be called at a time when
     * nothing else needs anything we have loaded
     * Thus, we may hack away with impunity
     * Just to be safe, though, we'll still lock/unlock the arclist chain
     */
    
    pthread_mutex_lock(&arc_lock);
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
        nextarc = temparc->next;
        free(temparc);
        temparc = nextarc;
        
        debug("Arclist freed (can't display name, it's gone)");
    }
    pthread_mutex_unlock(&arc_lock);
    
    debug("Resources stopped and ready for engine closure.");
}
