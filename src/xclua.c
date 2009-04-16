#include <string.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <xchat-plugin.h>
#include <xclua.h>
#include <xclua_callbacks.h>
#include <xclua_context.h>
#include <xclua_hooks.h>
#include <xclua_io.h>
#include <xclua_list.h>
#include <xclua_misc.h>
#include <stdio.h>

#include <xclua.lb>

#ifdef DEBUG
static FILE * fout;
#endif

/*	============================================================================
		Globals
============================================================================  */
xchat_plugin * ph;	/* plugin handle */
lua_State * master;	/* master state */

void luaopen_xchat(lua_State * L) {
	static luaL_Reg xclua_funcs[] = {
	  // from xclua_context
	  	{ "find_context",		xclua_find_context },
		{ "get_context",		xclua_get_context },
		{ "set_context",		xclua_set_context },
		{ "get_prefs",			xclua_get_prefs },
		{ "get_info",			xclua_get_info },
	  // from xclua_hooks
		{ "hook_command",		xclua_hook_command },
//		{ "hook_fd",			Not implemented
		{ "hook_print",			xclua_hook_print },
		{ "hook_server",		xclua_hook_server },
		{ "hook_timer",			xclua_hook_timer },
		{ "unhook", 			xclua_unhook },
	  // from xclua_io
		{ "print",				xclua_print },
//		{ "printf", 			Implemented in Lua
		{ "command",			xclua_command },
//		{ "commandf",			Implemented in Lua
		{ "emit_print", 		xclua_emit_print },
		{ "send_modes", 		xclua_send_modes },
	  // from xclua_list
		{ "list_get",			xclua_list_get },
	  // from xclua_misc
		{ "nickcmp",			xclua_nickcmp },
		{ "strip",				xclua_strip },
		{ NULL, NULL }
	};
	luaL_openlibs(L);
	luaL_register(L, "xchat", xclua_funcs);

	// callback return codes
	lua_pushinteger(L, XCHAT_EAT_NONE);
	lua_setfield(L, -2, "EAT_NONE");
	lua_pushinteger(L, XCHAT_EAT_PLUGIN);
	lua_setfield(L, -2, "EAT_PLUGIN");
	lua_pushinteger(L, XCHAT_EAT_XCHAT);
	lua_setfield(L, -2, "EAT_XCHAT");
	lua_pushinteger(L, XCHAT_EAT_ALL);
	lua_setfield(L, -2, "EAT_ALL");
	// xchat_strip() modes
	lua_pushinteger(L, 1);
	lua_setfield(L, -2, "STRIP_COLOR");
	lua_pushinteger(L, 1);
	lua_setfield(L, -2, "STRIP_COLOUR");
	lua_pushinteger(L, 2);
	lua_setfield(L, -2, "STRIP_FORMAT");
	// priority levels
	lua_pushinteger(L, XCHAT_PRI_HIGHEST);
	lua_setfield(L, -2, "PRI_HIGHEST");
	lua_pushinteger(L, XCHAT_PRI_HIGH);
	lua_setfield(L, -2, "PRI_HIGH");
	lua_pushinteger(L, XCHAT_PRI_NORM);
	lua_setfield(L, -2, "PRI_NORM");
	lua_pushinteger(L, XCHAT_PRI_LOW);
	lua_setfield(L, -2, "PRI_LOW");
	lua_pushinteger(L, XCHAT_PRI_LOWEST);
	lua_setfield(L, -2, "PRI_LOWEST");

	lua_pop(L, 1);

	lua_pushlightuserdata(L, (void *)ph);
	lua_newtable(L);
	lua_rawset(L, LUA_REGISTRYINDEX);
	
	// lua-implemented functions
	luaL_loadbuffer(L, xclua_lua_buffer, xclua_lua_buflen, "XCLua lua-implemented functions");
	lua_pcall(L, 0, 0, 0);
	
	return;
}

int __export xchat_plugin_init(
				xchat_plugin *plugin_handle,
				char **plugin_name,
				char **plugin_desc,
				char **plugin_version,
				char *arg)
{
	ph = plugin_handle;

	*plugin_name = PNAME;
	*plugin_desc = PDESC;
	*plugin_version = PVERSION;

	master = luaL_newstate();
	luaopen_xchat(master);

	xchat_hook_command(ph, "load", XCHAT_PRI_NORM, xclua_load, NULL, NULL);
	xchat_hook_command(ph, "unload", XCHAT_PRI_NORM, xclua_unload, NULL, NULL);

	xchat_printf(ph, "[Lua]\tExecuting lua-onload script, please ignore errors (if any)...");
	xchat_commandf(ph, "load plugins/lua-onload.lua");
	xchat_printf(ph, "[Lua]\tLoaded.");

	// FIXME - iterate over all Lua files in the plugins dir and load them

#ifdef DEBUG
	fout = fopen("xclua_log.txt", "w");
#endif

	return 1;
}

