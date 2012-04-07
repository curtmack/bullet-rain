/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * scripts.h
 * Contains function prototypes for contextualizing and running scripts
 * loaded by the engine.
 */

#ifndef SCRIPTS_H

#define SCRIPTS_H

#include "resource.h"
#include "./lua/lua.h"
#include "./lua/lauxlib.h"

extern void set_runner(resource *res);
extern void set_header(resource *res);
extern void set_main(resource *res);

extern void load_scripts(void);
extern void reset_scripts(void);

extern void exec_bullet_scripts(void);
extern void add_bullet(int bid, const char *func);
extern void set_stage(const char *func);

extern void init_scripts(void);
extern void stop_scripts(void);

#define check_lua_error(good,luastate) \
    warn2(good, "Lua error:", (char*) luaL_checkstring(luastate, -1))

#endif /* !def SCRIPTS_H */
