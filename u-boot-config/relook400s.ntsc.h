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

#define CONFIG_STBx25xx		1	/* CPU Name		*/
#define CONFIG_RELOOK400S	1	/* DGStation relook400s */

#define CONFIG_CPU_CLK_FREQ     (CONFIG_SYS_CLK_FREQ*4)	/* external frequency to pll   */
#define CONFIG_SYS_CLK_FREQ     63000000		/* external frequency to pll   */
#define CONFIG_PLL_FREQ		27000000
#define CONFIG_TB_CLK_FREQ	(CONFIG_PLL_FREQ)	/* time base clock freq */

#define CFG_ENV_IS_IN_FLASH     1	/* use FLASH for environment vars	*/
#define CONFIG_ENV_OVERWRITE		/* allow change important environment */
#define CONFIG_MISC_INIT_F

#define CONFIG_BAUDRATE		115200

#define CONFIG_BOOTCOMMAND	\
				"setenv bootargs $bootargs videofmt=$videofmt;" \
				"bootm ff840000;"		\
				"setenv stdout vga;"		\
				"echo failed to boot...;"	\
				"echo going into debug mode.."
#define CONFIG_PREBOOT		\
				"dcache on;"			\
					"echo default for stv6412; " \
					"imw 4a 00 00; "	\
					"imw 4a 01 11; "	\
					"imw 4a 02 11; "	\
					"imw 4a 03 86; "	\
					"imw 4a 04 00; "	\
					"imw 4a 05 33; "	\
					"imw 4a 06 00; "	\
					"echo default for rfmod; " \
					"imw 65 84 48; "	\
					"imw 65 25 f4; "	\
				"edb toenv ethaddr;"		\
				"protect off all;"		\
				"echo Press esc to debug bootloader.;" \
				"bootmenu"

#define CONFIG_STDIN		"serial"
#define CONFIG_STDOUT		"serial"
#define CONFIG_STDERR		"serial"

#define CONFIG_EXTRA_ENV_SETTINGS \
				"videofmt=ntsc\0" \
				"antiflicker=2\0" \
				"stdin=serial\0" \
				"stdout=serial\0" \
				"stderr=serial\0" \
				"menu_pal=front_puts \"PAL MODE\";edb add videofmt pal;  echo resetting...;reset\0" \
				"menu_vmode=setenv stdout vga;setenv stderr vga;front_puts \"VMODE MENU\";vmode_select; echo resetting...;reset\0" \
				"menu_mid=setenv stdout vga;setenv stderr vga;front_puts \"MID MENU\";mid_select;  echo resetting...;reset\0" \
				"menu_timeout=logo;netupd;echo booting...;front_puts \"booting\";boot\0" \
				"menu_serialbreak=echo entering boot loader console.;front_puts \"console\"\0" \
				"menu_upgusb=setenv stdout vga;setenv stderr vga;if usbupg; then reset; else echo Oops...; echo You can debug through serial port.; fi\0" \
				"menu_upgserial=setenv stdout vga;setenv stderr vga;serial_upgrade;reset\0" \
				"menu_set2set=setenv stdout vga;setenv stderr vga;serial_set2set;reset\0" \
				"menu_hddfmt=setenv bootargs $bootargs format_hdd=1;front_puts \"format hdd\";boot\0" \
				"menu_debugmode=setenv bootargs $bootargs debug debug_app=1;front_puts \"debug shell\";boot\0" \
				"menu_upgnet=setenv bootargs $bootargs factory_update=1;front_puts \"net upg\";boot\0" \
				"serial#=relook400s debugging\0"

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
#define CFG_VIDEO_STARTUP_IMAGE_ADDR 0xff810000
#define CFG_VIDEO_STARTUP_IMAGE_SIZE (0xff840000-0xff810000)

/* #define CONFIG_BOOTARGS		"console=ttyS0,115200 root=/dev/nfs init=/sbin/simpleinit panic=1 ip=dhcp" */
#define CONFIG_BOOTARGS		"console=ttyS0,115200 root=/dev/mtdblock2 panic=1"

