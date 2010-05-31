#include <xclua.h>
#include <xclua_hooks.h>
#include <xchat-plugin.h>
#include <lua.h>
#include <lauxlib.h>
#include <xclua_util.h>
#include <stdlib.h>
#include <string.h>

static int xclua_timer_callback(void * userdata)
{
    return xclua_callback((Hook *)userdata, NULL, NULL);
}

/* xchat.hook_timer(timeout, callback) */
int xclua_hook_timer(lua_State * L)
{
    int timeout = luaL_checkinteger(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    
    Hook * hook = xclua_alloc(L, sizeof(*hook), "xchat_hook");
    hook->L = L;
    
    lua_pushfstring(L, "timer (%p)", hook);
    hook->name = malloc(lua_objlen(L, -1)+1);
    strcpy(hook->name, lua_tostring(L, -1));
    lua_pop(L, 1);
    
    lua_pushvalue(L, 2);
    hook->ref = luaL_ref(L, LUA_REGISTRYINDEX);
    hook->hook = xchat_hook_timer(ph
        , timeout
        , xclua_timer_callback
        , hook);
    
    return 1;
}
