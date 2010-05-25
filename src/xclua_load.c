#include <string.h>
#include <stdlib.h>

#include <xchat-plugin.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <xclua.h>
#include <xclua_libs.h>

static void * l_alloc(lua_State * L, size_t size, lua_CFunction gc)
{
    void * ptr = lua_newuserdata(L, size);
    memset(ptr, 0, size);
    
    if (gc)
    {
        lua_newtable(L);
        lua_pushcfunction(L, gc);
        lua_setfield(L, -2, "__gc");
        lua_setmetatable(L, -2);
    }
    luaL_ref(L, LUA_REGISTRYINDEX);
    return ptr;
}

static const char * l_alloc_str(lua_State * L, const char * str)
{
    const char * ptr;
    
    lua_pushstring(L, str);
    ptr = lua_tostring(L, -1);
    luaL_ref(L, LUA_REGISTRYINDEX);
    return ptr;
}

int luaopen_xchat(lua_State * L)
{
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "xchat");
    luaopen_xclua_io(L);
    luaopen_xclua_list(L);
    luaopen_xclua_str(L);
    luaopen_xclua_context(L);
    lua_pop(L, 1);
    
    return 0;
}

static int plugin_gc(lua_State * L)
{
    Plugin * self = lua_touserdata(L, 1);

    /* FIXME: xchat.deinit gc handler */

    if (self->gui)
        xchat_plugingui_remove(ph, self->gui);

    return 0;
}

static Plugin * create_plugin(const char * file)
{
    /* check whether this is already loaded */
    lua_getglobal(xclua_L, file);
    if (lua_type(xclua_L, -1) != LUA_TNIL)
    {
        lua_pop(xclua_L, 1);
        xchat_printf(ph
            , "[lua]\tScript '%s' is already loaded."
            , file);
        return NULL;
    }
    lua_pop(xclua_L, 1);
    
    /* create a VM for this plugin */
    lua_State * L = luaL_newstate();
        
    /* load standard libraries */
    luaL_openlibs(L);
    
    /* FIXME: load xchat libraries */
    luaopen_xchat(L);
    
    /* create a Plugin object around it */
    Plugin * P = l_alloc(L, sizeof(Plugin), plugin_gc);
    P->file = l_alloc_str(L, file);
    P->L = L;
    
    /* load the script */
    if (luaL_loadfile(L, P->file) != 0)
    {
        xchat_printf(ph
            , "[lua]\tError loading '%s': %s"
            , P->file
            , lua_tostring(L, -1));
        lua_close(L);
        return NULL;
    }

    /* call the script */ 
    if (lua_pcall(L, 0, 0, 0) != 0)
    {
        xchat_printf(ph, "[lua]\tError initializing '%s': %s"
            , P->file
            , lua_tostring(L, -1));
        lua_close(L);
        return NULL;
    }

    return P;
}

int xclua_load(char ** word, char ** word_eol, void * userdata)
{
    /* no arguments/not a lua file: fall through to other plugins */
    if (word[2] == NULL
     || strstr(word[2], ".lua") == NULL)
        return XCHAT_EAT_NONE;
    
    Plugin * P = create_plugin(word[2]);
    
    if (!P) return XCHAT_EAT_ALL;
    
    /* call initialization function xchat.init */
    lua_getglobal(P->L, "xchat");
    if (lua_type(P->L, -1) != LUA_TTABLE)
    {
        xchat_printf(ph, "[lua]\tPlugin '%s' removed xchat library during initialization.", P->file);
        lua_close(P->L);
        return XCHAT_EAT_ALL;
    }
    
    lua_getfield(P->L, -1, "init");
    if (lua_type(P->L, -1) != LUA_TFUNCTION)
    {
        xchat_printf(ph, "[lua]\tPlugin '%s' removed xchat.init and failed to provide a replacement.", P->file);
        lua_close(P->L);
        return XCHAT_EAT_ALL;
    }
    
    lua_pushstring(P->L, P->file);
    if (lua_pcall(P->L, 1, 3, 0) != 0)
    {
        xchat_printf(ph, "[lua]\tPlugin '%s' raised an error in xchat.init:", P->file, lua_tostring(P->L, -1));
        lua_close(P->L);
        return XCHAT_EAT_ALL;
    }
        
    P->name = lua_tostring(P->L, -1);
    P->desc = lua_tostring(P->L, -2);
    P->version = lua_tostring(P->L, -3);
        
    if (!(P->name && P->desc && P->version))
    {
        xchat_printf(ph, "[lua]\tPlugin '%s' init function did not return correct values", P->file);
        /* FIXME: print stack contents */
        lua_close(P->L);
        return XCHAT_EAT_ALL;
    }
    
    P->gui = xchat_plugingui_add(ph, P->file, P->name, P->desc, P->version, NULL);
    
    lua_pushlightuserdata(xclua_L, P);
    lua_setglobal(xclua_L, P->file);
    
    return XCHAT_EAT_ALL;
}