int __export xchat_plugin_deinit() {
	// first, we need to iterate over all loaded plugins and unload them
	// then we clear our own hooks
	// finally we delete our state and return
	
	lua_pushlightuserdata(master, (void *)ph);
	lua_rawget(master, LUA_REGISTRYINDEX);
	lua_pushnil(master);
	while(lua_next(master, -2)) {
		lua_pop(master, 1);
		const char * filename = lua_tostring(master, -1);
		TRACE("FILE %s\n", filename)
		static const char * word[4];
		word[0] = "";
		word[1] = "";
		word[2] = filename;
		word[3] = "";
		xclua_unload((char **)word, (char **)word, NULL);
	}
	
	TRACE("killing master state\n");
	// now we should have no open plugins. If some of them couldn't unload that's just too bad
	lua_close(master);
	
	xchat_printf(ph, "[Lua]\tUnloaded.");

#ifdef DEBUG
	fclose(fout);
#endif

	return 1;
}

/**
	Windows doesn't have basename, and POSIX basename is terrifyingly dangerous in
	any case, so we roll our own
**/
const char * xclua_basename(const char * pathname) {
	const char * tmp = strrchr(pathname, '/');
#ifdef WIN32
	if(tmp == NULL) tmp = strrchr(pathname, '\\');
#endif
	if(tmp == NULL)
		return pathname;
	return tmp+1;
}

/**
	Based on a filename, make a snap judgement about whether it is or is not a Lua script
**/
int xclua_is_script(const char * file) {
	const char * tmp = strrchr(file, '.');
	if(tmp == NULL)
		return 0;
	if(strcmp(tmp, ".lua") && strcmp(tmp, ".lc"))
		return 0;
	return 1;
}

int xclua_load(char ** word, char ** word_eol, void * ud) {
	const char * file = xclua_basename(word_eol[2]);
	if(!xclua_is_script(file))
		return XCHAT_EAT_NONE;
	
	lua_pushlightuserdata(master, (void *)ph);	/* ph */
	lua_rawget(master, LUA_REGISTRYINDEX);		/* ref */
	lua_pushstring(master, file);				/* ref file */
	lua_rawget(master, -2); 					/* ref xref */
	if(!lua_isnil(master, -1)) {
		xchat_printf(ph, "[Lua]\tError: script is already loaded, use /unload first");
		lua_pop(master, 2);
		return XCHAT_EAT_ALL;
	}
	lua_pop(master, 1);							/* ref */
	
	lua_State * L = luaL_newstate();
	luaopen_xchat(L);
	
	if(luaL_loadfile(L, word_eol[2])
	|| lua_pcall(L, 0, 1, 0)) {
		xchat_printf(ph, "[Lua]\tError loading script: %s", lua_tostring(L, -1));
		lua_close(L);
		lua_pop(master, 1);
		return XCHAT_EAT_ALL;
	}
	
	if(!lua_toboolean(L, 1)) {
		// FIXME - we need to clean up any hooks it's laid here
		xchat_printf(ph, "[Lua]\tError loading script: initialization function returned false");
		lua_close(L);
		lua_pop(master, 1);
		return XCHAT_EAT_ALL;
	}
	
	// get the script information
	const char * script_name = file;
	const char * script_desc = "(no description provided)";
	const char * script_ver = "(unknown)";
	lua_settop(L, 0);
	lua_getglobal(L, "plugin_name");
	if(lua_isstring(L, 1)) {
		script_name = lua_tostring(L, 1);
	}
	lua_getglobal(L, "plugin_description");
	if(lua_isstring(L, 2)) {
		script_desc = lua_tostring(L, 2);
	}
	lua_getglobal(L, "plugin_version");
	if(lua_isstring(L, 3)) {
		script_ver = lua_tostring(L, 3);
	}
	
	// if we make it this far, we were able to successfully load and execute the script
	// so, we now have some additional bookkeeping to do: the master state entries and
	// GUI entry
	void * plugingui = xchat_plugingui_add(ph,
				file,
				script_name,
				script_desc,
				script_ver,
				NULL);
	
	lua_settop(L, 0);
	
	lua_pushstring(master, file);				/* ref filename */
	lua_newtable(master);						/* ref filename {} */
	lua_pushinteger(master, 1); 				/* ref filename {} 1 */
	lua_pushlightuserdata(master, (void *)L);	/* ref filename {} 1 L */
	lua_rawset(master, -3); 					/* ref filename { L } */
	lua_pushinteger(master, 2); 				/* ref filename { L } 2 */
	lua_pushlightuserdata(master, plugingui);	/* ref filename { L } 2 gui */
	lua_rawset(master, -3); 					/* ref filename { L, gui } */
	lua_rawset(master, -3); 					/* ref */
	lua_pop(master, 1);
	
	return XCHAT_EAT_ALL;
}

