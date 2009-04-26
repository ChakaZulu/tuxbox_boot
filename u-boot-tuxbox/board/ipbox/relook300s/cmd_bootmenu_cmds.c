#include <common.h>

#ifdef CONFIG_DGS_BOOTMENU

#include "common/cmd_bootmenu.h"

dgs_bootmenu_t dgs_bootmenu_cmds[] =
{
	{
		{ key_power, key_null },
		20*3,
		"menu_upgusb",
	},
	{
		{ key_front_power, key_null },
		20*3,
		"menu_upgusb",
	},
	{
		{ key_front_p_up, key_front_power, key_null },
		0,
		"menu_upgusb",
	},
	{
		{ key_front_p_right, key_front_power, key_null },
		0,
		"menu_upgserial",
	},
	{
		{ key_front_down, key_null },
		20*3,
		"menu_hddfmt",
	},
	{
		{ key_front_left, key_null },
		20*3,
		"menu_debugmode",
	},
	{	/* always NULL terminated */
		{ key_null },
		0,
		NULL,
	},
};

#endif
