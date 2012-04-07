/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * scripts.c
 * Contains code for contextualizing and running scripts loaded by the engine.
 */

#include "debug.h"
#include "scrfuncs.h"
#include "scripts.h"
#include "./lua/lua.h"
#include "./lua/lauxlib.h"
#include "./lua/lualib.h"

static lua_State *L_main;

resource *runner  = NULL;
resource *header  = NULL;
resource *runmain = NULL;

/* Simple setter scripts */
void set_runner(resource *res)
{
    runner = res;
}
void set_header(resource *res)
{
    header = res;
}
void set_main(resource *res)
{
    runmain = res;
}

/*
 * The reader function for lua_load
 * Because we want to essentially piece three files together, we take
 * advantage of how the lua_Reader is set up.
 * Each time the reader is called, we switch to the next piece.
 */
int done_reading = FALSE;
static const char *reader(lua_State *L, void *data, size_t *size)
{
    resource *res = (resource*) data;
    
    /* Test for null resource */
    if (res == NULL) {
        *size = 0;
        return NULL;
    }
    
    /* Test if we've already read it */
    if (done_reading) {
        done_reading = FALSE;
        *size = 0;
        return NULL;
    }
    else {
        done_reading = TRUE;
        *size = (size_t)(res->size);
        return (const char*)(res->data);
    }
}

/* Loads the files into the Lua state */
void load_scripts(void)
{
    int r;
    
    r = lua_load(L_main, reader, (void*)runner , "<<runner>>"  , "bt");
    check_lua_error(r == LUA_OK, L_main);
    
    r = lua_pcall(L_main, 0, 0, 0);
    check_lua_error(r == LUA_OK, L_main);
    
    r = lua_load(L_main, reader, (void*)header , "<<header>>"  , "bt");
    check_lua_error(r == LUA_OK, L_main);
    
    r = lua_pcall(L_main, 0, 0, 0);
    check_lua_error(r == LUA_OK, L_main);
    
    r = lua_load(L_main, reader, (void*)runmain, "<<runmain>>" , "bt");
    check_lua_error(r == LUA_OK, L_main);
    
    r = lua_pcall(L_main, 0, 0, 0);
    check_lua_error(r == LUA_OK, L_main);
    
    lua_getglobal(L_main, "init_table");
    r = lua_pcall(L_main, 0, 0, 0);
    check_lua_error(r == LUA_OK, L_main);
}

/* Resets the script */
void reset_scripts(void)
{
    /* Close and recreate the Lua state */
    stop_scripts();
    init_scripts();
    
    /* Reload all of the files */
    load_scripts();
}

/* A shortcut for running the exec_bullet_scripts function in Lua. */
void exec_bullet_scripts(void)
{
    int r;
    
    lua_getglobal(L_main, "exec_bullet_scripts");
    r = lua_pcall(L_main, 0, 0, 0);
    check_lua_error(r == LUA_OK, L_main);
}

/* A shortcut for calling the add_bullet function in Lua. */
void add_bullet(int bid, const char *func)
{
    int r;
    
    lua_getglobal(L_main, "add_bullet");
    lua_pushinteger(L_main, bid);
    lua_pushstring(L_main, func);
    r = lua_pcall(L_main, 2, 0, 0);
    check_lua_error(r == LUA_OK, L_main);
}

/* A shortcut for calling the set_stage function in Lua. */
void set_stage(const char *func)
{
    int r;
    
    lua_getglobal(L_main, "set_stage");
    lua_pushstring(L_main, func);
    r = lua_pcall(L_main, 1, 0, 0);
    check_lua_error(r == LUA_OK, L_main);
}

/* Initializes L_main */
void init_scripts()
{
    L_main = luaL_newstate();
    luaL_openlibs(L_main);
    
    init_library();
    luaopen_bulletrain(L_main);
}

/* Closes L_main */
void stop_scripts()
{
    lua_close(L_main);
}
