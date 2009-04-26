/*
 * (C) Copyright 2000, 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define CONFIG_STB034xx		1	/* CPU Name			*/
#define CONFIG_RELOOK300S	1	/* ...on a DGStation relook300s */

#define CONFIG_CPU_CLK_FREQ     (CONFIG_SYS_CLK_FREQ*3)	/* external frequency to pll   */
#define CONFIG_SYS_CLK_FREQ     54000000	/* external frequency to pll   */
#define CONFIG_PLL_FREQ		27000000
#define CONFIG_TB_CLK_FREQ	(CONFIG_PLL_FREQ*2)	/* time base clock freq */

#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/

#define CONFIG_MISC_INIT_F

#define CONFIG_BAUDRATE		115200

#define CONFIG_BOOTCOMMAND	\
				"bootm ff830000;"		\
				"setenv stdout vga;"		\
				"echo failed to boot...;"	\
				"echo going into debug mode.."
#define CONFIG_PREBOOT		\
				"dcache on;" \
				"if fsload 10 init.rfmod.uboot; then autoscr 10; fi;" \
				"echo Press esc to debug bootloader.;bootmenu"

#define CONFIG_STDIN		"serial"
#define CONFIG_STDOUT		"serial"
#define CONFIG_STDERR		"serial"

#define CONFIG_EXTRA_ENV_SETTINGS \
				"videofmt=pal\0" \
				"antiflicker=2\0" \
				"stdin=serial\0" \
				"stdout=serial\0" \
				"stderr=serial\0" \
				"menu_timeout=echo booting...;front_puts zrun;boot\0" \
				"menu_serialbreak=echo entering boot loader console.;front_puts \"cons\"\0" \
				"menu_upgusb=setenv stdout vga;setenv stderr vga;if usbupg; then reset; else echo Oops...; echo You can debug through serial port.; fi\0" \
				"menu_upgserial=setenv stdout vga;setenv stderr vga;serial_upgrade;reset\0" \
				"menu_hddfmt=setenv bootargs $bootargs format_hdd=1;front_puts FHdd;boot\0" \
				"menu_debugmode=setenv bootargs $bootargs debug debug_app=1;front_puts shel;boot\0" \
				"serial#=relook300s debugging\0"

#define CONFIG_CFB_CONSOLE
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_VIDEO_CLEARSCR
#define CFG_VIDEO_CHANGE_SIZE
#define CFG_CONSOLE_INFO_QUIET 1
#define CFG_VIDEO_START_FUNCTION
#define CFG_CONSOLE_IS_IN_ENV	1
#define CFG_VIDEO_STARTUP_IMAGE
#define CFG_VIDEO_STARTUP_IMAGE_ADDR 0xff804000
#define CFG_VIDEO_STARTUP_IMAGE_SIZE (0xff830000-0xff804000)

/* Size (bytes) of interrupt driven serial port buffer.
 * Set to 0 to use polling instead of interrupts.
 * Setting to 0 will also disable RTS/CTS handshaking.
 */
#define CONFIG_BOOTARGS		"console=ttySICC,115200 root=/dev/mtdblock2 panic=1"

/*-----------------------------------------------------------------------
 * Commands
 */
#define CONFIG_COMMANDS	       ( CFG_CMD_CACHE	| \
				CFG_CMD_AUTOSCRIPT | \
				CFG_CMD_BOOTD	| \
				CFG_CMD_I2C	| \
				CFG_CMD_IRQ	| \
				CFG_CMD_ENV	| \
				CFG_CMD_JFFS2	| \
				CFG_CMD_MEMORY	| \
				CFG_CMD_FLASH	)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*-----------------------------------------------------------------------
 * for jffs2 file system
 */
#if (CONFIG_COMMANDS&CFG_CMD_JFFS2)
#define CFG_JFFS2_STARTADDR	0xffd90000
#if 0
#define CFG_JFFS2_FIRST_BANK	0
#define CFG_JFFS2_FIRST_SECTOR	25
#define CFG_JFFS2_NUM_BANKS	1
#else
#define CFG_JFFS2_OFFSET	0xffd90000
#define CFG_JFFS2_SIZE		0x00240000
#endif
#endif

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 */
#define CFG_SICC_CONSOLE

