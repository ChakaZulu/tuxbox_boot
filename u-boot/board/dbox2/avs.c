/*
 * avs.c
 *
 * Copyright (C) 2002 Bastian Blank <waldi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: avs.c,v 1.4 2003/02/17 20:37:32 alexw Exp $
 */

#include <common.h>
#include <i2c.h>

#ifdef CONFIG_DBOX2_FB

void avs_blank (int mid)
{
	switch (mid)
	{
		case 1:
			i2c_write (0x48, 0, 0, "\x00\x00\x00\x00\x00", 5);
			break;
		case 2:
			i2c_write (0x4a, 0, 0, "\x00\x00\x00\x00\x00\x00\x00\x00", 8);
			break;
		case 3:
			i2c_write (0x48, 0, 0, "\x00\x00\x00\x00\x00\x00\x00", 7);
			break;
		default:
			;
	}
}

void avs_init (int mid)
{
	switch (mid)
	{
		case 1:
			i2c_write (0x48, 0, 0, "\x00\x29\x89\x01\x00", 5);
			break;
		case 2:
			i2c_write (0x4a, 0, 0, "\x00\x00\x19\x11\xa5\x00\x30\x88", 8);
			break;
		case 3:
			i2c_write (0x48, 0, 0, "\x00\x00\x00\x00\x04\x3F\x00", 7);
			break;
		default:
			;
	}
}

#endif
