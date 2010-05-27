#ifndef XCLUA_HOOKS_H
#define XCLUA_HOOKS_H

typedef struct _hook {
    lua_State * L;
    xchat_hook * hook;
    int ref;
} Hook;

void luaopen_xclua_hooks(lua_State * L);
int xclua_hook_command(lua_State * L);
int xclua_hook_timer(lua_State * L);
int xclua_hook_collect(lua_State * L);

int xclua_callback(Hook * hook, char * name, char ** word, char ** word_eol);

#endif
