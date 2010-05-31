#ifndef XCLUA_UTIL_H
#define XCLUA_UTIL_H

#include <xclua.h>

void * xclua_alloc(lua_State * L, size_t size, char * type);
void xclua_free(lua_State * L, void * ptr);

#endif
