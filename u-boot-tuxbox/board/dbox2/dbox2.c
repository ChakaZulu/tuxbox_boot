/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2001 Florian Schirmer <jolt@tuxbox.org>
 *               2002 Bastian Blank <waldi@tuxbox.org>
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

#include <common.h>
#include <config.h>
#include "mpc8xx.h"

#ifdef CONFIG_DBOX2_ENV_READ
#include <cmd_fs.h>
#include <net.h>
#endif /* CONFIG_DBOX2_ENV_READ */

#if (CONFIG_COMMANDS & CFG_CMD_DHCP) && (CONFIG_BOOTP_MASK & CONFIG_BOOTP_VENDOREX)
#include <version.h>
#endif /* CFG_CMD_DHCP && CONFIG_BOOTP_VENDOREX */

/* ------------------------------------------------------------------------- */

#define _NOT_USED_ 0xFFFFFFFF

const uint sdram_table_upmb_nokia[] =
{
	0xFFF3CC04, 0xCFF7CC04, 0x0FFFDC04, 0x0FFFDC84,
	0x0FFFDC04, 0x0FFFDC84, 0x0FF3CC00, 0xFFF3CC47,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	0xFFFFCC04, 0xCFFFCC04, 0x00FFCC04, 0x00FFDC84,
	0x00FFDC04, 0x00FFDC84, 0x0FF3CC00, 0xFFF7CC47,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	0x7FFFFC07, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_
};

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity
 */

const char *id2name[] = { "EMPTY", "Nokia", "Philips", "Sagem" };
static unsigned char *hwi = (unsigned char *) (CFG_FLASH_BASE + CFG_HWINFO_OFFSET);
unsigned char mid = 0;

int checkboard (void)
{
	const char *bmon_version = "unknown";
	const char *ptr;
	mid = hwi[0];

	if (mid < 1 && mid > 3)
	{
		printf ("Board: unknown (0x%02x)\n", mid);
		return -1;
	}

	ptr = (char *)(CFG_FLASH_BASE + 0x14000);

	while (ptr < (char *)(CFG_FLASH_BASE + 0x16000)) {
		if (!memcmp(ptr, "dbox2:", 6)) {
			bmon_version = &ptr[8];
			break;
		}
		ptr += 4;
	}

	puts ("Board: DBOX2, ");
	puts (id2name[mid]);
	puts (", BMon V");
	puts (bmon_version);
	puts ("\n");

	return 0;
}

/* ------------------------------------------------------------------------- */

long int initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	int size = 0;

	if (mid == 1)
		upmconfig (UPMB, (uint *) sdram_table_upmb_nokia, sizeof (sdram_table_upmb_nokia) / sizeof (uint));

	if ( memctl->memc_br1 & 0x1 )
		size += ~(memctl->memc_or1 & 0xffff8000) + 1;
	if ( memctl->memc_br2 & 0x1 )
		size += ~(memctl->memc_or2 & 0xffff8000) + 1;

#ifdef CONFIG_DBOX2_IDE
	// set values for memcontroller for IDE interface  (will crash on boxes with external SDRAM)
	memctl->memc_br2 = 0x02000001;
	memctl->memc_or2 = 0xfe000966;
#endif

	return size;
}

/* ----------------------------------------------------------------------- */

