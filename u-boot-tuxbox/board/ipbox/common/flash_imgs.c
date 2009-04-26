#include <common.h>

#if defined(CONFIG_SERIAL_SET2SET) || defined(CFG_JFFS_CUSTOM_PART)

#include "common/flash_img_info.h"

/*
 * MUST use same values from kernel mtd block sizes...
 *
 * see file dgs/common/linux/drivers/mtd/maps/relookxxx.c
 * you can just copy below definations from the file.
 */
#define WINDOW_ADDR	0xff800000
#define WINDOW_SIZE	0x00800000


#if defined(CONFIG_RELOOK100S) 		/* mutant has smaller kernel */

#define KERNEL_SIZE	0x120000	// kernel size
#define DB_SIZE		0x2f0000

#else								/* hdd machines (cube and relook400)*/

#define KERNEL_SIZE	0x140000	// kernel size
#define DB_SIZE		0x230000

#endif

#define WELCOME_SIZE	0x040000
#define SYSTEM_SIZE	(WINDOW_SIZE-WELCOME_SIZE-KERNEL_SIZE-DB_SIZE-BOOT_SIZE)
#define DB_SIZE		0x230000
#define BOOT_SIZE	0x030000	// boot loader

flash_img_info_t dgs_flash_imgs[] =
{
	{
		"config_welcome",
		WINDOW_ADDR,
		WELCOME_SIZE,
	},
	{
		"kernel",
		WINDOW_ADDR+WELCOME_SIZE,
		KERNEL_SIZE,
	},
	{
		"root",
		WINDOW_ADDR+WELCOME_SIZE+KERNEL_SIZE,
		SYSTEM_SIZE,
	},
	{
		"db",
		WINDOW_ADDR+WELCOME_SIZE+KERNEL_SIZE+SYSTEM_SIZE,
		DB_SIZE,
	},
	{
		"boot",
		WINDOW_ADDR+WELCOME_SIZE+KERNEL_SIZE+SYSTEM_SIZE+DB_SIZE,
		BOOT_SIZE,
	},

	/*************************/
	{
		"all",
		WINDOW_ADDR,
		WINDOW_SIZE,
	},
	{
		"all_noboot",
		WINDOW_ADDR,
		WELCOME_SIZE+KERNEL_SIZE+SYSTEM_SIZE+DB_SIZE,
	},
	{
		"kernel_root",
		WINDOW_ADDR+WELCOME_SIZE,
		KERNEL_SIZE+SYSTEM_SIZE,
	},

	/*************************/
	{
		NULL,
		0x00000000,
		0x00000000,
	},
};

#endif
