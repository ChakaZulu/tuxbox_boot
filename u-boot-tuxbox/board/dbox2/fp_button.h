/*
 * $Id: fp_button.h,v 1.1 2009/07/11 22:26:41 houdini Exp $
 *
 * Copyright (C) 2009 by Harald Küthe <harald-tuxbox@arcor.de>
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

#define FP_GETID		0x1D
#define FP_I2C_ID		(0x60>>1)

#define KEY_RESERVED		0
#define KEY_UP			103
#define KEY_DOWN		108
#define KEY_POWER		116

int fp_get_button(void);
int fp_button_init(void);