int misc_init_r (void)
{
	char tmp[32];

	sprintf (tmp, "%02x:%02x:%02x:%02x:%02x:%02x",
			hwi[3], hwi[4], hwi[5], hwi[6], hwi[7], hwi[8]);
	setenv("ethaddr", tmp);

	return 0;
}

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_DBOX2_ENV_READ
#ifdef CONFIG_DBOX2_ENV_READ_FS
void load_env_fs (void)
{
	int size, i = 0;
	char *s = (char *) CFG_LOAD_ADDR;
	char *c = s;
	int namestart, nameend, valuestart, valueend;

	size = fs_fsload ((unsigned long) s, CONFIG_DBOX2_ENV_READ_FS);

	while (1) {
		if (i >= size)
			break;
		namestart = i;
		while (i < size && *c != '\n' && *c != '=') { i++; c++; }
		nameend = i;
		if (i >= size)
			break;
		i++; c++;
		valuestart = i;
		while (i < size && *c != '\n') { i++; c++; };
		valueend = i;
		i++; c++;
		s[nameend] = '\0';
		s[valueend] = '\0';
		
		if (!strcmp (&s[namestart], "bootcmd"))
			setenv ("bootcmd", &s[valuestart]);
		else if (!strcmp (&s[namestart], "console"))
			setenv ("console", &s[valuestart]);
		else if (!strcmp (&s[namestart], "lcd_contrast")) 
			setenv ("lcd_contrast", &s[valuestart]); 
		else if (!strcmp (&s[namestart], "lcd_inverse")) 
			setenv ("lcd_inverse", &s[valuestart]); 
		else
			printf ("env: can't set \"%s\"\n", &s[namestart]);
	}
}
#elif defined CONFIG_DBOX2_ENV_READ_NFS
void load_env_net (void)
{
	int size, i = 0;
	char *s = (char *) CFG_LOAD_ADDR;
	char *c = s;
	int namestart, nameend, valuestart, valueend;
	char *r;
	char tmp[256];

	NetLoop (BOOTP);
	if (getenv_r("rootpath", tmp, sizeof(tmp)) > 0) {
		r = tmp;
		strncat(r, CONFIG_DBOX2_ENV_READ_NFS, strlen(CONFIG_DBOX2_ENV_READ_NFS));
	} else {
		return;
	}

	copy_filename (BootFile, r, sizeof (BootFile));
	size = NetLoop (NFS);

	if (size <= 0) {
		printf ("can't find boot.conf\n");
		return;
	}

	while (1) {
		if (i >= size)
			break;
		namestart = i;
		while (i < size && *c != '\n' && *c != '=') { i++; c++; }
		nameend = i;
		if (i >= size)
			break;
		i++; c++;
		valuestart = i;
		while (i < size && *c != '\n') { i++; c++; };
		valueend = i;
		i++; c++;
                s[nameend] = '\0';
		s[valueend] = '\0';

		if (!strcmp (&s[namestart], "bootcmd"))
			setenv ("bootcmd", &s[valuestart]);
		else if (!strcmp (&s[namestart], "console"))
			setenv ("console", &s[valuestart]);
		else if (!strcmp (&s[namestart], "lcd_contrast"))
			setenv ("lcd_contrast", &s[valuestart]);
		else if (!strcmp (&s[namestart], "lcd_inverse"))
			setenv ("lcd_inverse", &s[valuestart]);
		else 
			printf ("env: can't set \"%s\"\n", &s[namestart]);
		
	}
}

#elif defined CONFIG_DBOX2_ENV_READ_TFTP
void load_env_net (void)
{
	int size, i = 0;
	char *s = (char *) CFG_LOAD_ADDR;
	char *c = s;
	int namestart, nameend, valuestart, valueend;

	NetLoop (BOOTP);
	copy_filename (BootFile, CONFIG_DBOX2_ENV_READ_TFTP, sizeof (BootFile));
	size = NetLoop (TFTP);

	if (size <= 0) {
		printf ("can't find boot.conf\n");
		return ;
	}
	
	while (1) {
		if (i >= size)
			break;
		namestart = i;
		while (i < size && *c != '\n' && *c != '=') { i++; c++; }
		nameend = i;
		if (i >= size)
			break;
		i++; c++;
		valuestart = i;
		while (i < size && *c != '\n') { i++; c++; };
		valueend = i;
		i++; c++;
		s[nameend] = '\0';
		s[valueend] = '\0';

		if (!strcmp (&s[namestart], "bootcmd"))
			setenv ("bootcmd", &s[valuestart]);
		else if (!strcmp (&s[namestart], "console"))
			setenv ("console", &s[valuestart]);
		else if (!strcmp (&s[namestart], "lcd_contrast"))
			setenv ("lcd_contrast", &s[valuestart]);
		else if (!strcmp (&s[namestart], "lcd_inverse"))
			setenv ("lcd_inverse", &s[valuestart]);
		else
			printf ("env: can't set \"%s\"\n", &s[namestart]);
	}
}

