#include <xclua.h>
#include <xclua_hooks.h>
#include <xchat-plugin.h>
#include <lua.h>
#include <lauxlib.h>
#include <xclua_util.h>

static int xclua_print_callback(char ** word, void * userdata)
{
    return xclua_callback((Hook *)userdata, "<timer>", word, NULL);
}

/* xchat.hook_print(name, fn, priority) */
int xclua_hook_print(lua_State * L)
{
    const char * name = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    int priority = XCHAT_PRI_NORM;

    if (lua_gettop(L) > 2)
        priority = (int)luaL_checkudata(L, 3, "xchat_priority");
    
    Hook * hook = xclua_alloc(L, sizeof(*hook), xclua_hook_collect, "xchat_hook");
    hook->L = L;
    lua_pushvalue(L, 2);
    hook->ref = luaL_ref(L, LUA_REGISTRYINDEX);
    hook->hook = xchat_hook_print(ph
        , name
        , priority
        , xclua_print_callback
        , hook);
    
    return 1;
}
