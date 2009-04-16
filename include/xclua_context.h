#ifndef XCLUA_CONTEXT_H
#define XCLUA_CONTEXT_H

int xclua_find_context(lua_State * L);
int xclua_get_context(lua_State * L);
int xclua_get_info(lua_State * L);
int xclua_get_prefs(lua_State * L);
int xclua_set_context(lua_State * L);

#endif
