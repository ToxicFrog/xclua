#include <xchat-plugin.h>
#include <lua.h>

#include <xclua.h>

static int xclua_unload_by_name(const char * name)
{
    lua_State * L = xclua_L;
    
    lua_getglobal(L, name);
    Plugin * P = lua_touserdata(L, -1);
    lua_pop(L, 1);
    
    if (!P)
        return XCHAT_EAT_NONE;
    
    /* clear entry in master plugins table */
    lua_pushnil(L);
    lua_setglobal(L, name);
    
    /* release plugin state - garbage collector will handle everything else */
    lua_close(P->L);
    
    return XCHAT_EAT_ALL;
}

int xclua_unload(char ** word, char ** word_eol, void * userdata)
{
    /* no arguments/not a lua file: fall through to other plugins */
    if (word[2] == NULL)
        return XCHAT_EAT_NONE;

    /* find the plugin by filename and unload it */
    return xclua_unload_by_name(word[2]);
}
