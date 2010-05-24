#include <lua.h>
#include <lauxlib.h>
#include <xchat-plugin.h>
#include <xclua.h>

// xchat.nickcmp(lhs, rhs)
static int xclua_nickcmp(lua_State * L) {
	const char * lhs = luaL_checkstring(L, 1);
	const char * rhs = luaL_checkstring(L, 2);
	int R = xchat_nickcmp(ph, lhs, rhs);
	lua_pushinteger(L, R);
	return 1;
}

// xchat.strip(str, [flags])
static int xclua_strip(lua_State * L) {
	const char * str = luaL_checkstring(L, 1);
	int flags = luaL_optint(L, 2, 3);	// 3 == xchat.STRIP_COLOR + xchat.STRIP_FORMAT
	
	char * stripped = xchat_strip(ph, str, -1, flags);
	lua_pushstring(L, stripped);
	xchat_free(ph, stripped);
	return 1;
}

void luaopen_xclua_str(lua_State * L)
{
    lua_pushcfunction(L, xclua_nickcmp);
    lua_setfield(L, -2, "nickcmp");
    lua_pushcfunction(L, xclua_strip);
    lua_setfield(L, -2, "strip");
}
