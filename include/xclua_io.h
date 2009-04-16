#ifndef XCLUA_IO_H
#define XCLUA_IO_H

int xclua_print(lua_State * L);
int xclua_command(lua_State * L);
int xclua_emit_print(lua_State * L);
int xclua_send_modes(lua_State * L);

#endif
