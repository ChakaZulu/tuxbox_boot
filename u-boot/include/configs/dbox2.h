/*
 * (C) Copyright 2000, 2001, 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2002 Bastian Blank <waldi@tuxbox.org>
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

/*
 * board/config.h - configuration options, board specific
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */

#define	CONFIG_MPC823		1	/* This is a MPC823 CPU		*/
#define	CONFIG_DBOX2		1	/* ...on a dbox2 device		*/

#define	CONFIG_LCD_BOARD	1	/* ...with LCD			*/
#define	CONFIG_DBOX2_FB		1	/* ...with FB			*/

#define	CONFIG_I2C		1	/* ...and I2C support		*/
#define	CONFIG_HARD_I2C		1	/*  ...hardware one		*/
#define	CFG_I2C_SPEED		50000
#define	CFG_I2C_SLAVE		0xFE

#define	CONFIG_8xx_CONS_SMC1	1	/* Console is on SMC1		*/
#undef	CONFIG_8xx_CONS_SMC2
#undef	CONFIG_8xx_CONS_NONE

#if 0
#define CONFIG_BOOTDELAY	-1	/* autoboot disabled		*/
#else
#define CONFIG_BOOTDELAY	5	/* autoboot after 5 seconds	*/
#endif

#define	CONFIG_CLOCKS_IN_MHZ	1	/* clocks passsed to Linux in MHz */

#undef	CONFIG_BOOTARGS

#define	CONFIG_BOOTCOMMAND							\
	"bootp; tftp \"kernel-cdk\"; "						\
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

#define CONFIG_BAUDRATE		9600	/* console baudrate = 9.6kbps	*/
#undef	CFG_LOADS_BAUD_CHANGE		/* don't allow baudrate change	*/

#define	CONFIG_WATCHDOG		1	/* watchdog enabled		*/

#define CONFIG_BOOTP_MASK	CONFIG_BOOTP_ALL

#define CONFIG_COMMANDS		( CONFIG_CMD_DFL | CFG_CMD_FS )

#define	CONFIG_FS		( CFG_FS_CRAMFS | CFG_FS_JFFS2 )

#define	CFG_FS_PART0_TYPE	CFG_FS_CRAMFS
#define	CFG_FS_PART0_OFFSET	0x10040000
#define	CFG_FS_PART0_SIZE	0x6e0000
#define	CFG_FS_PART1_TYPE	CFG_FS_JFFS2
#define	CFG_FS_PART1_OFFSET	0x10720000
#define	CFG_FS_PART1_SIZE	0xe0000

#define	CONFIG_DBOX2_FS_ENV_READ		"1:env"

#ifdef	CONFIG_LCD_BOARD
#define	CONFIG_DBOX2_LCD_INFO			1
#define	CONFIG_DBOX2_LCD_LOGO			1
#define	CONFIG_DBOX2_LCD_LOGO_FS		"1:logo-lcd"
#define	CONFIG_DBOX2_LCD_LOGO_TFTP		"logo-lcd"
#define	CONFIG_DBOX2_LCD_LOGO_RESERVE		2
//#define	CONFIG_DBOX2_LCD_FONT8x16		1
#endif	/* CONFIG_LCD_BOARD */
#ifdef	CONFIG_DBOX2_FB
#define	CONFIG_DBOX2_FB_LOGO			1
#define	CONFIG_DBOX2_FB_LOGO_FS			"1:logo-fb"
#define	CONFIG_DBOX2_FB_LOGO_TFTP		"logo-fb"
#endif	/* CONFIG_DBOX2_FB */

