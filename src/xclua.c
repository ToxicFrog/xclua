#include <xchat-plugin.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <xclua.h>
#include <xclua_load_unload.h>
#include <xclua_ftw.h>

/*	============================================================================
		Globals
============================================================================  */
xchat_plugin * ph;	/* plugin handle */
lua_State * xclua_L;

/*	============================================================================
		Entry Points
============================================================================  */

int xchat_plugin_init(
				xchat_plugin *plugin_handle,
				char **plugin_name,
				char **plugin_desc,
				char **plugin_version,
				char *arg)
{
    ph = plugin_handle;
    *plugin_name = "Lua";
    *plugin_desc = "Lua scripting interface";
    *plugin_version = XCLUA_VERSION " (" LUA_RELEASE ")";
    
    /* FIXME - register hooks */
    xchat_hook_command(ph, "load", XCHAT_PRI_NORM, xclua_load, NULL, NULL);
    xchat_hook_command(ph, "unload", XCHAT_PRI_NORM, xclua_unload, NULL, NULL);
    /* register /lua */
    
    xclua_L = luaL_newstate();
    xclua_open_all_scripts();
    
    xchat_printf(ph, "[lua]\tLua scripting interface %s (%s) loaded.\n", XCLUA_VERSION, LUA_RELEASE);
    return 1;
}

int xchat_plugin_deinit()
{
    /* Unload all child plugins */
    lua_State * L = xclua_L;
    lua_pushnil(L);
    while (lua_next(L, LUA_GLOBALSINDEX))
    {
        Plugin * P = lua_touserdata(L, -1);
        lua_pop(L, 1);
        lua_close(P->L);
    }
    
    lua_close(L);
    xchat_printf(ph, "[lua]\tLua scripting interface unloaded.\n");
	return 1;
}
