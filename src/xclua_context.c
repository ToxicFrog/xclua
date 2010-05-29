#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <xchat-plugin.h>
#include <xclua.h>

// xchat_find_context(server, channel)
int xclua_find_context(lua_State * L) {
	const char * server = luaL_optstring(L, 1, NULL);
	const char * channel = luaL_optstring(L, 2, NULL);
	xchat_context * context = xchat_find_context(ph, server, channel);
    if (context)
        lua_pushlightuserdata(L, context);
    else
        lua_pushnil(L);
	return 1;
}

int xclua_get_context(lua_State * L) {
	xchat_context * context = xchat_get_context(ph);
	lua_pushlightuserdata(L, (void *)context);
	return 1;
}

int xclua_set_context(lua_State * L) {
	xchat_context * context = (xchat_context *)lua_touserdata(L, 1);
	
    if(context == NULL) {
		luaL_error(L, "set_context() argument 1: expected light userdata, got nothing");
	}
    
	if(xchat_set_context(ph, context)) {
		lua_pushboolean(L, 1);
	} else {
		lua_pushboolean(L, 0);
	}
    
	return 1;
}

void luaopen_xclua_context(lua_State * L)
{
    lua_pushcfunction(L, xclua_find_context);
    lua_setfield(L, -2, "find_context");
    lua_pushcfunction(L, xclua_get_context);
    lua_setfield(L, -2, "get_context");
    lua_pushcfunction(L, xclua_set_context);
    lua_setfield(L, -2, "set_context");
}
