#include <xchat-plugin.h>

/*	============================================================================
		Globals
============================================================================  */
xchat_plugin * ph;	/* plugin handle */


/*	============================================================================
		Entry Points
============================================================================  */
int __export xchat_plugin_init(
				xchat_plugin *plugin_handle,
				char **plugin_name,
				char **plugin_desc,
				char **plugin_version,
				char *arg)
{
    return 1;
}

int __export xchat_plugin_deinit()
{
	return 1;
}