#define	CONFIG_DBOX2_AUTOBOOT_SELECT		1
#define	CONFIG_DBOX2_AUTOBOOT_SELECT_AUTOBOOT	1
#define	CONFIG_DBOX2_AUTOBOOT_SELECT_NUMBER	3
#define	CONFIG_DBOX2_AUTOBOOT_SELECT_1_TEXT	"framebuffer on ttyS0"
#define	CONFIG_DBOX2_AUTOBOOT_SELECT_1_COMMAND	"setenv console ttyS0"
#define	CONFIG_DBOX2_AUTOBOOT_SELECT_2_TEXT	"framebuffer on fb0"
#define	CONFIG_DBOX2_AUTOBOOT_SELECT_2_COMMAND	"setenv console tty"
#define	CONFIG_DBOX2_AUTOBOOT_SELECT_3_TEXT	"framebuffer on null"
#define	CONFIG_DBOX2_AUTOBOOT_SELECT_3_COMMAND	"setenv console null"
//#define	CONFIG_DBOX2_AUTOBOOT_SELECT_4_TEXT	""
//#define	CONFIG_DBOX2_AUTOBOOT_SELECT_4_COMMAND	""
//#define	CONFIG_DBOX2_AUTOBOOT_SELECT_5_TEXT	""
//#define	CONFIG_DBOX2_AUTOBOOT_SELECT_5_COMMAND	""

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/*
 * Miscellaneous configurable options
 */
#define	CFG_LONGHELP			/* undef to save memory		*/
#define	CFG_PROMPT		"=> "	/* Monitor Command Prompt	*/

#if 0
#define	CFG_HUSH_PARSER		1	/* use "hush" command parser	*/
#endif
#ifdef	CFG_HUSH_PARSER
#define	CFG_PROMPT_HUSH_PS2	"> "
#endif

#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define	CFG_CBSIZE		1024	/* Console I/O Buffer Size	*/
#else
#define	CFG_CBSIZE		256	/* Console I/O Buffer Size	*/
#endif
#define	CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define	CFG_MAXARGS		16	/* max number of command args	*/
#define CFG_BARGSIZE	CFG_CBSIZE	/* Boot Argument Buffer Size	*/

#define CFG_MEMTEST_START	0x0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0x0C00000	/* 4 ... 12 MB in DRAM	*/

#define	CFG_LOAD_ADDR		0x100000	/* default load address	*/

#define	CFG_HZ			1000	/* decrementer freq: 1 ms ticks	*/

#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/*
 * Low Level Configuration Settings
 * (address mappings, register initial values, etc.)
 * You should know what you are doing if you make changes here.
 */
/*-----------------------------------------------------------------------
 * Internal Memory Mapped Register
 */
#define CFG_IMMR		0xFFF00000

/*-----------------------------------------------------------------------
 * Definitions for initial stack pointer and data area (in DPRAM)
 */
#define CFG_INIT_RAM_ADDR	CFG_IMMR
#define	CFG_INIT_RAM_END	0x2F00	/* End of used area in DPRAM	*/
#define	CFG_GBL_DATA_SIZE	64  /* size in bytes reserved for initial data */
#define CFG_GBL_DATA_OFFSET	(CFG_INIT_RAM_END - CFG_GBL_DATA_SIZE)
#define	CFG_INIT_SP_OFFSET	CFG_GBL_DATA_OFFSET

/*-----------------------------------------------------------------------
 * Start addresses for the final memory configuration
 * (Set up by the startup code)
 * Please note that CFG_SDRAM_BASE _must_ start at 0
 */
#define	CFG_SDRAM_BASE		0x00000000
#define CFG_FLASH_BASE		0x10000000
#define	CFG_MONITOR_LEN		(256 << 10)	/* Reserve 256 kB for Monitor	*/
#define CFG_MONITOR_BASE	CFG_FLASH_BASE
#define	CFG_MALLOC_LEN		(128 << 10)	/* Reserve 128 kB for malloc()	*/

/*
 * For booting Linux, the board info and command line data
 * have to be in the first 8 MB of memory, since this is
 * the maximum mapped by the Linux kernel during initialization.
 */
#define	CFG_BOOTMAPSZ		(8 << 20)	/* Initial Memory map for Linux	*/

/*-----------------------------------------------------------------------
 * FLASH organization
 */
#define CFG_MAX_FLASH_BANKS	2	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	71	/* max number of sectors on one chip	*/

