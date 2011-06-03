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
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Clips any non-printing characters off the end of a string
 * Mainly for newlines
 * We should usually call this on a string before sending it
 * to calculate_sid
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
 * Jenkins one-at-a-time hash 
 * NOTE TO SELF: If I ever change this algorithm, be sure
 * to change the precalculated hashes below!
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

/* 
 * Precalculated hashes, used to detect filetype from extension
 * e.g. PNG_HASH is the result of calculate_sid(".png")
 */
#define PNG_HASH 0x5c4046f0
#define BIN_HASH 0x090e5a36
#define LUA_HASH 0xf4af8b0f
#define TXT_HASH 0x1c6ca03e
#define MAP_HASH 0x0dc0b9a8

/* TODO: Finish this gunk */
///* 
 //* This is used by the resource thread to keep track of 
 //* what it needs to do 
 //*/
//typedef enum {
	//RES_STATE_LOAD_ARC;
	//RES_STATE_LOAD_RES;
	//RES_STATE_FREE_ARC;
	//RES_STATE_FREE_RES;
	//RES_STATE_GARBAGE;
	//RES_STATE_STOP;
//} action_type;

//typedef struct action action;
//struct action {
	//action *next;
	
	//action_type type;

	//char[16] arc;
	//char[16] res;
//};

//action *queue_head;
//action *queue_tail;
//struct mutex_t *queue_lock;



///* This is used to keep track of loading information */
///* Progress goes from 0 to 32767 - easily enough granularity */
//int progress;
///* Human-readable description of what I'm doing */
//char[40] doing;

///* Used internally to add an action to the action queue */
//void _add_action_to_queue(action_type type, char *arc, char *res)
//{
	//action *new;
	//char *newarc, *newres;
	//const char nullstr[16] = "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	
	//action *new = malloc(sizeof(action));
	//panic(new, "Could not allocate memory for new resource action");
	///* have to make copies, these may be stack-allocated */
	//char *newarc = malloc(16);
	//char *newres = malloc(16);
	///* might be NULLs in here */
	//strncpy(newarc, (arc ? arc : nullstr), 15);
	//strncpy(newres, (res ? res : nullstr), 15);
	///* 
	 //* strncpy doesn't guarantee a null-terminator, hence copying
	 //* only 15 characters and setting it explicitly here
	 //*/
	//newarc[15]='\0';
	//newres[15]='\0';
	
	///* prepare new */
	//new -> next = NULL;
	//new -> type = type;
	//new -> arc = newarc;
	//new -> res = newres;
	
	//pthread_mutex_lock(queue_lock);
	//if (queue_head) {
		///* Queue is nonempty, just need to add the element */
		//queue_tail -> next = new;
		//queue_tail = new;
	//}
	//else {
		///* Queue is empty, need to fill it up */
		//queue_head = new;
		//queue_tail = new;
	//}
	//pthread_mutex_unlock(queue_lock);
//}



///*
 //* These functions just add actions to the queue, the thread function
 //* actually performs the loading and unloading
 //*/
 
//void load_arc(char *arcname)
//{
	//_add_action_to_queue(RES_STATE_LOAD_ARC, arcname, NULL);
//}

//void free_arc(char *arcname)
//{
	//_add_action_to_queue(RES_STATE_FREE_ARC, arcname, NULL);
//}

//void load_res(char *arcname, char *resname)
//{
	//_add_action_to_queue(RES_STATE_LOAD_RES, arcname, resname);
//}

//void free_res(char *arcname, char *resname)
//{
	//_add_action_to_queue(RES_STATE_FREE_RES, arcname, resname);
//}

//void garbage_collect(void)
//{
	//_add_action_to_queue(RES_STATE_GARBAGE, NULL, NULL);
//}

//extern arclist *get_arc(char *arcname);
//extern resource *get_res(char *arcname, char *resname);
	
//const struct timespec sleeptime = {0,10000000L} /* 1/100 sec */

///* Main resource thread loop, we don't use arg */
//void _resource_thread(void *arg)
//{
	//int r;
	//action *temp;
	//arclist *newarclist;
	//resource *newresource;
	//struct archive *newarc;
	//struct archive_entry *newentry;
	
	///* Main loop */
	//while (1) {
		//pthread_mutex_lock(queue_lock);
		//if (queue_head) {
			//switch(queue_head -> type) {
				//case RES_STATE_LOAD_ARC:
					///* Load an archive */
					//newarc = archive_read_new();
					
					///* All archives are ncompressed tarballs */
					//archive_read_support_filter_compress(newarc);
					//archive_read_support_format_tar(newarc);
					
					///* Open it up */
					//r = archive_read_open_filename(newarc,
										//queue_head->arc, 10240);
					//panic2(r==ARCHIVE_OK, "Couldn't open archive: ",
											//queue_head->arc);
					
					
					//break;
				
			//}
			
			///* clear the memory used by the action */
			//temp = queue_head;
			//queue_head = queue_head->next;
			//free(temp -> arc);
			//free(temp -> res);
			//free(temp);
		//}
		//pthread_mutex_unlock(queue_lock);
		///* take a nap */
		//nanosleep(&sleeptime, NULL);
	//}
//}

///* Starting or stopping the thread */
//void init_resources(pthread_t *t)
//{
	//int r;
	
	///* Intitialize queue mutex*/
	//r = pthread_mutex_init(queue_lock, NULL);
	//panic(!r, "Could not initialize queue_lock mutex");
	
	///* Create thread */
	//r = pthread_create(t, NULL, _resource_thread, NULL);
	//panic(!r, "Could not create resource loading thread");
//}

//void stop_resources(void)
//{
	//_add_action_to_queue(RES_STATE_STOP, NULL, NULL);
//}