/*-----------------------------------------------------------------------
 * Commands
 */
#define CONFIG_COMMANDS	       ( CFG_CMD_CACHE	| \
				CFG_CMD_ECHO	| \
				CFG_CMD_AUTOSCRIPT | \
				CFG_CMD_BOOTD	| \
				CFG_CMD_EEPROM	| \
				CFG_CMD_I2C	| \
				CFG_CMD_IRQ	| \
				CFG_CMD_PING	| \
				CFG_CMD_ENV	| \
				CFG_CMD_NET	| \
				CFG_CMD_MEMORY	| \
				CFG_CMD_SETGETDCR | \
				CFG_CMD_FLASH	)

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>


/*-----------------------------------------------------------------------
 * for eeprom
 */
#define CONFIG_DGS_EEPROM_DB
#define CFG_I2C_EEPROM_ADDR		0x50
#define CFG_I2C_EEPROM_ADDR_LEN		2
#define CFG_EEPROM_SIZE			(16*1024)
#define CFG_EEPROM_PAGE_WRITE_DELAY_MS	11	/* 10ms. but give more */
#define CFG_EEPROM_PAGE_WRITE_BITS	6	/* 64 bytes in one page */

/*-----------------------------------------------------------------------
 * Miscellaneous configurable options
 */
#define CFG_HUSH_PARSER 1
#ifdef  CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2     "> "
#endif

#define CFG_LONGHELP		/* undef to save memory		*/
#define CFG_PROMPT		"[u-boot@relook400s boot]$ "	/* Monitor Command Prompt	*/
#define	CFG_CBSIZE		256		/* Console I/O Buffer Size	*/
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x00400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x00C00000	/* 4 ... 12 MB in DRAM	*/

/* The following table includes the supported baudrates */
#define CFG_BAUDRATE_TABLE  \
    {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400}

#define CFG_LOAD_ADDR		0xff840000	/* default load address */
//#define CFG_LOAD_END_ADDR	0xff8fffff

#define	CFG_HZ		100		/* decrementer freq: 10 ms ticks	*/

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
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	256	/* max number of sectors on one chip	*/

#define CFG_FLASH_BASE		0xff800000
#define FLASH_BASE0_PRELIM	CFG_FLASH_BASE			/* FLASH bank #0	*/

#define CFG_FLASH_ERASE_TOUT	20000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	5000	/* Timeout for Flash Write (in ms)	*/

//#define CFG_FLASH_SUP_AMD
#define CFG_FLASH_SUP_SPA	/* spansion is same with AMD */
#define CFG_FLASH_SUP_STM
#define CFG_FLASH_SUP_MX
#define CFG_FLASH_SUP_EON
#define CFG_FLASH_SUP_STM_29W640DT
#define CFG_FLASH_SUP_SPA_S29GL064MR3	/* top type. */
#define CFG_FLASH_SUP_SPA_S29GL064MR4	/* bottom type. */
#define CFG_FLASH_SUP_MX_29LV640MT	/* top type. this is same with SPA_S29GL064MR3 */
#define CFG_FLASH_SUP_MX_29LV640DT
#define CFG_FLASH_SUP_EON_29LV640B	/* bottom type */

/*-----------------------------------------------------------------------
 * Environnement in flash
 */
#ifdef CFG_ENV_IS_IN_FLASH
#define	CFG_ENV_SIZE		0x10000	/* Total Size of Environment Sector	*/
#define CFG_ENV_SECT_SIZE	0x10000	/* see README - env sector total size	*/
#define CFG_ENV_OFFSET		(0)	/* Offset of Environment Sector  */
#endif

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_DCACHE_SIZE		(256*CFG_CACHELINE_SIZE*2)
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
#if 0
#define CONFIG_DRIVER_DM9000
#define CONFIG_DM9000_IOBASE	0xfee00000
#define CONFIG_DM9000_IRQNUM	29

