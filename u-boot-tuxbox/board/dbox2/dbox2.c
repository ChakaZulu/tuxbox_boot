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

