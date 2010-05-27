#ifndef XCLUA_LIBS_H
#define XCLUA_LIBS_H

void luaopen_xclua_io(lua_State * L);
void luaopen_xclua_list(lua_State * L);
void luaopen_xclua_str(lua_State * L);
void luaopen_xclua_context(lua_State * L);
void luaopen_xclua_hooks(lua_State * L);

#endif
