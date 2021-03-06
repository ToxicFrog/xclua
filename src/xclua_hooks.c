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
    
    if (!hook->hook)
    {
        /* we have a problem - a double unhook */
        /* for now we just emit a warning */
        xchat_printf(ph, "[lua]\tWarning: attempt to unhook %s, which is already unhooked\n", hook->name);
    }
        
    xchat_unhook(ph, hook->hook);
    hook->hook = NULL;
    
    /* it is now subject to garbage collection if they aren't holding on to it */
    xclua_free(L, hook);
    return 0;
}

static int xclua_hook_gc(lua_State * L)
{
    Hook * hook = (Hook *)luaL_checkudata(L, 1, "xchat_hook");
    
    if (hook->hook)
        xchat_unhook(ph, hook->hook);
    if (hook->name)
        free(hook->name);
    
    return 0;
}

static int xclua_hook_tostring(lua_State * L)
{
    Hook * hook = luaL_checkudata(L, 1, "xchat_hook");
    lua_pushfstring(L, "xchat_hook: %s", hook->name);
    return 1;
}

void luaopen_xclua_hooks(lua_State * L)
{
    /* xchat.hook_* functions */
    lua_pushcfunction(L, xclua_hook_command);
    lua_setfield(L, -2, "hook_command");
    lua_pushcfunction(L, xclua_hook_timer);
    lua_setfield(L, -2, "hook_timer");
    lua_pushcfunction(L, xclua_hook_server);
    lua_setfield(L, -2, "hook_server");
    lua_pushcfunction(L, xclua_hook_print);
    lua_setfield(L, -2, "hook_print");

    /* xchat.unhook */
    lua_pushcfunction(L, xclua_unhook);
    lua_setfield(L, -2, "unhook");
    
    /* metatable for xchat_hook type */
    luaL_newmetatable(L, "xchat_hook");
    lua_pushcfunction(L, xclua_hook_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, xclua_hook_tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pop(L, 1);
    
    int * tmp;
    
    /* XCHAT_EAT_* constants */
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
    
    /* XCHAT_PRI_* constants */
    #define pushpri(pri, name) \
        tmp = lua_newuserdata(L, sizeof(int)); \
        *tmp = pri; \
        luaL_newmetatable(L, "xchat_priority"); \
        lua_setmetatable(L, -2); \
        lua_setfield(L, -2, name);
    
    pushpri(XCHAT_PRI_HIGHEST, "PRI_HIGHEST");
    pushpri(XCHAT_PRI_HIGH, "PRI_HIGH");
    pushpri(XCHAT_PRI_NORM, "PRI_NORMAL");
    pushpri(XCHAT_PRI_LOW, "PRI_LOW");
    pushpri(XCHAT_PRI_LOWEST, "PRI_LOWEST");
    #undef pushpri
}

int xclua_callback(Hook * hook, char ** word, char ** word_eol)
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
        xchat_printf(ph, "[lua]\tError in callback for %s: %s\n", hook->name, lua_tostring(L, -1));
        ret = XCHAT_EAT_ALL;
    } else {
        lua_getmetatable(L, -1);
        luaL_newmetatable(L, "xchat_eat");
        if (!lua_equal(L, -1, -2))
        {
            xchat_printf(ph, "[lua]\tCallback for %s did not return a legal xchat.EAT_* value\n", hook->name);
            ret = XCHAT_EAT_ALL;
        } else {
            ret = *(int *)lua_touserdata(L, -3);
        }
    }
    
    lua_settop(L, 0);
    return ret;
}
