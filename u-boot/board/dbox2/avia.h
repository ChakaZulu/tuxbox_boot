/*
 * avia.h
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
 * $Id: avia.h,v 1.1 2002/12/22 17:38:55 bastian Exp $
 */

#ifndef __DBOX2__FB__AVIA_H
#define __DBOX2__FB__AVIA_H

#define GTX_REG_BASE	0x08400000
#define GTX_MEM_BASE	0x08000000

#define GTX_REG_GMR	0x000
#define GTX_REG_GVSA	0x00C
#define GTX_REG_GVP	0x010
#define GTX_REG_GVS	0x014
#define GTX_REG_VCR	0x0F4
#define GTX_REG_VHT	0x0FA
#define GTX_REG_VLT	0x0FC
#define GTX_REG_RR0	0x100
#define GTX_REG_RR1	0x102
#define GTX_REG_CR0	0x104

#define ENX_REG_BASE	0x08000000
#define ENX_MEM_BASE	0x09000000

#define ENX_REG_RSTR0	0x0000
#define ENX_REG_CFGR0	0x0008
#define ENX_REG_SCSC	0x0084
#define ENX_REG_MC	0x0380
#define ENX_REG_GMR1	0x03C0
#define ENX_REG_GVSA1	0x03CC
#define ENX_REG_GVP1	0x03D0
#define ENX_REG_GVSZ1	0x03D4
#define ENX_REG_VBR	0x04C0
#define ENX_REG_VCR	0x04C4
#define ENX_REG_VHT	0x04CA
#define ENX_REG_VLT	0x04CC
#define ENX_REG_DAC_PC	0x0BC0
#define ENX_REG_DAC_CP	0x0BC2

#define gtx_reg_16(register) ((unsigned short)(*((unsigned short*)(reg_addr + GTX_REG_ ## register))))
#define gtx_reg_32(register) ((unsigned int)(*((unsigned int*)(reg_addr + GTX_REG_ ## register))))
#define enx_reg_16(register) ((unsigned short)(*((unsigned short*)(reg_addr + ENX_REG_ ## register))))
#define enx_reg_32(register) ((unsigned int)(*((unsigned int*)(reg_addr + ENX_REG_ ## register))))

#endif