#endif /* CONFIG_DBOX2_ENV_READ_*    */
#endif /* CONFIG_DBOX2_ENV_READ      */

/* ------------------------------------------------------------------------- */

#if (CONFIG_COMMANDS & CFG_CMD_DHCP) && (CONFIG_BOOTP_MASK & CONFIG_BOOTP_VENDOREX)
u8 *dhcp_vendorex_prep (u8 *e)
{
	const char *part1 = "DBOX2, ";
	const char *part2 = id2name[mid];

	/* DHCP vendor-class-identifier = 60 */
	*e++ = 60;
	*e++ = strlen (part1) + strlen (part2);
	while (*part1)
		*e++ = *part1++;
	while (*part2)
		*e++ = *part2++;

	return e;
}

/* ------------------------------------------------------------------------- */

u8 *dhcp_vendorex_proc (u8 * popt)
{
	return NULL;
}
#endif /* CFG_CMD_DHCP && CONFIG_BOOTP_VENDOREX */

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_DBOX2_CPLD_IDE
static int cpld_avail = 0;


/* a lot of this code is copied from the ide driver. 
   todo: maybe it is possible to use the same source file for this */

/* address-offsets of features in the CPLD 
  (we can't call them registers...) */
#define CPLD_READ_DATA          0x00000000
#define CPLD_READ_FIFO          0x00000A00
#define CPLD_READ_CTRL          0x00000C00

#define CPLD_WRITE_FIFO         0x00000980
#define CPLD_WRITE_FIFO_HIGH    0x00000900
#define CPLD_WRITE_FIFO_LOW     0x00000880
#define CPLD_WRITE_CTRL_TIMING  0x00000860
#define CPLD_WRITE_CTRL         0x00000840
#define CPLD_WRITE_TIMING       0x00000820
#define CPLD_WRITE_DATA         0x00000800

/* bits in the control word */
#define CPLD_CTRL_WRITING 0x20
#define CPLD_CTRL_ENABLE  0x40
#define CPLD_CTRL_REPEAT  0x80

/* helping macros to access the CPLD */
#define CPLD_OUT(offset, value) ( *(volatile uint*)(idebase+(offset)) = (value))
#define CPLD_IN(offset) ( *(volatile uint*)(idebase+(offset)))
#define CPLD_FIFO_LEVEL() (CPLD_IN( CPLD_READ_CTRL)>>28)

#define printk printf

#define IMAP_ADDR CFG_IMMR

#define idebase 0x02000000

#define IDE_DELAY() udelay(50000)

static void dboxide_problem(const char *msg)
{
	printk("dboxide: %s\n", msg);
	printk("CPLD Status is %08x\n", CPLD_IN(CPLD_READ_CTRL));
}

#define WAIT_FOR_FIFO_EMPTY() wait_for_fifo_empty()
#define MAX_WAIT_FOR_FIFO_EMPTY  1000
static void wait_for_fifo_empty(void)
{
	int cnt = MAX_WAIT_FOR_FIFO_EMPTY;
	uint level;

	do {
		cnt--;
		level = CPLD_FIFO_LEVEL();
	} while ((level != 0) && (cnt > 0));

	if (cnt <= 0)
		dboxide_problem("fifo didn't get empty in time");
}


/* the CPLD is connected to CS2, which should be inactive.
   if not there might be something using that hardware and
   we don't want to disturb that */
static int activate_cs2(void)
{
	volatile immap_t *immap = (immap_t *) IMAP_ADDR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	uint br2 = memctl->memc_br2;

	if (br2 & 0x1) {
		printk("dboxide: cs2 already activated\n");
		return 0;
	}

	if (br2 != 0x02000080) {
		printk("dboxide: cs2: unexpected value for br2: %08x\n", br2);
		return 0;
	}

	br2 |= 0x1;

	printk("dboxide: activating cs2\n");
   
	memctl->memc_br2 = br2;

	return 1;
}