#define CFG_HUSH_PARSER 1
#ifdef  CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2     "> "
#endif

#define CFG_LONGHELP		/* undef to save memory		*/
#define CFG_PROMPT		"[u-boot@relook300s boot]$ "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

#define CFG_LOAD_ADDR		0xff830000	/* default load address */
//#define CFG_LOAD_END_ADDR	0xff8fffff

#define	CFG_HZ		1000		/* decrementer freq: 1 ms ticks	*/

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_MONITOR_BASE	0xfffd0000
#define CFG_MONITOR_LEN		(64*3 * 1024)	/* Reserve 196 kB for Monitor	*/
#define CFG_MALLOC_LEN		(128 * 1024)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux */
/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CFG_FLASH_BASE		0xff800000
#define FLASH_BASE0_PRELIM	CFG_FLASH_BASE			/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	(CFG_FLASH_BASE+0x00400000)	/* FLASH bank #1	*/

#define CFG_FLASH_ERASE_TOUT	20000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	5000	/* Timeout for Flash Write (in ms)	*/

#define CFG_FLASH_UNPROTECT_MONITOR	/* do not protect monitor.		*/

#define CFG_FLASH_SUP_AMD
#define CFG_FLASH_SUP_STM
#define CFG_FLASH_SUP_MX
#define CFG_FLASH_SUP_EON
#define CFG_FLASH_SUP_AMD_LV320B
#define CFG_FLASH_SUP_STM_29W320DB
#define CFG_FLASH_SUP_MX_29LV320AB


/*-----------------------------------------------------------------------
 * Environnement in flash
 */
#ifdef CFG_ENV_IS_IN_FLASH
#define	CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/
#define CFG_ENV_SECT_SIZE	0x4000	/* see README - env sector total size	*/
#define CFG_ENV_OFFSET		(0)	/* Offset of Environment Sector  */
#endif

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		(128*CFG_CACHELINE_SIZE*2)
#define CFG_CACHELINE_SIZE	0x20
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif


/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR       0x00ff0000  /* inside of SDRAM                 */
#define CFG_INIT_RAM_END        0x00010000  /* End of used area in RAM         */
#define CFG_GBL_DATA_SIZE      128  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET    (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET      CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * for network device(dm9000)
 */
/* #define CONFIG_DRIVER_DM9000 */

/*-----------------------------------------------------------------------
 * booting menu that will select normal booting, system upgrade...
 */
#define CONFIG_DGS_BOOTMENU

/*-----------------------------------------------------------------------
 * system upgrade API
 */
#define CONFIG_DGS_UPGRADE
#define CFG_DGS_UPGRADE_BUFSIZE	0x810000	/* reserve 8M for usb download buffer. */
#define MY_VENDOR_ID		0x00444753	// " DGS"
#define MY_PRODUCT_ID		0x6c6f6f6b	// "look"
#define MY_HW_MODEL		0x00030000
#define MY_HW_VERSION		0x00010000

/*-----------------------------------------------------------------------
 * USB Client device driver for upgrade.
 */
#define CONFIG_PDIUSB12
#define PDIUSB_IOBASE		0xff500000
#define PDIUSB_IRQ		26
#define PDIUSB_SUSPEND_GPIO	28
#define PDIUSB_AD_ADDRESS	21
#define PDIUSB_CONNECT_TIMEOUT	10
#define PDIUSB_BUSTYPE		unsigned short

/*-----------------------------------------------------------------------
 * serial upgrade function.
 */
#define CONFIG_SERIAL_UPGRADE
#define CFG_SERIALDOWN_HEADER	"DGStationVestaTwin8M"

/*-----------------------------------------------------------------------
 * I2C device speed and address.
 */
#define CONFIG_HARD_I2C
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
#define CFG_I2C_SPEED		100000
#define CFG_I2C_SLAVE		0xfe
#endif

/*-----------------------------------------------------------------------
 * some other device driver
 */
#define CONFIG_PPC405_GPIO
#define CONFIG_DGS_FRONT


#endif
