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
#include "mpc8xx.h"

#ifdef CONFIG_DBOX2_ENV_READ_FS
#include <cmd_fs.h>
#include <net.h>
#endif /* CONFIG_DBOX2_ENV_READ_FS */

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

int checkboard (void)
{
	unsigned char *hwi = (unsigned char *) (CFG_FLASH_BASE + CFG_HWINFO_OFFSET);
	unsigned char k = hwi[0];

	switch (k)
	{
		case 1:
		case 2:
		case 3:
			printf ("DBOX2, ");
			break;
		default:
			printf ("unknown board (0x%02x)\n", k);
			return -1;
	}

	switch (k)
	{
		case 1:
			printf ("Nokia\n");
			break;
		case 2:
			printf ("Phillips\n");
			break;
		case 3:
			printf ("Sagem\n");
			break;
	}

	return 0;
}

/* ------------------------------------------------------------------------- */

long int initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	int size = 0;
	unsigned char *hwi = (unsigned char *) (CFG_FLASH_BASE + CFG_HWINFO_OFFSET);
	unsigned char k = hwi[0];

	switch (k)
	{
		case 1:
			upmconfig (UPMB, (uint *) sdram_table_upmb_nokia, sizeof (sdram_table_upmb_nokia) / sizeof (uint));
			break;
	}

	if ( memctl->memc_br1 & 0x1 )
		size += ~(memctl->memc_or1 & 0xffff8000) + 1;
	if ( memctl->memc_br2 & 0x1 )
		size += ~(memctl->memc_or2 & 0xffff8000) + 1;

	return size;
}

/*-----------------------------------------------------------------------
 * Process Hardware Information Block:
 */

void load_sernum_ethaddr (bd_t *bd)
{
	int i;
	unsigned char *hwi;
	unsigned char  ethaddr[18];
	static char byte_to_hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	
	hwi = (unsigned char *) (CFG_FLASH_BASE + CFG_HWINFO_OFFSET);

	for (i = 0; i < 6; i++)
	{
		ethaddr[i*3] = byte_to_hex [ hwi[i+3] >> 4 ];
		ethaddr[i*3+1] = byte_to_hex [ hwi[i+3] & 0xf ];
		ethaddr[i*3+2] = ':';
	}
	ethaddr[17] = '\0';

	if (getenv ("ethaddr") == NULL) {
		setenv ("ethaddr", ethaddr);
	}
}

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_DBOX2_ENV_READ_FS
void load_env_fs (void)
{
	int size, i = 0;
	char *s = (char *) 0x100000;
	char *c = s;
	int namestart, nameend, valuestart, valueend;

	size = fs_fsload ((unsigned long) s, CONFIG_DBOX2_ENV_READ_FS);

	while (1)
	{
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
		else
			printf ("env: can't set \"%s\"\n", &s[namestart]);
	}
}
#endif /* CONFIG_DBOX2_ENV_READ_FS */

/* ------------------------------------------------------------------------- */
