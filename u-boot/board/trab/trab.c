/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
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

/* #define DEBUG */

#include <common.h>
#include <cmd_bsp.h>
#include <malloc.h>
#include <s3c2400.h>

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_MODEM_SUPPORT
static int key_pressed(void);
extern void disable_putc(void);
extern int do_mdm_init; /* defined in common/main.c */

/*
 * We need a delay of at least 500 us after turning on the VFD clock
 * before we can read any useful information for the CPLD controlling
 * the keyboard switches. Let's play safe and wait 5 ms. The problem
 * is that timers are not available yet, so we use a manually timed
 * loop.
 */
#define KBD_MDELAY	5000
static void udelay_no_timer (int usec)
{
	DECLARE_GLOBAL_DATA_PTR;

	int i;
	int delay = usec * 3;

	for (i = 0; i < delay; i ++) gd->bd->bi_arch_number = 145;
}
#endif /* CONFIG_MODEM_SUPPORT */

/*
 * Miscellaneous platform dependent initialisations
 */

int board_init ()
{
#if defined(CONFIG_MODEM_SUPPORT) && defined(CONFIG_VFD)
	ulong size;
	unsigned long addr;
	extern void mem_malloc_init (ulong);
	extern int drv_vfd_init(void);
#endif
	DECLARE_GLOBAL_DATA_PTR;

	/* memory and cpu-speed are setup before relocation */
#ifdef CONFIG_TRAB_50MHZ
	/* change the clock to be 50 MHz 1:1:1 */
	/* MDIV:0x5c PDIV:4 SDIV:2 */
	rMPLLCON = 0x5c042;
	rCLKDIVN = 0;
#else
	/* change the clock to be 133 MHz 1:2:4 */
	/* MDIV:0x7d PDIV:4 SDIV:1 */
	rMPLLCON = 0x7d041;
	rCLKDIVN = 3;
#endif

	/* set up the I/O ports */
	rPACON = 0x3ffff;
	rPBCON = 0xaaaaaaaa;
	rPBUP  = 0xffff;
	/* INPUT nCTS0 nRTS0 TXD[1] TXD[0] RXD[1] RXD[0]	*/
	/*  00,    10,      10,      10,      10,      10,      10 	*/
	rPFCON = (2<<0) | (2<<2) | (2<<4) | (2<<6) | (2<<8) | (2<<10);
#ifdef CONFIG_HWFLOW
	/* do not pull up RXD0, RXD1, TXD0, TXD1, CTS0, RTS0 */
	rPFUP  = (1<<0) | (1<<1) | (1<<2) | (1<<3) | (1<<4) | (1<<5);
#else
	/* do not pull up RXD0, RXD1, TXD0, TXD1 */
	rPFUP  = (1<<0) | (1<<1) | (1<<2) | (1<<3);
#endif
	rPGCON = 0x0;
	rPGUP  = 0x0;
	rOPENCR= 0x0;

	/* arch number of SAMSUNG-Board */
	/* MACH_TYPE_SMDK2400 */
	/* XXX this isn't really correct, but keep it for now */
	gd->bd->bi_arch_number = 145;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = 0x0c000100;

#ifdef CONFIG_MODEM_SUPPORT
#ifdef CONFIG_VFD
#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif
	/*
	 * reserve memory for VFD display (always full pages)
	 */
	/* armboot_real_end is defined in the board-specific linker script */
	addr = (_armboot_real_end + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
	size = vfd_setmem (addr);
	gd->fb_base = addr;
	/* round to the next page boundary */
	addr += size;
	addr = (addr + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);
	mem_malloc_init (addr);
	/* must do this after the framebuffer is allocated */
	drv_vfd_init();
#endif /* CONFIG_VFD */

	udelay_no_timer (KBD_MDELAY);

	if (key_pressed()) {
		disable_putc();	/* modem doesn't understand banner etc */
		do_mdm_init = 1;
	}
#endif	/* CONFIG_MODEM_SUPPORT */

	return 0;
}

int dram_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	return 0;
}

/*-----------------------------------------------------------------------
 * Keyboard Controller
 */

/* Maximum key number */
#define KEYBD_KEY_NUM		4

#define KBD_DATA	(((*(volatile ulong *)0x04020000) >> 16) & 0xF)

static uchar *key_match (ulong);

