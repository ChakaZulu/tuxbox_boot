/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2003 Felix Domke <tmbinc@tuxbox.org>
 *               2003 Bastian Blank <waldi@tuxbox.org>
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
#include <i2c.h>
#include <asm/processor.h>

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity
 */

static unsigned char config[128];

int checkboard (void)
{
	puts ("Board: DREAMBOX");
	putc ('\n');
	
	return (0);
}

static unsigned long testram(int base, int max)
{
	int amount;
	out32(base, 0x55AA55AA);
	for (amount=22; amount < max; amount++)
	{
		out32((1<<amount)|base, 0xAA55AA55);
		if (in32(base) != 0x55AA55AA)
			break;
	}
	return 1<<amount;
}

long int initdram (int board_type)
{
	int ram0=testram(0, 25);
	printf("ram0: %d\n", ram0/1024/1024);
	int ram1=testram(0x20000000, 25);
	printf("ram1: %d\n", ram1/1024/1024);
#if 0
		/* setup linear address space */
//	mtdcr(icbs0_amap0, ((ram0/1024/1024)<<4)|5);
	mtdcr(icbs0_amap0, ((64)<<4)|5);
	printf("amap0: %x\n", mfdcr(icbs0_amap0));
	mtdcr(icbs0_amap1, 0x4008600c);

	mtdcr(sdram1_cr0, 0x00000000);
	mtdcr(sdram1_cr1, 0x00000000);
		/* re-setup at base 0 */
	mtdcr(sdram1_br0, 0x04003000);
	mtdcr(sdram1_br1, 0x05003000);
	mtdcr(sdram1_br2, 0x00000000);
	mtdcr(sdram1_br3, 0x00000000);
	mtdcr(sdram1_cr0, 0x00f00000);
	mtdcr(sdram1_cr1, 0x00f00000);
	mtdcr(sdram1_cr2, 0x00000000);
	mtdcr(sdram1_cr3, 0x00000000);
	mtdcr(sdram1_cr0, 0x00f08000);
	
	printf("ok, linear mapping up and running.\n");
#if 0
	out32(0x03000000, 0x12345);
	printf("%x, %x\n", in32(0), in32(0x03000000));
	
	printf("clearing memory.\n");
	memset((void*)0x01000000, 48*1024*1024, 0);
#endif

	{
		int i;
		for (i=0x84000000; i<0x85000000; i+=4)
		{
			if (!(i&0xFFFF))
				printf("\rW%08x", i);
			out32(i, i);
		}
		for (i=0x84000000; i<0x85000000; i+=4)
		{
			if (!(i&0xFFFF))
				printf("\rR%08x", i);
			if (in32(i) != i)	
				printf("damaged at %x: %x\n", i, in32(i));
		}
	}
#endif
	
		/* yet fixed */
	return ram0; // +ram1;
}

/* ------------------------------------------------------------------------- */

int testdram (void)
{
	/* TODO: XXX XXX XXX */
	printf ("test: xxx MB - ok\n");

	return (0);
}

/* ------------------------------------------------------------------------- */

#define MIN(a,b)  ((a)<(b)?(a):(b))
#define MAX(a,b)  ((a)>(b)?(a):(b))
#define FIT(v,min,max)  MAX(MIN(v,max),min)
#define ENOUGH(v,unit)  (((v)-1)/(unit)+1)
#define EZ(v,unit)  ((v)?ENOUGH(v,unit):0)
#define SYS_CLOCK_NS  (1000 / IDE_SYS_FREQ)
#define IDE_SYS_FREQ  63      /* MHz */

#define MK_TIMING(AS, DIOP, DIOY, DH) \
    ((FIT((AS),0, 15) << 27) | \
   (FIT((DIOP),0, 63) << 20) | \
   (FIT((DIOY),0, 63) << 13) | \
       (FIT((DH),0,7) << 9))

static void stb_ide_init(void)
{
	out32(0x400f0008, 0x80000000); // enable channel
	out32(0x400f0124, 0xdc800000); // clear all status
	out32(0x400f0004, 0x80000000); // int enable
	out32(0x400f0108, (EZ(EZ(1250 + 2 * SYS_CLOCK_NS, SYS_CLOCK_NS) - 1, 8)) << 23);/* Chan 0 timeout */

	out32(0x400f0100, MK_TIMING(6, 19, 15, 2));
	out32(0x400f0104, MK_TIMING(6, 19, 15, 2));
}

void ide_set_reset(int on)
{
#if 0
	*((long*)0x40060000)&=~(0x80000000>>16);
	*((long*)0x40060004)|= (0x80000000>>16);
	*((long*)0x4006000C)&=~(0xC0000000);
	*((long*)0x40060014)&=~(0xC0000000);
	if (on)
		*((long*)0x40060000)&=~(0x80000000>>16);
	else
		*((long*)0x40060000)|=(0x80000000>>16);
#endif
}

/* ------------------------------------------------------------------------- */

int board_pre_init (void)
{
	/* hsmc controller */
	mtdcr (icbs0_amap0, 0);
	mtdcr (icbs0_amap1, 0);
	
	mtdcr (sdram1_besr, 0);

	/* second bank at 0x20000000... */
	mtdcr (sdram1_br0, 0x20003000);
	mtdcr (sdram1_br1, 0x21003000);
	mtdcr (sdram1_br2, 0x00000000);
	mtdcr (sdram1_br3, 0x00000000);
	mtdcr (sdram1_cr0, 0x00f00000);
	mtdcr (sdram1_cr1, 0x00f00000);
	mtdcr (sdram1_cr2, 0x00000000);
	mtdcr (sdram1_cr3, 0x00000000);

	/* enable sdram controller */
	mtdcr (sdram1_cr0, 0x00f08000);
	mtdcr (sdram1_cr1, 0x00f08000);
	
	/* everything is already initialized in the openbios phase */
	mtdcr (uicsr, 0xFFFFFFFF);	/* clear all ints */
	mtdcr (uicer, 0x00000000);	/* disable all ints */
	mtdcr (uiccr, 0x00000000);	/* set all to be non-critical */
	mtdcr (uicpr, 0xFFFFFF81);	/* set int polarities */
	mtdcr (uictr, 0x10000000);	/* set int trigger levels */
	mtdcr (uicvcr, 0x00000001);	/* set vect base=0,INT0 highest priority */
	mtdcr (uicsr, 0xFFFFFFFF);	/* clear all ints */

	stb_ide_init();

	return 0;
}

/* ------------------------------------------------------------------------- */

int misc_init_r (void)
{
	DECLARE_GLOBAL_DATA_PTR;

	char addr[] = {0x00, 0x09, 0x34, 0xba, 0xda, 0xdd};
	char tmp[32];

	i2c_write (0xEE>>1, 0x00, 0x00, "\x00", 1);
 	printf ("res: %d\n", i2c_read (0xEE>>1, 0x00, 0x00, config, 128));

	sprintf (tmp, "%02x:%02x:%02x:%02x:%02x:%02x",
			addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	setenv ("ethaddr", tmp);

	smc_set_mac_addr (addr);

	return 0;
}

/* ------------------------------------------------------------------------- */

#if (CONFIG_COMMANDS & CFG_CMD_DHCP) && (CONFIG_BOOTP_MASK & CONFIG_BOOTP_VENDOREX)
u8 *dhcp_vendorex_prep (u8 *e)
{
	const char *part1 = "DREAMBOX ";
	//const char *part2 = id2name[id];
	const char *part2 = "DM7000";

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