#define CFG_FLASH_ERASE_TOUT	120000	/* Timeout for Flash Erase (in ms)	*/
#define CFG_FLASH_WRITE_TOUT	500	/* Timeout for Flash Write (in ms)	*/

#define	CFG_ENV_IS_IN_FLASH	1
#define	CFG_ENV_OFFSET		0x8000	/*   Offset   of Environment Sector	*/
#define	CFG_ENV_SIZE		0x4000	/* Total Size of Environment Sector	*/

/* Address and size of Redundant Environment Sector	*/
#define CFG_ENV_OFFSET_REDUND	(CFG_ENV_OFFSET+CFG_ENV_SIZE)
#define CFG_ENV_SIZE_REDUND	(CFG_ENV_SIZE)

/*-----------------------------------------------------------------------
 * Hardware Information Block
 */
#define CFG_HWINFO_OFFSET	0x0001FFE0	/* offset of HW Info block */
#define CFG_HWINFO_SIZE		0x00000020	/* size   of HW Info block */

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#define CFG_CACHELINE_SIZE	16	/* For all MPC8xx CPUs			*/
#if (CONFIG_COMMANDS & CFG_CMD_KGDB)
#define CFG_CACHELINE_SHIFT	4	/* log base 2 of the above value	*/
#endif

/*-----------------------------------------------------------------------
 * SYPCR - System Protection Control				11-9
 * SYPCR can only be written once after reset!
 *-----------------------------------------------------------------------
 * Software & Bus Monitor Timer max, Bus Monitor enable, SW Watchdog freeze
 */
#if defined(CONFIG_WATCHDOG)
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | \
			 SYPCR_SWE  | SYPCR_SWRI| SYPCR_SWP)
#else
#define CFG_SYPCR	(SYPCR_SWTC | SYPCR_BMT | SYPCR_BME | SYPCR_SWF | SYPCR_SWP)
#endif

/*-----------------------------------------------------------------------
 * SIUMCR - SIU Module Configuration				11-6
 *-----------------------------------------------------------------------
 * PCMCIA config., multi-function pin tri-state
 */
#define CFG_SIUMCR	(SIUMCR_DBGC00 | SIUMCR_DBPC00 | SIUMCR_MLRC01)

/*-----------------------------------------------------------------------
 * TBSCR - Time Base Status and Control				11-26
 *-----------------------------------------------------------------------
 * Clear Reference Interrupt Status, Timebase freezing enabled
 */
#define CFG_TBSCR	(TBSCR_REFA | TBSCR_REFB | TBSCR_TBF)

/*-----------------------------------------------------------------------
 * RTCSC - Real-Time Clock Status and Control Register		11-27
 *-----------------------------------------------------------------------
 */
#define CFG_RTCSC	(RTCSC_SEC | RTCSC_ALR | RTCSC_RTF| RTCSC_RTE)

/*-----------------------------------------------------------------------
 * PISCR - Periodic Interrupt Status and Control		11-31
 *-----------------------------------------------------------------------
 * Clear Periodic Interrupt Status, Interrupt Timer freezing enabled
 */
#define CFG_PISCR	(PISCR_PS | PISCR_PITF)

/*-----------------------------------------------------------------------
 * PLPRCR - PLL, Low-Power, and Reset Control Register		15-30
 *-----------------------------------------------------------------------
 * Reset PLL lock status sticky bit, timer expired status bit and timer
 * interrupt status bit
 */
#define CFG_PLPRCR	(PLPRCR_SPLSS | PLPRCR_TEXPS | PLPRCR_TMIST)

/*-----------------------------------------------------------------------
 * SCCR - System Clock and reset Control Register		15-27
 *-----------------------------------------------------------------------
 * Set clock output, timebase and RTC source and divider,
 * power management and some other internal clocks
 */
#define SCCR_MASK	SCCR_EBDF11
#define CFG_SCCR	(SCCR_TBS     | \
			 SCCR_COM00   | SCCR_DFSYNC00 | SCCR_DFBRG00  | \
			 SCCR_DFNL000 | SCCR_DFNH000  | SCCR_DFLCD000 | \
			 SCCR_DFALCD00)

