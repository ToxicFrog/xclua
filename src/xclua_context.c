#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <xchat-plugin.h>
#include <xclua.h>
#include <xclua_context.h>

// xchat_find_context(server, channel)
int xclua_find_context(lua_State * L) {
	const char * server = luaL_optstring(L, 1, NULL);
	const char * channel = luaL_optstring(L, 2, NULL);
	xchat_context * context = xchat_find_context(ph, server, channel);
	lua_pushlightuserdata(L, context);
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

int xclua_get_info(lua_State * L) {
	int argc = lua_gettop(L);
	lua_checkstack(L, argc +2);
	
	for(int i = 0; i < argc; ++i)
	{
		const char * key = luaL_checkstring(L, 1);
		lua_remove(L, 1);
		
		if( !strcmp(key, "xclua_version") )
		{
			lua_pushstring(L, PVERSION);
		} else if( !strcmp(key, "lua_version") ) {
			lua_pushstring(L, LUA_VERSION);
		} else {
			lua_pushstring(L, xchat_get_info(ph, key));
		}
	}
	
	return argc;
}

int xclua_get_prefs(lua_State * L) {
	int argc = lua_gettop(L);
	lua_checkstack(L, argc +2);
	
	for(int i = 0; i < argc; ++i)
	{
		const char * key = luaL_checkstring(L, 1);
		lua_remove(L, 1);

		int ival;
		const char * sval;

		switch(xchat_get_prefs(ph, key, &sval, &ival))
		{
		  case 1:	// returned a string
		  	lua_pushstring(L, sval);
			break;
		  case 2:	// returned an integer
		  	lua_pushinteger(L, ival);
			break;
		  case 3:	// returned a boolean
		  	lua_pushboolean(L, ival);
			break;
		  case 0:	// failure - fallthrough
		  default:
			lua_pushnil(L);
			break;
		}
	}
	return argc;
}
