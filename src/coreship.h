/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * coreship.h
 * Contains function prototypes for the coreship, a player built into the
 * engine for use in the system test
 */

#include "compile.h"
#include "player.h"

#ifndef CORESHIP_H

#define CORESHIP_H

/* These values are all stored in gamedata, thus we need indices for them */

/* Timing our shots */
#define TIME_TO_MAIN_SHOT 0
#define TIME_TO_SIDE_SHOT 1

/* Timer for death animation */
#define DEATH_ANIM  2
#define DEATH_TIMER 3

/* Invulnerability timer after death */
#define INVULN_TIMER 4

/* Inputs */
#define CORESHIP_INPUT_UP       0
#define CORESHIP_INPUT_DOWN     1
#define CORESHIP_INPUT_LEFT     2
#define CORESHIP_INPUT_RIGHT    3
#define CORESHIP_INPUT_SHOOT    4

/* 
 * Loads resources needed for the coreship
 * Note that this has to be called after the video mode has been set,
 * so it is not called by init_all, but rather by the first code that needs
 * the coreship set up
 */
extern int init_coreship (void);

/* Makes the coreship */
extern player make_coreship (void);
                                  
/* Registers inputs needed for the coreship and gives them default binds */
extern void setup_coreship (int id);

/* Updates the coreship */
extern void update_coreship (int id, player *ship);



#endif /* !def CORESHIP_H */
