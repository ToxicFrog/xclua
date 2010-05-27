#include <xclua.h>
#include <xclua_hooks.h>
#include <xchat-plugin.h>
#include <lua.h>
#include <lauxlib.h>
#include <xclua_util.h>

static int xclua_callback(Hook * hook, char * name, char ** word, char ** word_eol)
{
    lua_State * L = hook->L;
    lua_rawgeti(L, LUA_REGISTRYINDEX, hook->ref);

    if (word)
    {
        lua_newtable(L); /* word */
        for (size_t i = 1; word[i][0] != '\0'; ++i)
        {
            lua_pushstring(L, word[i]);
            lua_rawseti(L, -2, i);
        }
    }
    
    if (word_eol)
    {
        lua_newtable(L); /* word word_eol */
        for (size_t i = 1; word_eol[i][0] != '\0'; ++i)
        {
            lua_pushstring(L, word_eol[i]);
            lua_rawseti(L, -2, i);
        }
    }
    
    unsigned int ret;
    
    if (lua_pcall(L, 2, 1, 0))
    {
        xchat_printf(ph, "[lua]\tError in callback for %s: %s", name, lua_tostring(L, -1));
        ret = XCHAT_EAT_ALL;
    } else if (lua_type(L, -1) != LUA_TNUMBER
           || (ret = (unsigned int)lua_tonumber(L, -1)) > XCHAT_EAT_ALL) {
        xchat_printf(ph, "[lua]\tCallback for %s did not return a legal xchat.EAT_* value", name);
        ret = XCHAT_EAT_ALL;
    }
    
    lua_settop(L, 0);
    return ret;
}

static int xclua_command_callback(char * word[], char * word_eol[], void * userdata)
{
    Hook * hook = (Hook *)userdata;
    lua_State * L = hook->L;
    lua_rawgeti(L, LUA_REGISTRYINDEX, hook->ref);

    lua_newtable(L); /* word */
    for (size_t i = 1; word[i][0] != '\0'; ++i)
    {
        lua_pushstring(L, word[i]);
        lua_rawseti(L, -2, i);
    }
    
    lua_newtable(L); /* word word_eol */
    for (size_t i = 1; word_eol[i][0] != '\0'; ++i)
    {
        lua_pushstring(L, word_eol[i]);
        lua_rawseti(L, -2, i);
    }
    
    unsigned int ret;
    
    if (lua_pcall(L, 2, 1, 0))
    {
        xchat_printf(ph, "[lua]\tError in callback for %s: %s", word[1], lua_tostring(L, -1));
        ret = XCHAT_EAT_ALL;
    } else if (lua_type(L, -1) != LUA_TNUMBER
           || (ret = (unsigned int)lua_tonumber(L, -1)) > XCHAT_EAT_ALL) {
        xchat_printf(ph, "[lua]\tCallback for %s did not return a legal xchat.EAT_* value", word[1]);
        ret = XCHAT_EAT_ALL;
    }
    
    lua_settop(L, 0);
    return ret;
}

static void xclua_hook_command_validate(lua_State * L, const char ** command, int * pri, const char ** help)
{
    *command = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    
    if (lua_gettop(L) > 2)
    {
        if (lua_type(L, 3) == LUA_TNUMBER)
            *pri = lua_tonumber(L, 3);
        else if (lua_type(L, 3) == LUA_TSTRING)
            *help = lua_tostring(L, 3);
        else
            luaL_typerror(L, 3, "number or string");
    }
    
    if (lua_gettop(L) > 3 && !help)
    {
        *help = luaL_checkstring(L, 4);
    }
}

/* xchat.hook_command(command, fn, [pri], [help]) */
int xclua_hook_command(lua_State * L)
{
    const char * command = NULL;
    const char * help = NULL;
    int pri;
    
    xclua_hook_command_validate(L, &command, &pri, &help);
    
    Hook * hook = xclua_alloc(L, sizeof(*hook), xclua_hook_collect, "xchat_hook");
    hook->L = L;
    lua_pushvalue(L, 2);
    hook->ref = luaL_ref(L, LUA_REGISTRYINDEX);
    hook->hook = xchat_hook_command(ph
        , command
        , pri
        , xclua_command_callback
        , help
        , hook);
    
    return 1;
}
