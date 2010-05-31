#include <lua.h>
#include <lauxlib.h>
#include <string.h>

void * xclua_alloc(lua_State * L, size_t size, char * type)
{
    void * ptr = lua_newuserdata(L, size);      /* obj */
    memset(ptr, 0, size);
    
    luaL_newmetatable(L, type);                 /* obj mt */ 
    lua_setmetatable(L, -2);                    /* obj */
    
    lua_pushlightuserdata(L, ptr);              /* obj _obj_ */
    lua_pushvalue(L, -2);                       /* obj _obj_ obj */
    lua_settable(L, LUA_REGISTRYINDEX);         /* obj */

    return ptr;
}

void xclua_free(lua_State * L, void * ptr)
{
    lua_pushlightuserdata(L, ptr);      /* _obj_ */
    lua_pushnil(L);                     /* _obj_ nil */
    lua_settable(L, LUA_REGISTRYINDEX); /* */
}
