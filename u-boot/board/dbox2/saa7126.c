/*
 * saa7126.c
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
 * $Id: saa7126.c,v 1.4 2003/03/12 20:20:13 waldi Exp $
 */

#include <common.h>
#include <i2c.h>

#ifdef CONFIG_DBOX2_FB

struct saa7126_initdata {

	u8 board_id;
	u8 mode_id;
	u8 reg;
	u8 len;
	u8 buf[28];

} __attribute__ ((packed));

static struct saa7126_initdata saa7126_inittab [] =
{
	{ 0x07, 0x01, 0x26,  8, { 0x00, 0x00, 0x21, 0x1d, 0x00, 0x00, 0x00, 0x0f } },	/* common - pal */
	{ 0x07, 0x02, 0x26,  8, { 0x00, 0x00, 0x19, 0x1d, 0x00, 0x00, 0x00, 0x0f } },	/* common - ntsc */

        /* gain luminance, gain colour difference */
	{ 0x07, 0x03, 0x38,  3, { 0x1a, 0x1a, 0x03 } },

	{ 0x01, 0x03, 0x54,  1, { 0x03 } },						/* nokia - common */
	{ 0x06, 0x03, 0x54,  1, { 0x00 } },						/* philips, sagem - common */

	{ 0x07, 0x01, 0x55, 11, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x6b, 0x7d, 0xaf,	/* common - pal */
	                          0x33, 0x35, 0x35 } },
	{ 0x07, 0x02, 0x55, 11, { 0x00, 0x00, 0x00, 0x00, 0x00, 0xa3, 0x7d, 0xaf,	/* common - ntsc */
	                          0x33, 0x35, 0x35 } },

	{ 0x07, 0x01, 0x61, 28, { 0x06, 0x2f, 0xcb, 0x8a, 0x09, 0x2a, 0x00, 0x00,	/* common - pal */
	                          0x00, 0x00, 0x52, 0x28, 0x01, 0x20, 0x31, 0x7d,
	                          0xbb, 0x60, 0x42, 0x05, 0x00, 0x05, 0x16, 0x04,
	                          0x16, 0x16, 0x36, 0x60 } },
	{ 0x07, 0x02, 0x61, 28, { 0x04, 0x43, 0x1f, 0x7c, 0xf0, 0x21, 0x00, 0x00,	/* common - ntsc */
	                          0x00, 0x00, 0x52, 0x28, 0x01, 0x20, 0x31, 0x7d,
	                          0xbb, 0x60, 0x54, 0x05, 0x00, 0x06, 0x10, 0x05,
	                          0x10, 0x16, 0x36, 0xe0 } },

	{ 0x07, 0x03, 0x7d,  3, { 0x00, 0x00, 0x00 } },					/* common - common */

	{ 0xff },									/* end */
};

void saa7126_init (char mid)
{
	int i;
	u8 board_id = 1 << (mid - 1);
	u8 mode_id = 1;

	for (i = 0; saa7126_inittab[i].board_id != 0xff; i++)
		if ((saa7126_inittab[i].board_id & board_id) && (saa7126_inittab[i].mode_id & mode_id))
			i2c_write (0x44, saa7126_inittab[i].reg, 0x01, saa7126_inittab[i].buf, saa7126_inittab[i].len);
}

#endif