#endif

//New dm9k stuff
#define CONFIG_DRIVER_DM9000     1
#define CONFIG_DM9000_USE_EXTERNAL_EEPROM

#define CONFIG_DM9000_BASE 	0xfee00000
#define DM9000_IO           CONFIG_DM9000_BASE
#define DM9000_DATA         (CONFIG_DM9000_BASE+4)
/* #define CONFIG_DM9000_USE_8BIT */
#define CONFIG_DM9000_USE_16BIT
/* #define CONFIG_DM9000_USE_32BIT */

#define CONFIG_ETHADDR          00:13:18:00:00:ff
#define	CFG_DIRECT_FLASH_TFTP

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
#define MY_HW_MODEL		0x00040000

// old hardware before 05.06.17.
//#define MY_HW_VERSION		0x00010000

// new hardware after  05.06.17.
// . has 64MB system memory.
// . has data-bus buffer in each PCMCIA slot.
#define MY_HW_VERSION		0x00010001

/*-----------------------------------------------------------------------
 * USB Client device driver for upgrade.
 */
#define CONFIG_PDIUSB12
#define PDIUSB_IOBASE		0xfef00000
#define PDIUSB_IRQ		26
#define PDIUSB_DOWN_BUFFER	0x810000	/* reserve 8M for usb download buffer. */
#define PDIUSB_SUSPEND_GPIO	6
#define PDIUSB_AD_ADDRESS	21
#define PDIUSB_CONNECT_TIMEOUT	10
#define PDIUSB_BUSTYPE		unsigned char

/*-----------------------------------------------------------------------
 * serial upgrade function.
 */
#define CONFIG_SERIAL_UPGRADE
#define CONFIG_SERIAL_SET2SET
#define CFG_SERIALDOWN_HEADER	"relook400"

/*-----------------------------------------------------------------------
 * I2C device speed and address.
 */
/* #define CONFIG_HARD_I2C */
#define CONFIG_SOFT_I2C
#define CONFIG_I2CFAST
#if defined(CONFIG_HARD_I2C) || defined(CONFIG_SOFT_I2C)
#define CFG_I2C_SPEED		100000
#define CFG_I2C_SLAVE		0xfe
#endif

#ifdef CONFIG_SOFT_I2C
#define I2C_SDA_PIN_NUM	0
#define I2C_SCL_PIN_NUM	1
#define I2C_SDA_PIN	(0x80000000>>I2C_SDA_PIN_NUM)
#define I2C_SCL_PIN	(0x80000000>>I2C_SCL_PIN_NUM)
#define I2C_GPIO_OR	(*(volatile unsigned long*)0x40060000)
#define I2C_GPIO_TCR	(*(volatile unsigned long*)0x40060004)
#define I2C_GPIO_IR	(*(volatile unsigned long*)0x4006001c)

#define I2C_ACTIVE	do{ I2C_GPIO_TCR |=  I2C_SDA_PIN; }while(0)
#define I2C_TRISTATE	do{ I2C_GPIO_TCR &= ~I2C_SDA_PIN; }while(0)
#define I2C_READ	( (I2C_GPIO_IR&I2C_SDA_PIN) != 0 )
#define I2C_SDA(bit)	\
	if(bit)	I2C_GPIO_OR |=  I2C_SDA_PIN;	\
	else	I2C_GPIO_OR &= ~I2C_SDA_PIN;
#define I2C_SCL(bit)	\
	if(bit)	I2C_GPIO_OR |=  I2C_SCL_PIN;	\
	else	I2C_GPIO_OR &= ~I2C_SCL_PIN;
#define I2C_DELAY	udelay(2)
#endif

/*-----------------------------------------------------------------------
 * some other device driver
 */
#define CONFIG_PPC405_GPIO
#define CONFIG_DGS_FRONT

#endif	/* __CONFIG_H */
