/*
 * (C) Copyright 2002
 * Gary Jennejohn <gj@denx.de>
 *
 * Configuation settings for the TRAB board.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * If we are developing, we might want to start armboot from ram
 * so we MUST NOT initialize critical regs like mem-timing ...
 */
#define CONFIG_INIT_CRITICAL		/* undef for developing */

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_ARM920T		1	/* This is an arm920t CPU	*/
#define CONFIG_S3C2400		1	/* in a SAMSUNG S3C2400 SoC     */
#define CONFIG_TRAB		1	/* on a TRAB Board      */
#undef CONFIG_TRAB_50MHZ		/* run the CPU at 50 MHz */

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ	12000000 /* TRAB has 12 MHz input clock */

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */

#define CONFIG_CMDLINE_TAG	 1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	 1

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_CS8900	1	/* we have a CS8900 on-board */
#define CS8900_BASE		0x07000300 /* agrees with WIN CE PA */
#define CS8900_BUS16		1 /* the Linux driver does accesses as shorts */

#define CONFIG_VFD		1	/* VFD linear frame buffer driver */
#define VFD_TEST_LOGO		1	/* output a test logo to the VFDs */

/*
 * select serial console configuration
 */
#define CONFIG_SERIAL1          1	/* we use SERIAL 1 on TRAB */

#define CONFIG_HWFLOW			/* include RTS/CTS flow control support	*/

#define CONFIG_MODEM_SUPPORT	1	/* enable modem initialization stuff */

#define	CONFIG_MODEM_KEY_MAGIC	"23"	/* hold down these keys to enable modem */

/*
 * The following enables modem debugging stuff. The dbg() and
 * 'char screen[1024]' are used for debug printfs. Unfortunately,
 * it is usable only from BDI
 */
#undef CONFIG_MODEM_SUPPORT_DEBUG

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200

#define	CONFIG_TIMESTAMP	1	/* Print timestamp info for images */

#ifdef CONFIG_HWFLOW
#define CONFIG_COMMANDS_ADD_HWFLOW	CFG_CMD_HWFLOW
#else
#define	CONFIG_COMMANDS_ADD_HWFLOW	0
#endif

#ifdef	CONFIG_VFD
#define CONFIG_COMMANDS_ADD_VFD		CFG_CMD_VFD
#else
#define CONFIG_COMMANDS_ADD_VFD		0
#endif

#ifndef USE_920T_MMU
#define CONFIG_COMMANDS		((CONFIG_CMD_DFL & ~CFG_CMD_CACHE) | \
				 CFG_CMD_BSP			| \
				 CONFIG_COMMANDS_ADD_HWFLOW	| \
				 CONFIG_COMMANDS_ADD_VFD	)
#else
#define CONFIG_COMMANDS		(CONFIG_CMD_DFL			| \
				 CFG_CMD_BSP			| \
				 CONFIG_COMMANDS_ADD_HWFLOW	| \
				 CONFIG_COMMANDS_ADD_VFD	)
#endif

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>


#define CONFIG_BOOTDELAY	5
#define CONFIG_PREBOOT		"echo;echo *** booting ***;echo"
#define CONFIG_BOOTARGS    	"console=ttyS0"
#define CONFIG_ETHADDR		00:D0:93:00:61:11
#define CONFIG_NETMASK          255.255.255.0
#define CONFIG_IPADDR		192.168.3.27
#define CONFIG_SERVERIP		192.168.3.1
#define CONFIG_BOOTCOMMAND	"run flash_nfs"
#define	CONFIG_EXTRA_ENV_SETTINGS	\
	"nfs_args=setenv bootargs root=/dev/nfs rw " \
		"nfsroot=$(serverip):$(rootpath)\0" \
	"rootpath=/opt/eldk/arm_920TDI\0" \
	"ram_args=setenv bootargs root=/dev/ram rw\0" \
	"add_net=setenv bootargs $(bootargs) ethaddr=$(ethaddr) " \
		"ip=$(ipaddr):$(serverip):$(gatewayip):$(netmask):$(hostname)::off\0" \
	"add_misc=setenv bootargs $(bootargs) console=ttyS0 panic=1\0" \
	"load=tftp 0xC100000 /tftpboot/TRAB/u-boot.bin\0" \
	"update=protect off 1:0-8;era 1:0-8;cp.b 0xc100000 0 $(filesize);" \
		"setenv filesize;saveenv\0" \
	"loadfile=/tftpboot/TRAB/pImage\0" \
	"loadaddr=c400000\0" \
	"net_load=tftpboot $(loadaddr) $(loadfile)\0" \
	"net_nfs=run net_load nfs_args add_net add_misc;bootm\0" \
	"kernel_addr=00040000\0" \
	"flash_nfs=run nfs_args add_net add_misc;bootm $(kernel_addr)\0" \
	"mdm_init1=ATZ\0" \
	"mdm_init2=ATS0=1\0" \
	"mdm_flow_control=rts/cts\0"

#if 0	/* disabled for development */
#define	CONFIG_AUTOBOOT_KEYED		/* Enable password protection	*/
#define CONFIG_AUTOBOOT_PROMPT	"\nEnter password - autoboot in %d sec...\n"
#define CONFIG_AUTOBOOT_DELAY_STR	"system"	/* 1st password	*/
#endif

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CONFIG_KGDB_BAUDRATE	115200		/* speed to run kgdb serial port */
/* what's this ? it's not used anywhere */
#define CONFIG_KGDB_SER_INDEX	1		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP				/* undef to save memory		*/
#define	CFG_PROMPT		"TRAB # "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0c000000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0d000000	/* 16 MB in DRAM	*/

#undef  CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define	CFG_LOAD_ADDR		0x0cf00000	/* default load address	*/

#ifdef CONFIG_TRAB_50MHZ
/* the PWM TImer 4 uses a counter of 15625 for 10 ms, so we need */
/* it to wrap 100 times (total 1562500) to get 1 sec. */
/* this should _really_ be calculated !! */
#define	CFG_HZ			1562500
#else
/* the PWM TImer 4 uses a counter of 10390 for 10 ms, so we need */
/* it to wrap 100 times (total 1039000) to get 1 sec. */
/* this should _really_ be calculated !! */
#define	CFG_HZ			1039000
#endif

/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CONFIG_MISC_INIT_R		/* have misc_init_r() function	*/

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x0c000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x01000000 /* 16 MB */

#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */
#define PHYS_FLASH_SIZE		0x00800000 /* 8 MB */

/* The following #defines are needed to get flash environment right */
#define	CFG_MONITOR_BASE	PHYS_FLASH_1
#define	CFG_MONITOR_LEN		(256 << 10)

#define CFG_FLASH_BASE		PHYS_FLASH_1

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#define CFG_MAX_FLASH_SECT	(71)	/* max number of sectors on one chip */

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(2*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2*CFG_HZ) /* Timeout for Flash Write */

#define	CFG_ENV_IS_IN_FLASH	1

/* Address and size of Primary Environment Sector	*/
#define CFG_ENV_ADDR		(PHYS_FLASH_1 + 0x4000)
#define CFG_ENV_SIZE		0x4000

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_OFFSET_REDUND	(CFG_ENV_ADDR+CFG_ENV_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

#endif	/* __CONFIG_H */
