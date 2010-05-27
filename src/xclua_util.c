#include <lua.h>
#include <lauxlib.h>
#include <string.h>

void * xclua_alloc(lua_State * L, size_t size, lua_CFunction gc, char * type)
{
    void * ptr = lua_newuserdata(L, size);      /* obj */
    memset(ptr, 0, size);
    
    luaL_newmetatable(L, type);                 /* obj mt */ 
    if (gc)
    {
        lua_pushcfunction(L, gc);               /* obj mt __gc */
        lua_setfield(L, -2, "__gc");
    }
    lua_setmetatable(L, -2);
    
    lua_pushlightuserdata(L, ptr);
    lua_pushboolean(L, 1);
    lua_settable(L, LUA_REGISTRYINDEX);

    return ptr;
}

void xclua_free(lua_State * L, void * ptr)
{
    lua_pushlightuserdata(L, ptr);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);
}
