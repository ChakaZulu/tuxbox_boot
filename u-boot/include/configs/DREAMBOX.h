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

#define CONFIG_405D4		1	/* This is a PPC405D4 CPU	*/
#define CONFIG_4xx		1	/* ...member of PPC4xx family	*/
#define CONFIG_DREAMBOX		1	/* ...on a Dreambox board	*/
#define	CONFIG_SECONDSTAGE	1	/*  ...with a second state loader */

#define CONFIG_SYS_CLK_FREQ	27000000 /* external frequency to pll	*/

#if 1
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#undef	CONFIG_BOOTARGS

#define	CONFIG_BOOTCOMMAND							\
	"dhcp; tftp \"$(bootfile)\"; "						\
	"protect off 10040000 107fffff; "					\
        "setenv bootargs root=/dev/nfs rw nfsroot=$(serverip):$(rootpath),v3 "	\
	"ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask):$(hostname)::off "	\
	"console=$(console); "							\
	"bootm"
/*
#define	CONFIG_BOOTCOMMAND							\
	"protect off 10040000 107fffff; "					\
	"fsload; setenv bootargs root=/dev/mtdblock2 console=$(console); "	\
	"bootm"
*/

#define	CONFIG_EXTRA_ENV_SETTINGS 						\
	"console=ttyS0\0"

#define CONFIG_BAUDRATE		115200	/* console baudrate = 9.6kbps	*/
//#define CONFIG_BAUDRATE		9600	/* console baudrate = 9.6kbps	*/
#define	CONFIG_LOADS_ECHO	1	/* echo on for serial download	*/
#define	CFG_LOADS_BAUD_CHANGE	1	/* allow baudrate change	*/

#define CONFIG_BOOTP_MASK	( CONFIG_BOOTP_DEFAULT | CONFIG_BOOTP_VENDOREX )

#define CONFIG_COMMANDS		( CONFIG_CMD_DFL | \
				  CFG_CMD_FS | \
				  CFG_CMD_KGDB | \
				  CFG_CMD_IRQ )
/*				  CFG_CMD_DHCP | \ */

#define	CONFIG_FS		( CFG_FS_CRAMFS | CFG_FS_JFFS2 )

#define	CFG_FS_PART0_TYPE	CFG_FS_CRAMFS
#define	CFG_FS_PART0_OFFSET	0x10040000
#define	CFG_FS_PART0_SIZE	0x6e0000
#define	CFG_FS_PART1_TYPE	CFG_FS_JFFS2
#define	CFG_FS_PART1_OFFSET	0x10720000
#define	CFG_FS_PART1_SIZE	0xe0000

#define	CONFIG_DBOX2_FS_ENV_READ		"1:env"

#define	CONFIG_TUXBOX_NETWORK			1

#define	CONFIG_AUTOBOOT_SELECT			1
#define	CONFIG_AUTOBOOT_SELECT_AUTOBOOT		1
#define	CONFIG_AUTOBOOT_SELECT_NUMBER		3
#define	CONFIG_AUTOBOOT_SELECT_1_TEXT		"console on ttyS0"
#define	CONFIG_AUTOBOOT_SELECT_1_COMMAND	"setenv console ttyS0"
#define	CONFIG_AUTOBOOT_SELECT_2_TEXT		"console on fb0"
#define	CONFIG_AUTOBOOT_SELECT_2_COMMAND	"setenv console tty"
#define	CONFIG_AUTOBOOT_SELECT_3_TEXT		"console on null"
#define	CONFIG_AUTOBOOT_SELECT_3_COMMAND	"setenv console null"
#define	CONFIG_AUTOBOOT_SELECT_4_TEXT		""
#define	CONFIG_AUTOBOOT_SELECT_4_COMMAND	""
#define	CONFIG_AUTOBOOT_SELECT_5_TEXT		""
#define	CONFIG_AUTOBOOT_SELECT_5_COMMAND	""

#define CONFIG_BOARD_PRE_INIT
#define	CONFIG_MISC_INIT_R

#if 0
#define CONFIG_SERIAL_SOFTWARE_FIFO 4000
#else
#undef	CONFIG_SERIAL_SOFTWARE_FIFO
#endif


