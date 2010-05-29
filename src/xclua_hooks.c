#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <xchat-plugin.h>
#include <xclua.h>
#include <xclua_util.h>
#include <xclua_hooks.h>

static int xclua_unhook(lua_State * L)
{
    Hook * hook = (Hook *)luaL_checkudata(L, 1, "xchat_hook");
    xchat_unhook(ph, hook->hook);
    hook->hook = NULL;
    xclua_free(L, hook);
    return 0;
}

int xclua_hook_collect(lua_State * L)
{
    Hook * hook = (Hook *)luaL_checkudata(L, 1, "xchat_hook");
    if (hook->hook)
        xchat_unhook(ph, hook->hook);
    return 0;
}

void luaopen_xclua_hooks(lua_State * L)
{
    lua_pushcfunction(L, xclua_hook_command);
    lua_setfield(L, -2, "hook_command");
    lua_pushcfunction(L, xclua_hook_timer);
    lua_setfield(L, -2, "hook_timer");
    lua_pushcfunction(L, xclua_hook_server);
    lua_setfield(L, -2, "hook_server");

    lua_pushcfunction(L, xclua_unhook);
    lua_setfield(L, -2, "unhook");
    
    int * tmp;
    
    #define pusheat(eat, name) \
        tmp = lua_newuserdata(L, sizeof(int)); \
        *tmp = eat; \
        luaL_newmetatable(L, "xchat_eat"); \
        lua_setmetatable(L, -2); \
        lua_setfield(L, -2, name);
    
    pusheat(XCHAT_EAT_NONE, "EAT_NONE");
    pusheat(XCHAT_EAT_PLUGIN, "EAT_PLUGIN");
    pusheat(XCHAT_EAT_XCHAT, "EAT_XCHAT");
    pusheat(XCHAT_EAT_ALL, "EAT_ALL");
    #undef pusheat
    
    #define pushpri(pri, name) \
        tmp = lua_newuserdata(L, sizeof(int)); \
        *tmp = pri; \
        luaL_newmetatable(L, "xchat_priority"); \
        lua_setmetatable(L, -2); \
        lua_setfield(L, -2, name);
    
    pushpri(XCHAT_PRI_HIGHEST, "PRI_HIGHEST");
    pushpri(XCHAT_PRI_HIGH, "PRI_HIGH");
    pushpri(XCHAT_PRI_NORM, "PRI_NORM");
    pushpri(XCHAT_PRI_LOW, "PRI_LOW");
    pushpri(XCHAT_PRI_LOWEST, "PRI_LOWEST");
    #undef pushpri
}

int xclua_callback(Hook * hook, char * name, char ** word, char ** word_eol)
{
    lua_State * L = hook->L;
    lua_rawgeti(L, LUA_REGISTRYINDEX, hook->ref);
    size_t argc = 0;

    if (word)
    {
        ++argc;
        lua_newtable(L); /* word */
        for (size_t i = 1; word[i][0] != '\0'; ++i)
        {
            lua_pushstring(L, word[i]);
            lua_rawseti(L, -2, i);
        }
    }
    
    if (word_eol)
    {
        ++argc;
        lua_newtable(L); /* word word_eol */
        for (size_t i = 1; word_eol[i][0] != '\0'; ++i)
        {
            lua_pushstring(L, word_eol[i]);
            lua_rawseti(L, -2, i);
        }
    }
    
    int ret;
    
    if (lua_pcall(L, argc, 1, 0))
    {
        xchat_printf(ph, "[lua]\tError in callback for %s: %s", name, lua_tostring(L, -1));
        ret = XCHAT_EAT_ALL;
    } else {
        lua_getmetatable(L, -1);
        luaL_newmetatable(L, "xchat_eat");
        if (!lua_equal(L, -1, -2))
        {
            xchat_printf(ph, "[lua]\tCallback for %s did not return a legal xchat.EAT_* value", name);
            ret = XCHAT_EAT_ALL;
        } else {
            ret = *(int *)lua_touserdata(L, -3);
        }
    }
    
    lua_settop(L, 0);
    return ret;
}
