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
 * $Id: avia.c,v 1.1 2004/05/01 23:38:34 carjay Exp $
 */

#include <common.h>
 
#ifdef CONFIG_DBOX2_FB

#include "avia.h"

extern int decodestillmpg (void *, const void *, int, int);

unsigned char chip_type;
unsigned char *mem_addr;
unsigned char *reg_addr;


static int pal=1;
unsigned char *gtxmem, *gtxreg;
unsigned char *enx_mem_addr = (unsigned char*)ENX_MEM_BASE;
unsigned char *enx_reg_addr = (unsigned char*)ENX_REG_BASE;

#define fboffset 1024*1024


void avia_init (int mid, unsigned char* fb_logo)
{
	int cr;
	unsigned long val;

	switch (mid)
	{
		case 1:
			gtxmem=(unsigned char*)GTX_PHYSBASE;
			gtxreg=gtxmem+0x400000;
			
			rh(RR0)=0xFFFF;
			rh(RR1)=0x00FF;
			rh(RR0)&=~((1<<13)|(1<<12)|(1<<10)|(1<<9)|(1<<6)|1);
			rh(RR0)=0;   
			rh(RR1)=0; 
			cr=rh(CR0);
			cr|=1<<11;
			cr&=~(1<<10);
			cr|=1<<9;
			cr&=~(1<<5);
			cr|=1<<3; 
			    
			cr&=~(3<<6);
			cr|=1<<6; 
			cr&=~(1<<2);
			rh(CR0)=cr;

			decodestillmpg(gtxmem + fboffset, fb_logo, 720, 576);

			rh(VCR)=0x340;
			rh(VHT)=pal?858:852;
			rh(VLT)=pal?(623|(21<<11)):(523|(18<<11));
			
			val=3<<30;   //16bit rgb
			val|=3<<24;  //chroma filter	
			val|=0<<20;  //BLEV1
			val|=0<<16;  //BLEV0
			val|=720*2;  //Stride
			
			rw(GMR)=val;
			
			rh(CCR)=0x7FFF;
			rw(GVSA)=fboffset;
			rh(GVP)=0;
			VCR_SET_HP(2);
			VCR_SET_FP(0);
			
			val=pal?127:117;
			val*=8;
			if (rw(GMR)&(1<<28))
			        val/=9;
			else
			        val/=8;
			
			val-=64;
			GVP_SET_COORD(val, pal?42:36);
			rh(GFUNC)=0x10;
			rh(TCR)=0xFC0F;
			GVS_SET_XSZ(720);
			GVS_SET_YSZ(576);
			rw(VBR)=(1<<24)|0x123456;               // hier nochmal schwarz einbauen...

/*			mem_addr = (unsigned char *) GTX_MEM_BASE;
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
*/
			break;
		case 2:
		case 3:
			enx_reg_w(RSTR0) = 0xFCF6BEFF;
			enx_reg_w(SCSC) = 0x00000000;
			enx_reg_w(RSTR0) &= ~(1 << 12);
			enx_reg_w(MC) = 0x00001015;
			enx_reg_w(RSTR0) &= ~(1 << 11);
			enx_reg_w(RSTR0) &= ~(1 << 9);

			enx_reg_w(CFGR0) |= 1 << 24;    // dac
			enx_reg_w(RSTR0) &= ~(1 << 20); // Get dac out of reset state
			enx_reg_h(DAC_PC) = 0x0000;     // dac auf 0
			enx_reg_h(DAC_CP) = 0x0009;     // dac

			decodestillmpg(enx_mem_addr + fboffset, fb_logo, 720, 576);

			enx_reg_w(VBR) = 0;
			enx_reg_h(VCR) = 0x40;
			enx_reg_h(VHT) = 857|0x5000;
			enx_reg_h(VLT) = 623|(21<<11);

			val=0;
			val|=1<<26;  
			val|=3<<20;  
			val|=(720*2);		

			enx_reg_w(GMR1)=val;
			enx_reg_w(GMR2)=0;
			enx_reg_h(GBLEV1) = 0;
			enx_reg_h(GBLEV2) = 0;
			enx_reg_w(GVSA1) = fboffset;
			enx_reg_w(GVP1) = 0;
			enx_reg_h(G1CFR)=0;
			enx_reg_h(G2CFR)=0;

#define ENX_GVP_SET_X(X)     enx_reg_w(GVP1) = ((enx_reg_w(GVP1)&(~(0x3FF<<16))) | ((X&0x3FF)<<16))
#define ENX_GVP_SET_Y(X)     enx_reg_w(GVP1) = ((enx_reg_w(GVP1)&(~0x3FF))|(X&0x3FF))
#define ENX_GVP_SET_COORD(X,Y) ENX_GVP_SET_X(X); ENX_GVP_SET_Y(Y)
#define ENX_GVS_SET_XSZ(X)   enx_reg_w(GVSZ1) = ((enx_reg_w(GVSZ1)&(~(0x3FF<<16))) | ((X&0x3FF)<<16))
#define ENX_GVS_SET_YSZ(X)   enx_reg_w(GVSZ1) = ((enx_reg_w(GVSZ1)&(~0x3FF))|(X&0x3FF))

			ENX_GVP_SET_COORD(113,42);
			ENX_GVS_SET_XSZ(720);
			ENX_GVS_SET_YSZ(576);

/*			mem_addr = (unsigned char *) ENX_MEM_BASE;
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
*/
			break;
	}
}

unsigned char *enx_get_reg_addr(void) {
    return enx_reg_addr;
}

#endif /* CONFIG_DBOX2_FB */
