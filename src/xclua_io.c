#include <lua.h>
#include <lauxlib.h>
#include <xchat-plugin.h>
#include <xclua.h>
#include <ctype.h>

static int xclua_print(lua_State * L) {
	const char * text = luaL_checkstring(L, 1);
	xchat_print(ph, text);
	return 0;                       
}

static int xclua_command(lua_State * L) {
	const char * text = luaL_checkstring(L, 1);
	xchat_command(ph, text);
	return 0;
}

/**
 * This is known to be a terrible hack.
 * however, at present there's no way to dynamically construct ... arguments
 * (at least, not portably), and xchat doesn't have an xchat_emit_print_va
 * function either.
 * As of 2.8.x, all of the text events require 4 arguments or less.
 * We'll cap this at 8 and hope it's enough.
**/
static int xclua_emit_print(lua_State * L)
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

/* xchat.send_modes ("+o", "person", "person","person")
   xchat.send_modes ("+o", people)
   */
static int xclua_send_modes(lua_State * L) // FIXME
{
    size_t len;
    size_t n;
    char sign = '\0';
    char mode = '\0';
    const char * modes = luaL_checklstring(L, 1, &len);
    const char ** targets;
    
    if (lua_type(L, 2) == LUA_TTABLE)
    {
        if (lua_gettop(L) != 2)
            return luaL_error(L, "Too many arguments to xchat.send_modes");
        
        n = lua_objlen(L, 2);
        targets = lua_newuserdata(L, sizeof(char *) * n);
        
        for (size_t i = 0; i < n; ++i)
        {
            lua_rawgeti(L, 2, i+1);
            targets[i] = lua_tostring(L, -1);
            lua_pop(L, 1);
        }
    } else if (lua_type(L, 2) == LUA_TSTRING)
    {
        n = lua_gettop(L) - 1;
        targets = lua_newuserdata(L, sizeof(char *) * n);
        
        for (size_t i = 0; i < n; ++i)
        {
            targets[i] = lua_tostring(L, i+2);
        }
    }
    
    for (size_t i = 0; i < len; ++i)
    {
        if (modes[i] == '+' || modes[i] == '-')
        {
            sign = modes[i];
        } else if (isalpha(modes[i]))
        {
            mode = modes[i];
            if (!(sign && mode)) break;
            
            xchat_send_modes(ph, targets, n, 0, sign, mode);
        }
    }

    if (!(sign && mode))
        return luaL_error(L, "Invalid mode string '%s' to xchat.send_modes", modes);
    
    return 0;
}

#define XCLUA_PRINTF \
    "local format = string.format\n" \
    "function xchat.printf(...) return xchat.print(format(...)) end"

#define XCLUA_COMMANDF \
    "local format = string.format\n" \
    "function xchat.commandf(...) return xchat.command(format(...)) end"
    
void luaopen_xclua_io(lua_State * L)
{
    lua_pushcfunction(L, xclua_print);
    lua_setfield(L, -2, "print");
    
    lua_pushcfunction(L, xclua_command);
    lua_setfield(L, -2, "command");
    
    lua_pushcfunction(L, xclua_emit_print);
    lua_setfield(L, -2, "printe");

    if (luaL_dostring(L, XCLUA_PRINTF) || luaL_dostring(L, XCLUA_COMMANDF))
    {
        xchat_printf(ph, "Error in embedded lua: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    lua_pushcfunction(L, xclua_send_modes);
    lua_setfield(L, -2, "send_modes");
    
    return;
}