int xclua_unload(char ** word, char ** word_eol, void * ud) {
	const char * file = xclua_basename(word_eol[2]);
	// if the script isn't loaded (check master table) return
	// get the script's state from the master script table
	// call the script's deinit function and abort if it refuses
	// get the script's hook table from its state and unhook and free all associated hooks
	// xchat_plugingui_remove() the script's GUI entry
	// clear the script's master entry
	// delete the script's state
	
	lua_pushlightuserdata(master, (void *)ph);	/* ... ph */
	lua_rawget(master, LUA_REGISTRYINDEX);		/* ... ref */
	lua_pushstring(master, file);				/* ... ref file */
	lua_rawget(master, -2); 					/* ... ref xref */
	if(lua_isnil(master, -1)) {
		lua_pop(master, 2);
		return XCHAT_EAT_NONE;	// xchat will take care of the error message for us
	}

	// get the script's local state
	lua_pushinteger(master, 1); 				/* ... ref xref 1 */
	lua_rawget(master, -2); 					/* ... ref xref L */
	lua_State * L = lua_touserdata(master, -1); /* ... ref xref L */
	TRACE("unload: local state: %08X\n", (ptrdiff_t)L);
	
	lua_getglobal(L, "plugin_deinit");
	if(lua_type(L, -1) == LUA_TFUNCTION) {
		TRACE("got deinit\n");
		if(lua_pcall(L, 0, 1, 0) == 0) {
			TRACE("pcall succeeded\n");
			if(!lua_toboolean(L, -1)) {
				xchat_printf(ph, "[Lua]\tPlugin is refusing to unload; aborting");
				lua_pop(master, 3);
				lua_settop(L, 0);
				return XCHAT_EAT_ALL;
			}
			// else it completed successfully
		} else {
			xchat_printf(ph, "[Lua]\tWarning: plugin_deinit() threw error (%s), forcing unload", lua_tostring(L, -1));
		}
	}
	TRACE("done deinit\n");
	lua_pop(L, 1);

	// at this point, the plugin's deinit function has completed successfully
	// now we need to clear all its hooks
	lua_pushlightuserdata(L, (void *)ph);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushnil(L);
	TRACE("iterator\n");
	while(lua_next(L, -2)) {
		lua_pop(L, 1);
		XCLHook * hook = (XCLHook *)lua_touserdata(L, -1);
		TRACE("unhook: hook: %08X\n", (ptrdiff_t)hook);
		TRACE("unhook: hook pointers: %08X %08X\n", (ptrdiff_t)hook->hook, (ptrdiff_t)hook->L);
		xchat_unhook(ph, hook->hook);
		free(hook);
	}
	
	// hooks have been cleared; remove it from the GUI
	lua_pushinteger(master, 2); 				/* ... ref xref L 2 */
	lua_rawget(master, -3); 					/* ... ref xref L gui */
	TRACE("ungui: gui handle: %08X\n", (ptrdiff_t)lua_touserdata(master, -1));
	xchat_plugingui_remove(ph, lua_touserdata(master, -1));
	
	TRACE("clear master table entry\n");
	// clear its master table entry
	lua_pushstring(master, file);				/* ... ref xref L gui file */
	lua_pushnil(master);						/* ... ref xref L gui file nil */
	lua_rawset(master, -6); 					/* ... ref xref L gui */
	
	TRACE("deallocate script\n");
	// delete the script
	lua_close(L);
	TRACE("cleanup\n");
	lua_pop(master, 4);

	TRACE("return\n");
	return XCHAT_EAT_ALL;
}
