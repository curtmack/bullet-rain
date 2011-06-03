/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * debug.c
 * Contains code for different debug tasks, like creating memory
 * dumps and producing warnings
 */

#include "compile.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef DEBUG

void _debug (char *msg1, char *msg2) {
    printf("  --> DEBUG: %s %s\n", msg1, msg2);
}

void _warn (char *msg1, char *msg2, char *file, int line)
{
    fprintf(stderr, "  --> WARNING at %s line %d: %s %s\n",
									 file,  line,msg1,msg2);
}

#endif /* def DEBUG */

/* This we want in non-debug builds */

void _panic (char *msg1, char *msg2, char *file, int line)
{
    fprintf(stderr, "  --> PANIC at %s line %d: %s %s\n  Shutting down!\n",
                                   file,  line,msg1,msg2);

    /* 
     * TODO: Not much I can do at this point,
     * there's no memory to dump!
     */
    exit(1);
}
