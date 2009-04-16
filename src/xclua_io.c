#include <lua.h>
#include <lauxlib.h>
#include <xchat-plugin.h>
#include <xclua.h>

int xclua_print(lua_State * L) {
	const char * text = luaL_checkstring(L, 1);
	xchat_print(ph, text);
	return 0;
}

int xclua_command(lua_State * L) {
	const char * text = luaL_checkstring(L, 1);
	xchat_command(ph, text);
	return 0;
}

int xclua_emit_print(lua_State * L)	// FIXME
{
	int argc = (lua_gettop(L) > 9) ? 9 : lua_gettop(L);
	const char * argv[8];
	const char * event;
	
	event = luaL_checkstring(L, 1);
	
	for(int i = 2; i <= argc; ++i)
	{
		argv[i-2] = luaL_optstring(L, i, "");
	}
	
	xchat_emit_print(ph, event,
		argv[0], argv[1], argv[2], argv[3],
		argv[4], argv[5], argv[6], argv[7]);

	return 0;
}

int xclua_send_modes(lua_State * L) // FIXME
{
	
	return 0;
}
