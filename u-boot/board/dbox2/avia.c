/*
 * avia.c
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
 * $Id: avia.c,v 1.2 2002/12/24 15:54:51 bastian Exp $
 */

#include <common.h>

#ifdef CONFIG_DBOX2_FB

#include "avia.h"

extern int decodestillmpg (void *, const void *, int, int);

unsigned char chip_type;
unsigned char *mem_addr;
unsigned char *reg_addr;

void avia_init_pre (int mid)
{
	unsigned long val;

	switch (mid)
	{
		case 1:
			mem_addr = (unsigned char *) GTX_MEM_BASE;
			reg_addr = (unsigned char *) GTX_REG_BASE;

			gtx_reg_16 (RR0) = 0xffff;
			gtx_reg_16 (RR1) = 0xff;
			gtx_reg_16 (RR0) = ~((1 << 13) | (1 << 10) | (1 << 0));

			val = gtx_reg_16 (CR0);
			val &= ~(3 << 6);
			val |= 1 << 6;
			val &= ~(1 << 5);
			val |= 1 << 3;
			val &= ~(1 << 2);
			gtx_reg_16 (CR0) = val;
			break;
		case 2:
		case 3:
			mem_addr = (unsigned char *) ENX_MEM_BASE;
			reg_addr = (unsigned char *) ENX_REG_BASE;

			enx_reg_32 (RSTR0) = 0xfffcffff;
			enx_reg_32 (SCSC) = 0x00000000;
			enx_reg_32 (RSTR0) = ~((1 << 12) | (1 << 11) | (1 << 9) | (1 << 7));
			enx_reg_32 (MC) = 0x00001011;

			if (mid == 3)
			{
				enx_reg_32 (CFGR0) |= 1 << 24;
				enx_reg_32 (RSTR0) &= ~(1 << 20);
				enx_reg_16 (DAC_PC) = 0x0000;
				enx_reg_16 (DAC_CP) = 0x0009;
			}
			break;
	}
}

void avia_init_load (unsigned char* fb_logo)
{
	decodestillmpg (mem_addr + 0x100000, fb_logo, 720, 576);
}

void avia_init_post (int mid)
{
	unsigned long val;

	switch (mid)
	{
		case 1:
			val = 3 << 30;
			val |= 3 << 24;
			val |= 720 * 2;
			gtx_reg_32 (GMR) = val;

			gtx_reg_32 (GVSA) = 0x100000;
			gtx_reg_32 (GVP) = ((127 * 8 % 8) << 27) | ((127 * 8 / 8 - 3 - 56) << 16) | (42 + 2);
			gtx_reg_32 (GVS) = (720 << 16) | 576;

			gtx_reg_16 (VCR) = 2 << 10;
			gtx_reg_16 (VHT) = 858;
			gtx_reg_16 (VLT) = 623 | (21 << 11);
			break;
		case 2:
		case 3:
			val = 3 << 20;
			val |= 1 << 26;
			val |= 720 * 2;
			enx_reg_32 (GMR1) = val;

			enx_reg_32 (GVSA1) = 0x100000;
			enx_reg_32 (GVP1) = ((118 - 3) << 16) | (42 + 2);
			enx_reg_32 (GVSZ1) = (720 << 16) | 576;

			enx_reg_16 (VHT) = 857 | (1 << 14) | (1 << 12);
			enx_reg_16 (VLT) = 623 | (21 << 11);
			break;
	}
}

#endif /* CONFIG_DBOX2_FB */