/*-----------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------
 *
 */
/*#define	CFG_DER	0x2002000F*/
#define CFG_DER	0

/*
 * Init Memory Controller:
 *
 * BR0/1 and OR0/1 (FLASH)
 */

#define FLASH_BASE0_PRELIM	0x10000000	/* FLASH bank #0	*/

/* used to re-map FLASH both when starting from SRAM or FLASH:
 * restrict access enough to keep SRAM working (if any)
 * but not too much to meddle with FLASH accesses
 */
#define CFG_REMAP_OR_AM		0x80000000	/* OR addr mask */
#define CFG_PRELIM_OR_AM	0xE0000000	/* OR addr mask */

/*
 * FLASH timing:
 */
/* ACS = 11, TRLX = 0, CSNT = 1, SCY = 5, EHTR = 1 */
#define	CFG_OR_TIMING_FLASH	(OR_ACS_DIV2  | OR_CSNT_SAM | \
				 OR_SCY_5_CLK | OR_EHTR | OR_BI)

#define CFG_OR0_REMAP	(CFG_REMAP_OR_AM  | CFG_OR_TIMING_FLASH)
#define CFG_OR0_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_FLASH)
#define CFG_BR0_PRELIM	((FLASH_BASE0_PRELIM & BR_BA_MSK) | BR_V )

/*
 * BR2/3 and OR2/3 (SDRAM)
 *
 */
#define SDRAM_BASE2_PRELIM	0x00000000	/* SDRAM bank #0	*/
#define	SDRAM_MAX_SIZE		0x04000000	/* max 64 MB per bank	*/

/* SDRAM timing: Multiplexed addresses, GPL5 output to GPL5_A (don't care)	*/
#define CFG_OR_TIMING_SDRAM	0x00000A00

#define CFG_OR2_PRELIM	(CFG_PRELIM_OR_AM | CFG_OR_TIMING_SDRAM )
#define CFG_BR2_PRELIM	((SDRAM_BASE2_PRELIM & BR_BA_MSK) | BR_MS_UPMA | BR_V )

/*
 * Memory Periodic Timer Prescaler
 */
#define CFG_MAMR_PTA		 97

/* refresh rate 15.6 us (= 64 ms / 4K = 62.4 / quad bursts) for <= 128 MBit	*/
#define CFG_MPTPR_2BK_4K	MPTPR_PTP_DIV16		/* setting for 2 banks	*/
#define CFG_MPTPR_1BK_4K	MPTPR_PTP_DIV32		/* setting for 1 bank	*/

/* refresh rate 7.8 us (= 64 ms / 8K = 31.2 / quad bursts) for 256 MBit		*/
#define CFG_MPTPR_2BK_8K	MPTPR_PTP_DIV8		/* setting for 2 banks	*/
#define CFG_MPTPR_1BK_8K	MPTPR_PTP_DIV16		/* setting for 1 bank	*/

/*
 * MAMR settings for SDRAM
 */

/* 8 column SDRAM */
#define CFG_MAMR_8COL	((CFG_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_0 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A11 |	\
			 MAMR_RLFA_1X	 | MAMR_WLFA_1X	   | MAMR_TLFA_4X)
/* 9 column SDRAM */
#define CFG_MAMR_9COL	((CFG_MAMR_PTA << MAMR_PTA_SHIFT)  | MAMR_PTAE	    |	\
			 MAMR_AMA_TYPE_1 | MAMR_DSA_1_CYCL | MAMR_G0CLA_A10 |	\
			 MAMR_RLFA_1X	 | MAMR_WLFA_1X	   | MAMR_TLFA_4X)


/*
 * Internal Definitions
 *
 * Boot Flags
 */
#define	BOOTFLAG_COLD	0x01		/* Normal Power-On: Boot from FLASH	*/
#define BOOTFLAG_WARM	0x02		/* Software reboot			*/

