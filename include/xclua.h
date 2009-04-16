/**
	The Big List
	
symbol			C	lua	init
----------------------------
plugin_init		*	*	-
load			*	-	-
unload			*	-	-
----------------------------
hook_command	*	-	*
hook_fd			-	-	-
hook_print		*	-	*
hook_server		*	-	*
hook_timer		*	-	*
unhook			*	-	*
----------------------------
command			*	-	*
commandf		-	*	*
print			*	-	*
printf			-	*	*
emit_print		*	-	*
send_modes		?	?	?
----------------------------
find_context	*	-	*
get_context		*	-	*
get_info		*	*	*
get_prefs		*	*	*
set_context		*	-	*
----------------------------
nickcmp			*	-	*
strip			*	-	*
free			-	-	-
----------------------------
list_get		*	-	*	as get_list
list_free		-	-	-
list_fields		-	-	-
list_next		-	-	-
list_str		-	-	-
list_int		-	-	-
list_time		-	-	-
----------------------------
plugingui_add	*	-	-
plugingui_rm	*	-	-
----------------------------
EAT_*			-	-	*
PRI_*			-	-	*
STRIP_*			-	-	*
**/

/**
	Note on where everything goes
	- Each plugin gets a table in REGISTRY[ph] which contains hook-function pairs
	- The master state contains a table in REGISTRY[ph] which contains filename-{ state, GUI-hook } pairs.
	- the former is used for hook, unhook, and callbacks
	- the latter for load and unload
**/

#ifndef XCLUA_H
#define XCLUA_H

#define PNAME		"XCLua"
#define PDESC		"Adds support for the Lua scripting language to XChat"
#define PVERSION	"1.0 RC1"

#ifdef WIN32
	#define PATHSEP '\\'
	#define __export __declspec(dllexport)
#else
	#define PATHSEP '/'
	#if (__GNUC__ >= 4)
		#define __export __attribute__ ((visibility("default")))
		#pragma GCC visibility push(hidden)
	#else
		#define __export
	#endif
#endif

#define lua_exch(L) lua_insert(L, -2)

#ifdef DEBUG
	#define TRACE(args...) fprintf(fout, args); fflush(fout)
#else
	#define TRACE(args...)
#endif

typedef struct ud_ {
	lua_State * L;
	xchat_hook * hook;
} XCLHook;

extern xchat_plugin * ph;	/* plugin handle */
extern lua_State * master;	/* master state */

int xclua_load(char ** word, char ** word_eol, void * ud);
int xclua_unload(char ** word, char ** word_eol, void * ud);
const char * xclua_basename(const char * pathname);
int xclua_is_script(const char * file);

#endif