/* Deactivate CS2 when driver is not loaded */
static int deactivate_cs2(void)
{
	immap_t *immap = (immap_t *) IMAP_ADDR;
	memctl8xx_t *memctl = &immap->im_memctl;
	uint br2 = memctl->memc_br2;

	if (br2 != 0x02000081) {
		printk("dboxide: cs2 configuration unexpected: %08x\n", br2);
		return 0;
	}

	br2 &= ~1;

	printk("dboxide: deactivating cs2\n");
	memctl->memc_br2 = br2;

	return 1;
}

/* detect_cpld: Check that the CPLD really works */
static int detect_cpld(void)
{
	int i;
	uint check, back;
	uint patterns[2] = { 0xCAFEFEED, 0xBEEFC0DE };

	/* This detection code not only checks that there is a CPLD,
	   but also that it does work more or less as expected.  */

	/* first perform a walking bit test via data register:
	   this checks that there is a data register and
	   that the data bus is correctly connected */

	for (i = 0; i < 31; i++) {
		/* only one bit is 1 */
		check = 1 << i;
		CPLD_OUT(CPLD_WRITE_DATA, check);
		back = CPLD_IN(CPLD_READ_DATA);
		if (check != back) {
			printk
			    ("dboxide: probing dbox2 IDE CPLD: walking bit test failed: %08x != %08x\n",
			     check, back);
			return 0;
		}

		/* only one bit is 0 */
		check = ~check;
		CPLD_OUT(CPLD_WRITE_DATA, check);
		back = CPLD_IN(CPLD_READ_DATA);
		if (check != back) {
			printk
			    ("dboxide: probing dbox2 IDE CPLD: walking bit test failed: %08x != %08x\n",
			     check, back);
			return 0;
		}
	}

	/* second: check ctrl register.
	   this also activates the IDE Reset. */
	check = 0x00FF0007;
	CPLD_OUT(CPLD_WRITE_CTRL_TIMING, check);
	back = CPLD_IN(CPLD_READ_CTRL);
	if ((back & check) != check) {
		printk
		    ("dboxide: probing dbox2 IDE CPLD: ctrl register not valid: %08x != %08x\n",
		     check, back & check);
		return 0;
	}

	/* Now test the fifo:
	   If there is still data inside, read it out to clear it */
	for (i = 3; (i > 0) && ((back & 0xF0000000) != 0); i--) {
		CPLD_IN(CPLD_READ_FIFO);
		back = CPLD_IN(CPLD_READ_CTRL);
	}

	if (i == 0) {
		printk
		    ("dboxide: fifo seems to have data but clearing did not succeed\n");
		return 0;
	}

	/* then write two long words to the fifo */
	CPLD_OUT(CPLD_WRITE_FIFO, patterns[0]);
	CPLD_OUT(CPLD_WRITE_FIFO, patterns[1]);

	/* and read them back */
	back = CPLD_IN(CPLD_READ_FIFO);
	if (back != patterns[0]) {
		printk("dboxide: fifo did not store first test pattern\n");
		return 0;
	}
	back = CPLD_IN(CPLD_READ_FIFO);
	if (back != patterns[1]) {
		printk("dboxide: fifo did not store second test pattern\n");
		return 0;
	}

	/* now the fifo must be empty again */
	back = CPLD_IN(CPLD_READ_CTRL);
	if ((back & 0xF0000000) != 0) {
		printk("dboxide: fifo not empty after test\n");
		return 0;
	}

	/* Clean up: clear bits in fifo */
	check = 0;
	CPLD_OUT(CPLD_WRITE_FIFO, check);
	back = CPLD_IN(CPLD_READ_FIFO);
	if (back != check) {
		printk("dboxide: final fifo clear did not work: %x!=%x\n", back,
		       check);
		return 0;
	}

	/* CPLD is valid!
	   Hopefully the IDE part will also work:
	   A test for that part is not implemented, but the kernel
	   will probe for drives etc, so this will check a lot
	 */

	/* before going releasing IDE Reset, wait some time... */
	for (i = 0; i < 10; i++)
		IDE_DELAY();

	/* Activate PIO Mode 4 timing and remove IDE Reset */
	CPLD_OUT(CPLD_WRITE_CTRL_TIMING, 0x0012001F);

	/* finally set all bits in data register, so nothing
	   useful is read when the CPLD is accessed by the
	   original inb/w/l routines */
	CPLD_OUT(CPLD_WRITE_DATA, 0xFFFFFFFF);


	return 1;
}