/* values according to the manual */

#define	BCSR_ADDR			((uint) 0xff010000)
#define	BCSR0				((uint) (BCSR_ADDR + 0x00))
#define	BCSR1				((uint) (BCSR_ADDR + 0x04))
#define	BCSR2				((uint) (BCSR_ADDR + 0x08))
#define	BCSR3				((uint) (BCSR_ADDR + 0x0c))
#define	BCSR4				((uint) (BCSR_ADDR + 0x10))

/* FADS bitvalues by Helmut Buchsbaum
 * see MPC8xxADS User's Manual for a proper description
 * of the following structures
 */

#define	BCSR0_ERB			((uint)0x80000000)
#define	BCSR0_IP			((uint)0x40000000)
#define	BCSR0_BDIS			((uint)0x10000000)
#define	BCSR0_BPS_MASK			((uint)0x0C000000)
#define	BCSR0_ISB_MASK			((uint)0x01800000)
#define	BCSR0_DBGC_MASK			((uint)0x00600000)
#define	BCSR0_DBPC_MASK			((uint)0x00180000)
#define	BCSR0_EBDF_MASK			((uint)0x00060000)

#define	BCSR1_FLASH_EN			((uint)0x80000000)
#define	BCSR1_DRAM_EN			((uint)0x40000000)
#define	BCSR1_ETHEN			((uint)0x20000000)
#define	BCSR1_IRDEN			((uint)0x10000000)
#define	BCSR1_FLASH_CFG_EN		((uint)0x08000000)
#define	BCSR1_CNT_REG_EN_PROTECT	((uint)0x04000000)
#define	BCSR1_BCSR_EN			((uint)0x02000000)
#define	BCSR1_RS232EN_1			((uint)0x01000000)
#define	BCSR1_PCCEN			((uint)0x00800000)
#define	BCSR1_PCCVCC0			((uint)0x00400000)
#define	BCSR1_PCCVPP_MASK		((uint)0x00300000)
#define	BCSR1_DRAM_HALF_WORD		((uint)0x00080000)
#define	BCSR1_RS232EN_2			((uint)0x00040000)
#define	BCSR1_SDRAM_EN			((uint)0x00020000)
#define	BCSR1_PCCVCC1			((uint)0x00010000)

#define	BCSR2_FLASH_PD_MASK		((uint)0xF0000000)
#define	BCSR2_DRAM_PD_MASK		((uint)0x07800000)
#define	BCSR2_DRAM_PD_SHIFT		(23)
#define	BCSR2_EXTTOLI_MASK		((uint)0x00780000)
#define	BCSR2_DBREVNR_MASK		((uint)0x00030000)

#define	BCSR3_DBID_MASK			((ushort)0x3800)
#define	BCSR3_CNT_REG_EN_PROTECT	((ushort)0x0400)
#define	BCSR3_BREVNR0			((ushort)0x0080)
#define	BCSR3_FLASH_PD_MASK		((ushort)0x0070)
#define	BCSR3_BREVN1			((ushort)0x0008)
#define	BCSR3_BREVN2_MASK		((ushort)0x0003)
#define	BCSR4_ETHLOOP			((uint)0x80000000)
#define	BCSR4_TFPLDL			((uint)0x40000000)
#define	BCSR4_TPSQEL			((uint)0x20000000)
#define	BCSR4_SIGNAL_LAMP		((uint)0x10000000)
#define	BCSR4_USB_EN			((uint)0x08000000)
#define	BCSR4_USB_SPEED			((uint)0x04000000)
#define	BCSR4_VCCO			((uint)0x02000000)
#define	BCSR4_VIDEO_ON			((uint)0x00800000)
#define	BCSR4_VDO_EKT_CLK_EN		((uint)0x00400000)
#define	BCSR4_VIDEO_RST			((uint)0x00200000)
#define	BCSR4_MODEM_EN			((uint)0x00100000)
#define	BCSR4_DATA_VOICE		((uint)0x00080000)

#endif	/* __CONFIG_H */
