/*
 * $Id: fp_button.c,v 1.1 2009/07/11 22:26:41 houdini Exp $
 *
 * Copyright (C) 2009 by Harald Küthe <harald-tuxbox@arcor.de>
 * used code from dbox_fp_button.c
 * Copyright (C) 2002 by Florian Schirmer <jolt@tuxbox.org>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <common.h>
#include <i2c.h> 
#include "fp_button.h"

static u16 button_new_code_map[] = {

	/* 000-007 */	KEY_RESERVED, KEY_POWER, KEY_UP, KEY_RESERVED, KEY_DOWN, KEY_RESERVED, KEY_RESERVED,
	
};

static u16 button_old_code_map[] = {

	/* 000-007 */	KEY_RESERVED, KEY_POWER, KEY_DOWN, KEY_RESERVED, KEY_UP, KEY_RESERVED, KEY_RESERVED,
};


int fp_get_button(void)
{
	u8 button;
	u16 key;
	u8 pressed = 0;
	int ret;

	ret = i2c_read(FP_I2C_ID, 0x25, 1, (u8 *)&button, sizeof(button));

	if (button & 0x80) 
	{
		pressed = !!((((button >> 1) ^ 0x07) & 0x07) & ((button >> 4) & 0x07));
		key = button_old_code_map[(button >> 4) & 0x07];
		
	} else 
	{
		pressed = !!(((button >> 1) & 0x07) & ((button >> 4) & 0x07));
		key = button_new_code_map[(button >> 4) & 0x07];
	}

	if (key != 0) 
	{
		printf("%s %s\n", key == KEY_UP ? "KEY_UP" : (key == KEY_DOWN ? "KEY_DOWN" : (key == KEY_POWER ? "KEY_POWER" : "INVALID")), pressed == 1 ? "pressed" : "released");
	}
	if (ret == 0)
	{
		return (key | (pressed<<16));
	}
	else
	{
		return(-1);
	}
}

int fp_button_init(void)
{
	u8 id [] = { 0x00, 0x00, 0x00 };
	int ret;

	/*
	 * FP ID
	 * -------------
	 * NOKIA  : 0x5A
	 * SAGEM  : 0x52
	 * PHILIPS: 0x52
	 */

	ret = i2c_read(FP_I2C_ID, FP_GETID, 1, id, sizeof(id));

	if ((id[0] != 0x52) && (id[0] != 0x5a)) 
	{
		printf("fp.o: bogus fpID %d\n", id[0]);
		return -1;
	}
	else
	{
		printf("fp.o: fpID %d\n", id[0]);
		return 0;
	}
}
