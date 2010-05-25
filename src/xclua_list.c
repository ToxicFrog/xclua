#include <string.h>

#include <lua.h>
#include <lauxlib.h>
#include <xchat-plugin.h>
#include <xclua.h>

static int xclua_list(lua_State * L)
{
	int argc = lua_gettop(L);
	lua_checkstack(L, argc+4); /* ensure room for return values */
	
	for(int i = 0; i < argc; ++i)
	{
		const char * key = luaL_checkstring(L, i);
		
		xchat_list * list = xchat_list_get(ph, key);
		if(list == NULL)
		{
			lua_pushnil(L);
			continue;
		}
		
		const char * const * fields = xchat_list_fields(ph, key);
		if(fields == NULL)
		{
			lua_pushnil(L);
			continue;
		}
		
		// now we have (1) the table, and (2) the list of fields associated with it
		// from this, we build the table on the stack...
		lua_newtable(L);
			/* ... Tlist */
		for(int entry = 1; xchat_list_next(ph, list); ++entry)
		{
			lua_pushinteger(L, entry);
			lua_newtable(L);
				/* ... Tlist Ientry Tentry */
			for(int f = 0; fields[f] != NULL; ++f)
			{
				lua_pushstring(L, fields[f]+1);
					/* ... Tlist Tentry fieldname */
				switch(fields[f][0])
				{
				  case 'i':
				  	lua_pushinteger(L, xchat_list_int(ph, list, fields[f]+1));
					break;
				  case 's':
				  	lua_pushstring(L, xchat_list_str(ph, list, fields[f]+1));
					break;
				  case 't':
				  	lua_pushnumber(L, (lua_Number)xchat_list_time(ph, list, fields[f]+1));
					break;
                  case 'p':
                    lua_pushlightuserdata(L, (void *)xchat_list_str(ph, list, fields[f]+1));
				  default:
				  	lua_pushnil(L);
				}
					/* ... Tlist Ientry Tentry fieldname fieldval */
				lua_settable(L, -3);
					/* ... Tlist Ientry Tentry */
			} // foreach field in entry
			lua_settable(L, -3);
			/* ... Tlist */
		} // foreach entry in table
        xchat_list_free(ph, list);
	} // foreach key in argv
	return argc;
}

static int xclua_pref(lua_State * L)
{
    const char * string;
    int integer;
    int argc = lua_gettop(L);
    lua_checkstack(L, argc); /* ensure room for the return values */
    
    for (int i = 0; i < argc; ++i)
    {
        const char * name = luaL_checkstring(L, i);
        switch (xchat_get_prefs(ph, name, &string, &integer))
        {
          case 1:
            lua_pushstring(L, string);
            break;
          case 2:
            lua_pushnumber(L, integer);
            break;
          case 3:
            lua_pushboolean(L, integer);
            break;
          default:
            lua_pushnil(L);
            break;
        }
    }
    return argc;
}

static int xclua_info(lua_State * L)
{
    int argc = lua_gettop(L);
    
    for (int i = 0; i < argc; ++i)
    {
        const char * key = luaL_checkstring(L, 1);
        const char * rv = xchat_get_info(ph, key);
        if (!strcmp(key, "win_ptr"))
            lua_pushlightuserdata(L, (void *)rv);
        else
            lua_pushstring(L, rv);
    }
    
    return argc;
}

void luaopen_xclua_list(lua_State * L)
{
    lua_pushcfunction(L, xclua_list);
    lua_setfield(L, -2, "list");
    lua_pushcfunction(L, xclua_pref);
    lua_setfield(L, -2, "pref");
    lua_pushcfunction(L, xclua_info);
    lua_setfield(L, -2, "info");
}
