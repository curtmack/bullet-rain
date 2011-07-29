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

void _debug (char *msg1, char *msg2)
{
    printf("  --> DEBUG: %s %s\n", msg1, msg2);
    fflush(stdout);
}
void _debugn (char *msg1, int num) {
    printf("  --> DEBUG: %s %d\n", msg1, num);
    fflush(stdout);
}

#endif /* def DEBUG */

/* These we want in non-debug builds */

void _warn (char *msg1, char *msg2, char *file, int line)
{
    printf("  --> WARNING at %s line %d: %s %s\n", file, line, msg1, msg2);
    fflush(stdout);
}
void _warnn (char *msg1, int num, char *file, int line)
{
    printf("  --> WARNING at %s line %d: %s %d\n", file, line, msg1, num);
    fflush(stdout);
}

void _memdump(void)
{
    /* TODO: Not much I can do at this point, there's nothing to dump! */
}

void _panic (char *msg1, char *msg2, char *file, int line)
{
    printf("  --> PANIC at %s line %d: %s %s\n  Shutting down!\n",
                          file,  line,msg1,msg2);
    fflush(stdout);

    _memdump();
    exit(1);
}

void _panicn (char *msg1, int num, char *file, int line)
{
    printf("  --> PANIC at %s line %d: %s %d\n  Shutting down!\n",
                          file,  line,msg1,num);
    fflush(stdout);

    _memdump();
    exit(1);
}
