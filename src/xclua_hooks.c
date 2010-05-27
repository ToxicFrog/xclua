#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <xchat-plugin.h>
#include <xclua.h>
#include <xclua_util.h>
#include <xclua_hooks.h>

static int xclua_unhook(lua_State * L)
{
    Hook * hook = (Hook *)luaL_checkudata(L, 1, "xchat_hook");
    xchat_unhook(ph, hook->hook);
    hook->hook = NULL;
    xclua_free(L, hook);
    return 0;
}

int xclua_hook_collect(lua_State * L)
{
    Hook * hook = (Hook *)luaL_checkudata(L, 1, "xchat_hook");
    if (hook->hook)
        xchat_unhook(ph, hook->hook);
    return 0;
}

void luaopen_xclua_hooks(lua_State * L)
{
    lua_pushcfunction(L, xclua_hook_command);
    lua_setfield(L, -2, "hook_command");

    lua_pushcfunction(L, xclua_unhook);
    lua_setfield(L, -2, "unhook");
    
    lua_pushnumber(L, XCHAT_EAT_NONE);
    lua_setfield(L, -2, "EAT_NONE");
    lua_pushnumber(L, XCHAT_EAT_PLUGIN);
    lua_setfield(L, -2, "EAT_PLUGIN");
    lua_pushnumber(L, XCHAT_EAT_XCHAT);
    lua_setfield(L, -2, "EAT_XCHAT");
    lua_pushnumber(L, XCHAT_EAT_ALL);
    lua_setfield(L, -2, "EAT_ALL");
}

int xclua_callback(Hook * hook, char * name, char ** word, char ** word_eol)
{
    lua_State * L = hook->L;
    lua_rawgeti(L, LUA_REGISTRYINDEX, hook->ref);

    if (word)
    {
        lua_newtable(L); /* word */
        for (size_t i = 1; word[i][0] != '\0'; ++i)
        {
            lua_pushstring(L, word[i]);
            lua_rawseti(L, -2, i);
        }
    }
    
    if (word_eol)
    {
        lua_newtable(L); /* word word_eol */
        for (size_t i = 1; word_eol[i][0] != '\0'; ++i)
        {
            lua_pushstring(L, word_eol[i]);
            lua_rawseti(L, -2, i);
        }
    }
    
    unsigned int ret;
    
    if (lua_pcall(L, 2, 1, 0))
    {
        xchat_printf(ph, "[lua]\tError in callback for %s: %s", name, lua_tostring(L, -1));
        ret = XCHAT_EAT_ALL;
    } else if (lua_type(L, -1) != LUA_TNUMBER
           || (ret = (unsigned int)lua_tonumber(L, -1)) > XCHAT_EAT_ALL) {
        xchat_printf(ph, "[lua]\tCallback for %s did not return a legal xchat.EAT_* value", name);
        ret = XCHAT_EAT_ALL;
    }
    
    lua_settop(L, 0);
    return ret;
}


#if 0
/*	============================================================================
		xchat_hook * xchat.hook_command(trigger, function, [help-text], [priority])
============================================================================  */
int xclua_hook_command(lua_State * L) {
	const char * trigger = luaL_checkstring(L, 1);
	luaL_checktype(L, 2, LUA_TFUNCTION);
	const char * help_text = luaL_optstring(L, 3, "(no help text for this function)");
	const int priority = luaL_optint(L, 4, XCHAT_PRI_NORM);
	
	XCLHook * hook = malloc(sizeof(XCLHook));
	hook->L = L;
	
	hook->hook = xchat_hook_command(ph, trigger, priority, xclua_callback_command, help_text, hook);
	
	lua_pushvalue(L, 2);							/* trig fn ... fn */
	xclua_register_hook(hook);						/* trig fn ... */
	
	lua_pushlightuserdata(L, (void *)hook);			/* trig fn ... hook */
	return 1;										/* ( hook ) */
}

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
#endif
