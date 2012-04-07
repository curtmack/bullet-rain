/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * scrfuncs.h
 * Contains function prototypes for implementation of Lua functions
 */

#ifndef SCRFUNCS_H

#define SCRFUNCS_H

#include "./lua/lua.h"
#include "./lua/lauxlib.h"

extern int luaopen_bulletrain (lua_State *L);
extern int init_library();

#endif /* !def SCRFUNCS_H */
