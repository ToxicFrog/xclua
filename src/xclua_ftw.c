#include <xclua.h>
#include <xclua_ftw.h>
#include <stdio.h>
#include <string.h>

#include <limits.h>

#ifdef WIN32

#include <w32api.h>
#include <wtypes.h>

static char plugins[PATH_MAX];

void xclua_open_all_scripts()
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
    
    plugins[0] = '\0';
    strncat(plugins, xchat_get_info(ph, "xchatdir"), PATH_MAX);
    strncat(plugins, "/plugins/*.lua", PATH_MAX);

	hFind = FindFirstFile("plugins/*.lua", &findFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
            xchat_printf(ph, "[lua]\tAutoloading %s", findFileData.cFileName);
            xchat_commandf(ph, "load %s%s", plugins, findFileData.cFileName);
		} while (FindNextFile(hFind, &findFileData) != 0);
	}
	FindClose(hFind);
}

#else

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

/* Some linux distros are confused about where PATH_MAX goes */
#ifndef PATH_MAX
#include <linux/limits.h>
#endif

static char plugins[PATH_MAX];

static int is_lua_script(char * file)
{
    if (strstr(file, ".lua"))
        return 1;
    return 0;
}

void xclua_open_all_scripts()
{
    plugins[0] = '\0';
    strncat(plugins, xchat_get_info(ph, "xchatdir"), PATH_MAX);
    strncat(plugins, "/plugins/", PATH_MAX);

	DIR * din = opendir(plugins);
	struct dirent * ent;
	
	if( din != NULL )
	{
		while( (ent = readdir(din)) != NULL )
		{
			if( is_lua_script(ent->d_name) )
            {
                xchat_printf(ph, "[lua]\tAutoloading %s\n", ent->d_name);
                xchat_commandf(ph, "load %s%s", plugins, ent->d_name);
            }
		}
		closedir(din);
	}
}

#endif
