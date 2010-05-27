#ifndef XCLUA_H
#define XCLUA_H

#include <xchat-plugin.h>
#include <lua.h>

#define XCLUA_VERSION "0.1"

typedef struct _plugin {
    const char * file;
    const char * name;
    const char * desc;
    const char * version;
    void * gui;
    lua_State * L;
} Plugin;

/* global plugin handle */
extern xchat_plugin * ph;

/* global list of loaded scripts slash general use lua VM */
extern lua_State * xclua_L;

#endif // XCLUA_H
