#include <lua.h>
#include <lauxlib.h>
#include <xchat-plugin.h>
#include <xclua.h>
#include <xclua_list.h>

int xclua_list_get(lua_State * L)
{
	int argc = lua_gettop(L);
	lua_checkstack(L, argc +2);
	
	for(int i = 0; i < argc; ++i)
	{
		const char * key = luaL_checkstring(L, 1);
		lua_remove(L, 1);
		
		xchat_list * list = xchat_list_get(ph, key);
		if(list == NULL)
		{
			lua_pushnil(L);
			continue;
		}
		
		const char ** fields = xchat_list_fields(ph, key);
		if(fields == NULL)
		{
			lua_pushnil(L);
			continue;
		}
		
		// now we have (1) the table, and (2) the list of fields associated with it
		// from this, we build the table on the stack...
		lua_newtable(L);
			/* ... Tlist */
		for(int entry = 1; xchat_list_next(ph,list); ++entry)
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
	} // foreach key in argv
	return argc;
}
