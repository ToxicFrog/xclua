#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <xchat-plugin.h>
#include <xclua.h>
#include <xclua_callbacks.h>
#include <xclua_hooks.h>

/*	============================================================================
		Register a hook with the global and local states
============================================================================  */
void xclua_register_hook(XCLHook * hook) {
	// get the reference table for the local state
	lua_pushlightuserdata(hook->L, (void *)ph);				/* ... fn ph */
	lua_rawget(hook->L, LUA_REGISTRYINDEX);					/* ... fn ref */
	lua_pushlightuserdata(hook->L, (void *)hook);			/* ... fn ref hook */
	lua_pushvalue(hook->L, -3);								/* ... fn ref hook fn */
	lua_rawset(hook->L, -3);								/* ... fn ref */
	lua_pop(hook->L, 2);									/* ... */
}

/*	============================================================================
		xchat_hook * xchat.hook_command(trigger, function, [help-text], [priority])
============================================================================  */
int xclua_hook_command(lua_State * L) {
	XCLHook * hook = malloc(sizeof(XCLHook));
	hook->L = L;
	
	const char * trigger = luaL_checkstring(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	const char * help_text = luaL_optstring(L, 3, "(no help text for this function)");
	const int priority = luaL_optint(L, 4, XCHAT_PRI_NORM);
	
	hook->hook = xchat_hook_command(ph, trigger, priority, xclua_callback_command, help_text, hook);
	
	lua_pushvalue(L, 2);							/* trig fn ... fn */
	xclua_register_hook(hook);						/* trig fn ... */
	
	lua_pushlightuserdata(L, (void *)hook);			/* trig fn ... hook */
	return 1;										/* ( hook ) */
}

/*	============================================================================
		xchat_hook * xchat.hook_print(trigger, function, [priority])
============================================================================  */
int xclua_hook_print(lua_State * L) {
	XCLHook * hook = malloc(sizeof(XCLHook));
	hook->L = L;
	
	const char * trigger = luaL_checkstring(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	const int priority = luaL_optint(L, 3, XCHAT_PRI_NORM);
	
	hook->hook = xchat_hook_print(ph, trigger, priority, xclua_callback_print, hook);
	
	lua_pushvalue(L, 2);							/* trig fn ... fn */
	xclua_register_hook(hook);						/* trig fn ... */
	
	lua_pushlightuserdata(L, (void *)hook);			/* trig fn ... hook */
	return 1;										/* ( hook ) */
}

/*	============================================================================
		xchat_hook * xchat.hook_server(trigger, function, [priority])
============================================================================  */
int xclua_hook_server(lua_State * L) {
	XCLHook * hook = malloc(sizeof(XCLHook));
	hook->L = L;
	
	const char * trigger = luaL_checkstring(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	const int priority = luaL_optint(L, 3, XCHAT_PRI_NORM);
	
	hook->hook = xchat_hook_server(ph, trigger, priority, xclua_callback_server, hook);
	
	lua_pushvalue(L, 2);							/* trig fn ... fn */
	xclua_register_hook(hook);						/* trig fn ... */
	
	lua_pushlightuserdata(L, (void *)hook);			/* trig fn ... hook */
	return 1;										/* ( hook ) */
}

/*	============================================================================
		xchat_hook * xchat.hook_timer(timeout, function)
============================================================================  */
int xclua_hook_timer(lua_State * L) {
	XCLHook * hook = malloc(sizeof(XCLHook));
	hook->L = L;

	const int trigger = luaL_checkint(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	
	hook->hook = xchat_hook_timer(ph, trigger, xclua_callback_timer, hook);
	
	lua_pushvalue(L, 2);							/* trig fn ... fn */
	xclua_register_hook(hook);						/* trig fn ... */
	
	lua_pushlightuserdata(L, (void *)hook);			/* trig fn ... hook */
	return 1;										/* ( hook ) */
}		

/*	============================================================================
		xchat.unhook(xchat_hook *)
============================================================================  */
int xclua_unhook(lua_State * L) {
	XCLHook * hook = lua_touserdata(L, 1);
	if(hook == NULL) {
		luaL_error(L, "Invalid hook passed to xchat.unhook()");
	}
	
	// verify that the hook actually exists
	// L.registry[ph][hook] ?
	lua_pushlightuserdata(L, (void *)ph);			/* ... ph */
	lua_rawget(L, LUA_REGISTRYINDEX);				/* ... ref */
	lua_pushlightuserdata(L, (void *)hook);			/* ... ref hook */
	lua_rawget(L, -2);								/* ... ref fn */
	if(!lua_toboolean(L, -1)) {
		luaL_error(L, "Attempted to unhook a hook that isn't hooked...so to speak.");
	}
	
	xchat_unhook(ph, hook->hook);
	
	// clear the local reference to the hook
	// L.registry[ph][hook] = nil
	lua_pop(L, 1);									/* ... ref */
	lua_pushlightuserdata(L, (void *)hook);			/* ... ref hook */
	lua_pushnil(L);									/* ... ref hook nil */
	lua_rawset(L, -3);								/* ... ref */
	lua_pop(L, 1);									/* ... */
	
	// free the hook's data structures
	free(hook);
	return 0;
}		
