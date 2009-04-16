#include <lua.h>
#include <lauxlib.h>
#include <xchat-plugin.h>
#include <xclua.h>
#include <xclua_callbacks.h>

	/*
	When a callback is invoked, it goes and looks in its userdata for the state and hook;
	then it looks up the function it should call in the state using the hook as the key,
	*/
int xclua_callback(char ** word, void * ud) {
	XCLHook * hook = (XCLHook *)ud;
	
	lua_pushlightuserdata(hook->L, (void *)ph);		/* ... ph */
	lua_rawget(hook->L, LUA_REGISTRYINDEX);			/* ... plugin-table */
	lua_pushlightuserdata(hook->L, (void *)hook);	/* ... plugin-table hook */
	lua_rawget(hook->L, -2);						/* ... plugin-table fn */
	if(lua_isnil(hook->L, -1)) {
		xchat_printf(ph, "[Lua]\tUnable to locate function corresponding to hook 0x%08X", (ptrdiff_t)hook);
		lua_settop(hook->L, 0);
		return XCHAT_EAT_NONE;
	}
	
	int i;
	for(i = 1; word[i][0] != '\0'; ++i) {
		lua_pushstring(hook->L, word[i]);
	}
	
	if(lua_pcall(hook->L, i-1, 1, 0) != 0) {
		xchat_printf(ph, "[Lua]\tError in callback function: %s", lua_tostring(hook->L, -1));
		lua_settop(hook->L, 0);
		return XCHAT_EAT_NONE;
	}
	if(lua_isnumber(hook->L, -1)) {
		int R = lua_tointeger(hook->L, -1);
		lua_settop(hook->L, 0);
		return R;
	}
	xchat_printf(ph, "[Lua]\tError in return from callback: function did not return a number, returning XCHAT_EAT_NONE");
	lua_settop(hook->L, 0);
	return XCHAT_EAT_NONE;
}

int xclua_callback_command(char ** word, char ** word_eol, void * ud) {
	return xclua_callback(word, ud);
}

int xclua_callback_server(char ** word, char ** word_eol, void * ud) {
	return xclua_callback(word, ud);
}

int xclua_callback_timer(void * ud) {
	static char * tmp[2] = { "", "" };
	return xclua_callback(tmp, ud);
}