int ide_preinit(void)
{
  //printf ("Checking for IDE-CPLD\n");

  if (activate_cs2()==0) return 1;

  if (detect_cpld() == 0) {
		deactivate_cs2();
		return 1;
	}

  //printf("dboxide: valid cpld detected\n");

  cpld_avail = 1;
  return 0;
}

void ide_outb(int dev, int port, unsigned char value)
{
  if (!cpld_avail) return;
  //printf ("outb to %08x with value %02x\n", port, value);

  if (CPLD_FIFO_LEVEL() != 0)
	dboxide_problem("outb: fifo not empty?!");

  CPLD_OUT(CPLD_WRITE_CTRL, port | CPLD_CTRL_ENABLE | CPLD_CTRL_WRITING);
  CPLD_OUT(CPLD_WRITE_FIFO_LOW, value << 8);

  WAIT_FOR_FIFO_EMPTY();
}

unsigned char ide_inb(int dev, int port)
{
  int val;

  if (!cpld_avail) return 0;
  //printf ("inb from %08x\n", port);

	if (CPLD_FIFO_LEVEL() != 0)
		dboxide_problem("inb: fifo not empty?!\n");

	CPLD_OUT(CPLD_WRITE_CTRL, port);
	CPLD_OUT(CPLD_WRITE_CTRL, port | CPLD_CTRL_ENABLE);

	while (CPLD_FIFO_LEVEL() == 0) {
	};

	val = CPLD_IN(CPLD_READ_FIFO);

	val >>= 8;
	val &= 0xFF;

  return val;
}

void input_data(int dev, ulong *sect_buf, int words)
{
  int count = words*2;
  ulong * dest = sect_buf;
  int port = 0x10;

  if (!cpld_avail) return;
  //printf ("input_data\n");

  	if (CPLD_FIFO_LEVEL() != 0)
		dboxide_problem("insw: fifo not empty?!");

	/* activate reading to fifo with auto repeat */
	CPLD_OUT(CPLD_WRITE_CTRL, port);
	CPLD_OUT(CPLD_WRITE_CTRL, port | CPLD_CTRL_ENABLE | CPLD_CTRL_REPEAT);

	while (count > 4) {
		ulong a;
 		ulong b;
		while (CPLD_FIFO_LEVEL() != 0xF) { };
		a = CPLD_IN(CPLD_READ_FIFO);	/* read 2 16 bit words */
		b = CPLD_IN(CPLD_READ_FIFO);	/* read 2 16 bit words */
		dest[0] = a;
		dest[1] = b;
		count -= 4;
		dest += 2;
	}

	if (count != 4)
		printk
		    ("dboxide: oops: insw: something has gone wrong: count is %d\n",
		     count);

	/* wait until fifo is full = 4 Words */
	while (CPLD_FIFO_LEVEL() != 0xF) {
	};

	/* then stop reading from ide */
	CPLD_OUT(CPLD_WRITE_CTRL, port);

	/* and read the final 4 16 bit words */
	dest[0] = CPLD_IN(CPLD_READ_FIFO);
	dest[1] = CPLD_IN(CPLD_READ_FIFO);
}

void output_data(int dev, ulong *sect_buf, int words)
{
  if (!cpld_avail) return;
  //printf ("output_data\n");
}

void input_swap_data(int dev, ulong *sect_buf, int words)
{
  int i;
  if (!cpld_avail) return;
  //printf ("input_swap_data\n");

  /* first read normally */
  input_data(dev, sect_buf, words );

  /* then swap the words */
  for (i=0; i<words; i++, sect_buf++)
  {
     ulong value;
     value = *sect_buf;

     value = (ulong)(((value&0xFF00FF00)>>8) |
                     ((value&0x00FF00FF)<<8));

     *sect_buf = value;
  }

}

#endif /* CONFIG_DBOX2_CPLD_IDE */

/* ------------------------------------------------------------------------- */
