#include <common.h>

#ifdef CONFIG_DGS_BOOTMENU

#include "common/cmd_bootmenu.h"

dgs_bootmenu_t dgs_bootmenu_cmds[] =
{

	#if defined(CONFIG_RELOOK400S)	/*Usb bootloader */
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
	#else	/*Serial bootloader */
	{
		{ key_power, key_null },
		20*3,
		"menu_upgserial",
	},
	{
		{ key_front_power, key_null },
		20*3,
		"menu_upgserial",
	},
	{
		{ key_front_p_up, key_front_power, key_null },
		0,
		"menu_upgserial",
	},
	#endif
	{
		{ key_front_p_right, key_front_power, key_null },
		0,
		"menu_upgserial",
	},
	{
		{ key_front_p_down, key_front_power, key_null },
		0,
		"menu_set2set",
	},
	#if !defined(CONFIG_RELOOK100S)	/*no hdd in mutant */
	{
		{ key_front_down, key_null },
		20*3,
		"menu_hddfmt",
	},
	#endif
	{
		{ key_front_left, key_null },
		20*3,
		"menu_debugmode",
	},
	{
		{ key_front_p_left, key_front_power, key_null},
		0,
		"menu_upgnet",
	},

//Addition
	{
		{ key_front_ok, key_null },
		20*3,
		"menu_pal",
	},
	{
		{ key_front_up, key_null },
		20*3,
		"menu_vmode",
	},
	{
		{ key_front_right, key_null },
		20*3,
		"menu_mid",
	},



	{	/* always NULL terminated */
		{ key_null },
		0,
		NULL,
	},
};

#endif