int misc_init_r (void)
{
	ulong kbd_data = KBD_DATA;
	uchar keybd_env[KEYBD_KEY_NUM + 1];
	uchar *str;
	int i;

	for (i = 0; i < KEYBD_KEY_NUM; ++i) {
		keybd_env[i] = '0' + ((kbd_data >> i) & 1);
	}
	keybd_env[i] = '\0';
	debug ("** Setting keybd=\"%s\"\n", keybd_env);
	setenv ("keybd", keybd_env);

	str = strdup (key_match (kbd_data));	/* decode keys */

#ifdef CONFIG_PREBOOT	/* automatically configure "preboot" command on key match */
	debug ("** Setting preboot=\"%s\"\n", str);
	setenv ("preboot", str);	/* set or delete definition */
#endif /* CONFIG_PREBOOT */
	if (str != NULL) {
		free (str);
	}

	return (0);
}

#ifdef CONFIG_PREBOOT

static uchar kbd_magic_prefix[] = "key_magic";
static uchar kbd_command_prefix[] = "key_cmd";

static int compare_magic (ulong kbd_data, uchar *str)
{
	uchar key_mask;

	debug ("compare_magic: kbd: %04lx  str: \"%s\"\n",kbd_data,str);
	for (; *str; str++)
	{
		uchar c = *str - '1';

		if (c >= KEYBD_KEY_NUM)		/* bad key number */
			return -1;

		key_mask = 1 << c;

		if (!(kbd_data & key_mask)) {	/* key not pressed */
			debug ( "compare_magic: "
				"kbd: %04lx mask: %04lx - key not pressed\n",
				kbd_data, key_mask );
			return -1;
		}

		kbd_data &= ~key_mask;
	}

	if (kbd_data) {				/* key(s) not released */
		debug ( "compare_magic: "
			"kbd: %04lx - key(s) not released\n", kbd_data);
		return -1;
	}

	return 0;
}

/*-----------------------------------------------------------------------
 * Check if pressed key(s) match magic sequence,
 * and return the command string associated with that key(s).
 *
 * If no key press was decoded, NULL is returned.
 *
 * Note: the first character of the argument will be overwritten with
 * the "magic charcter code" of the decoded key(s), or '\0'.
 *
 *
 * Note: the string points to static environment data and must be
 * saved before you call any function that modifies the environment.
 */
static uchar *key_match (ulong kbd_data)
{
	uchar magic[sizeof (kbd_magic_prefix) + 1];
	uchar cmd_name[sizeof (kbd_command_prefix) + 1];
	uchar *suffix;
	uchar *kbd_magic_keys;

	/*
	 * The following string defines the characters that can pe appended
	 * to "key_magic" to form the names of environment variables that
	 * hold "magic" key codes, i. e. such key codes that can cause
	 * pre-boot actions. If the string is empty (""), then only
	 * "key_magic" is checked (old behaviour); the string "125" causes
	 * checks for "key_magic1", "key_magic2" and "key_magic5", etc.
	 */
	if ((kbd_magic_keys = getenv ("magic_keys")) == NULL)
		kbd_magic_keys = "";

	debug ("key_match: magic_keys=\"%s\"\n", kbd_magic_keys);

	/* loop over all magic keys;
	 * use '\0' suffix in case of empty string
	 */
	for (suffix=kbd_magic_keys; *suffix || suffix==kbd_magic_keys; ++suffix)
	{
		sprintf (magic, "%s%c", kbd_magic_prefix, *suffix);

		debug ("key_match: magic=\"%s\"\n",
			getenv(magic) ? getenv(magic) : "<UNDEFINED>");

		if (compare_magic(kbd_data, getenv(magic)) == 0)
		{
			sprintf (cmd_name, "%s%c", kbd_command_prefix, *suffix);
			debug ("key_match: cmdname %s=\"%s\"\n",
				cmd_name,
				getenv (cmd_name) ?
					getenv (cmd_name) :
					"<UNDEFINED>");
			return (getenv (cmd_name));
		}
	}
	debug ("key_match: no match\n");
	return (NULL);
}
#endif							/* CONFIG_PREBOOT */

/* Read Keyboard status */
int do_kbd (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	ulong kbd_data = KBD_DATA;
	uchar keybd_env[KEYBD_KEY_NUM + 1];
	int i;

	puts ("Keys:");
	for (i = 0; i < KEYBD_KEY_NUM; ++i) {
		keybd_env[i] = '0' + ((kbd_data >> i) & 1);
		printf (" %c", keybd_env[i]);
	}
	keybd_env[i] = '\0';
	putc ('\n');
	setenv ("keybd", keybd_env);
	return 0;
}

#ifdef CONFIG_MODEM_SUPPORT
static int key_pressed(void)
{
	return (compare_magic(KBD_DATA, CONFIG_MODEM_KEY_MAGIC) == 0);
}
#endif	/* CONFIG_MODEM_SUPPORT */
