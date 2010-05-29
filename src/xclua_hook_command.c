#include <xclua.h>
#include <xclua_hooks.h>
#include <xchat-plugin.h>
#include <lua.h>
#include <lauxlib.h>
#include <xclua_util.h>

static int xclua_command_callback(char * word[], char * word_eol[], void * userdata)
{
    return xclua_callback((Hook *)userdata, word[1], word, word_eol);
}

static void xclua_hook_command_validate(lua_State * L, const char ** command, int * pri, const char ** help)
{
    *command = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    
    if (lua_gettop(L) > 2)
    {
        if (lua_type(L, 3) == LUA_TUSERDATA)
            *pri = (int)luaL_checkudata(L, 3, "xchat_priority");
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
    int priority = XCHAT_PRI_NORM;

    xclua_hook_command_validate(L, &command, &priority, &help);
    
    Hook * hook = xclua_alloc(L, sizeof(*hook), xclua_hook_collect, "xchat_hook");
    hook->L = L;
    lua_pushvalue(L, 2);
    hook->ref = luaL_ref(L, LUA_REGISTRYINDEX);
    hook->hook = xchat_hook_command(ph
        , command
        , priority
        , xclua_command_callback
        , help
        , hook);
    
    return 1;
}
