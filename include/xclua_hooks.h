#ifndef XCLUA_HOOKS_H
#define XCLUA_HOOKS_H

typedef struct _hook {
    lua_State * L;
    xchat_hook * hook;
    int ref;
} Hook;

void luaopen_xclua_hooks(lua_State * L);
int xclua_hook_command(lua_State * L);
int xclua_hook_collect(lua_State * L);

#if 0
void xclua_register_hook(XCLHook * hook);
int xclua_hook_command(lua_State * L);
int xclua_hook_print(lua_State * L);
int xclua_hook_server(lua_State * L);
int xclua_hook_timer(lua_State * L);
int xclua_unhook(lua_State * L);
#endif

#endif