/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

#undef CONFIG_WATCHDOG			/* watchdog disabled		*/

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP			/* undef to save memory		*/
#define CFG_PROMPT	"=> "		/* Monitor Command Prompt	*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define	CFG_CBSIZE	1024		/* Console I/O Buffer Size	*/
#else
#define	CFG_CBSIZE	256		/* Console I/O Buffer Size	*/
#endif
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS	16		/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x02000000	/* 4 ... 64 MB in DRAM	*/

/*
 * If CFG_EXT_SERIAL_CLOCK, then the UART divisor is 1.
 * If CFG_405_UART_ERRATA_59, then UART divisor is 31.
 * Otherwise, UART divisor is determined by CPU Clock and CFG_BASE_BAUD value.
 * The Linux BASE_BAUD define should match this configuration.
 *    baseBaud = cpuClock/(uartDivisor*16)
 * If CFG_405_UART_ERRATA_59 and 200MHz CPU clock,
 * set Linux BASE_BAUD to 403200.
 */
#undef  CFG_EXT_SERIAL_CLOCK           /* external serial clock */
#undef  CFG_405_UART_ERRATA_59         /* 405GP/CR Rev. D silicon */
#define CFG_BASE_BAUD       691200

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

#define CFG_LOAD_ADDR		0x100000	/* default load address */
#define CFG_EXTBDINFO		1	/* To use extended board_into (bd_t) */

#define	CFG_HZ		1000		/* decrementer freq: 1 ms ticks	*/

#define CONFIG_HARD_I2C		1	/* I2C with hardware support	*/
#undef  CONFIG_SOFT_I2C			/* I2C bit-banged		*/
#define CFG_I2C_SPEED		400000	/* I2C speed and slave address	*/
#define CFG_I2C_SLAVE		0x7F


/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0xFF800000
#define CFG_MONITOR_BASE	0x00500000
#define CFG_MONITOR_LEN		(192 * 1024)	/* Reserve 196 kB for Monitor	*/
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
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/
#define	CFG_FLASH_CFI		1
#define	CFG_FLASH_INCREMENT	0

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define CFG_ENV_IS_IN_FLASH	1	/* use FLASH for environment vars */
#define CFG_ENV_OFFSET		0x00050000 /* Offset of Environment Sector  */
#define	CFG_ENV_SIZE		0x10000	/* Total Size of Environment Sector	*/
#define CFG_ENV_SECT_SIZE	0x10000	/* see README - env sector total size	*/
/*-----------------------------------------------------------------------
 * NVRAM organization
 */
#define CFG_NVRAM_BASE_ADDR	0xf0000000	/* NVRAM base address	*/
#define CFG_NVRAM_SIZE		0x1ff8		/* NVRAM size	*/

#ifdef CFG_ENV_IS_IN_NVRAM
#define CFG_ENV_SIZE		0x1000		/* Size of Environment vars	*/
#define CFG_ENV_ADDR		\
	(CFG_NVRAM_BASE_ADDR+CFG_NVRAM_SIZE-CFG_ENV_SIZE)	/* Env	*/
#endif
/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		8192	/* For IBM 405 CPUs			*/
#define CFG_CACHELINE_SIZE	32	/* ...			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	5	/* log base 2 of the above value	*/
#endif

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	CFG_FLASH_BASE	/* FLASH bank #0	*/
#define FLASH_BASE1_PRELIM	0		/* FLASH bank #1	*/


/* Configuration Port location */
#define CONFIG_PORT_ADDR	0xF0000500

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR       0x00100000  /* inside of SDRAM                     */
#define CFG_INIT_RAM_END        0x2000  /* End of used area in RAM             */
#define CFG_GBL_DATA_SIZE      128  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET    (CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define CFG_INIT_SP_OFFSET      CFG_GBL_DATA_OFFSET

/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	230400	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2	/* which serial port to use */
#endif

#define	CONFIG_DRIVER_SMC91111
#define	CONFIG_SMC91111_BASE	0x72000300

#endif	/* __CONFIG_H */
