/*
 *   fb-avia.c - fb driver for AVIA-GTX/ENX (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2001 
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *   
 *  $Log: fb-avia.c,v $
 *  Revision 1.2  2001/06/05 11:02:11  derget
 *
 *  test import
 *
 *
 *  $Revision: 1.2 $
 *
 */
#include <stdio.h>
#include <malloc.h>
#include "i2c.h"
#include "fb-avia.h"
#include <idxfs.h>

static int pal=1;
unsigned char *gtxmem, *gtxreg;

static unsigned char SAA7126_INIT[] = {

  0x00, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x80, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0f, 0xf7, 0xef, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x3f, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x21, 0x1d, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00,
  0x3f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0xff, 0x00,
  0x1a, 0x1a, 
  0x03, // normal mode
//  0x80, // colorbar mode
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x68, 0x7d, 0xaf, 0x33, 0x35, 0x35,
  0x00, 0x06, 0x2f, 0xcb, 0x8a, 0x09, 0x2a, 0x00,
  0x00, 0x00, 0x00, 0x52, 0x28, 0x01, 0x20, 0x31,
  0x7d, 0xbc, 0x60, 0x41, 0x05, 0x00, 0x06, 0x16,
  0x06, 0x16, 0x16, 0x36, 0x60, 0x00, 0x00, 0x00
  
};

void i2c_bus_init(void)
{
  i2c_init();
  i2c_setspeed(50000);
}

void saa7126_init(void)
{
  i2c_send(0x88, 0x00, 0x01, 0x40, SAA7126_INIT);
  i2c_send(0x88, 0x40, 0x01, 0x40, SAA7126_INIT + 0x40);
}

void avs_init(char mId)
{
  switch(mId) {
    case 1:
      i2c_send(0x90, 0, 0, 5, "\x0d\x29\xc9\xa9\x00");
      i2c_send(0x90, 0, 0, 5, "\x0f\x29\xc9\xa9\x80");
      i2c_send(0x90, 0, 0, 5, "\x0f\x2f\xcf\xaf\x80");
      i2c_send(0x90, 0, 0, 5, "\x0f\x29\xc9\xa9\x80");
      i2c_send(0x90, 0, 0, 5, "\x0d\x29\xc9\xa9\x00");
    break;
    case 2:
      i2c_send(0x94, 0, 0, 8, "\x00\x0a\x19\x99\xb5\x08\x34\x88");
    break;	
    case 3:
      i2c_send(0x90, 0, 0, 7, "\x00\x00\x00\x00\x00\x3F\x00");
    break;	
  }    
}

void gtxcore_init(void)
{
        int cr;
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
}

void gtxvideo_init(void)
{
        int val;
        rh(VCR)=0x340;
        rh(VHT)=pal?858:852;
        rh(VLT)=pal?(623|(21<<11)):(523|(18<<11));
    
        val=3<<30;
        val|=3<<24;
        val|=0<<20;
        val|=0<<16;
        val|=720*2;
  
        rw(GMR)=val;

        rh(CCR)=0x7FFF;
        rw(GVSA)=0;
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
}

#define XRES 720
#define YRES 576
extern int decodestillmpg(void *pic, const void *src, int X_RESOLUTION, int Y_RESOLUTION);
    
int fb_init(void)
{
  unsigned char mID = *(char*)(0x1001ffe0);
  unsigned int size, offset = 0;
  unsigned char *iframe_logo;
  idxfs_file_info((unsigned char*)IDXFS_OFFSET, 0, "logo-fb", &offset, &size);
  
    if (!offset) {
      printf("  FB logo at: none\n");
      return 0;
   }

  iframe_logo = (unsigned char*)(IDXFS_OFFSET + offset);
 

  i2c_bus_init();
  saa7126_init();

  switch (mID) {
    case 1:
      // NOKIA (GTX + CXA2092)
      gtxcore_init();
      printf("  FB driver (AVIA-GTX) initialized\n");
      decodestillmpg(gtxmem, iframe_logo, XRES, YRES);
      printf("  FB logo at: 0x%X (0x%X bytes)\n", offset, size);
      gtxvideo_init();
    break;
    case 2:
      // Philips (eNX + STV6411A)
    case 3:
      // SAGEM (eNX + CXA2126)
      //enxcore_init();
      //decodestillmpg(enx_get_mem_addr(), iframe_logo, XRES, YRES);
      //enxvideo_init();
    break;
  }
  
  avs_init(mID);
}
