/*
 *   lcd-ks0713.c - lcd driver for KS0713 and compatible (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2001 Gillem (htoa@gmx.net)
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
 *
 *   $Log: lcd-ks0713.c,v $
 *   Revision 1.3  2001/04/25 19:41:29  Jolt
 *   IdxFs fixes, lcd logo from flash
 *
 *   Revision 1.2  2001/04/23 14:15:06  Jolt
 *   Colorbar tests, lcd fixes
 *
 *   Revision 1.1  2001/04/23 10:01:06  Jolt
 *   LCD support
 *
 *   Revision 1.10  2001/03/03 18:00:28  waldi
 *   complete change to devfs; doesn't compile without devfs
 *
 *   Revision 1.9  2001/01/28 19:47:12  gillem
 *   - fix setpos ...
 *
 *   Revision 1.8  2001/01/28 18:49:08  gillem
 *   add ioctl
 *   LCD_IOCTL_CLEAR
 *   LCD_IOCTL_SET_POS
 *   LCD_IOCTL_GET_POS
 *
 *   Revision 1.7  2001/01/26 23:51:33  gillem
 *   some kernel styles change
 *
 *   Revision 1.6  2001/01/20 19:01:21  gillem
 *   - add pixel function
 *
 *   Revision 1.5  2001/01/06 10:06:35  gillem
 *   cvs check
 *
 *   $Revision: 1.3 $
 *
 */

#include <ppcboot.h>
#include "mpc8xx.h"
#include "lcd-ks0713.h"
#include <idxfs.h>

///////////////////////////////////////////////////////////////////////////////

volatile iop8xx_t * iop;

///////////////////////////////////////////////////////////////////////////////

/* Front panel LCD display major */
#define LCD_DELAY 				1

///////////////////////////////////////////////////////////////////////////////

/* internal functions ... first read the manual ! (not included) */

/* Read display data 11XXXXXXXX
 */
#define LCD_READ_DATA			0x0600

/* Write display data 10XXXXXXXX
 */
#define LCD_WRITE_DATA		0x0400

/* Read Status 01BAOR0000
 * B BUSY
 * A ADC
 * O ON/OFF
 * R RESETB
 */
#define LCD_READ_STATUS		0x0200

/* Display ON/OFF	001010111O
 * O 0=OFF 1=ON
 */
#define LCD_CMD_ON				0x00AE

/* Initial display line	0001FEDCBA
 * F-A ST5-0
 * Specify DDRAM line for COM1
 */
#define LCD_CMD_IDL				0x0040

/* Set reference voltage mode 0010000001
 */
#define LCD_CMD_SRV				0x0081

/* Set page address	001011DCBA
 * D-A P3-P1
 */
#define LCD_CMD_SPAGE			0x00B0

/* Set column address 000001DCBA
 * D-A  Y7-4
 * next Y3-0			
 */
#define LCD_CMD_COL				0x0010

/* ADC select 001010011A
 * A 0=normal direction 1=reverse
 */
#define LCD_CMD_ADC				0x00A0

/* Reverse display ON/OFF 001010011R
 * R 0=normal 1=reverse
 */
#define	LCD_CMD_REVERSE		0x00A6

/* Entire display ON/OFF 001010010E
 * E 0=normal 1=entire
 */
#define LCD_CMD_EON				0x00A4

/* LCD bias select 001010001B
 * B Bias
 */
#define LCD_CMD_BIAS			0x00A2

/* Set modify-read 0011100000
 */
#define LCD_CMD_SMR				0x00E0

/* Reset modify-read 0011101110
 */
#define LCD_CMD_RMR				0x00EE

/* Reset 0011100010
 * Initialize the internal functions
 */
#define LCD_CMD_RESET			0x00E2

/* SHL select 001100SXXX
 * S 0=normal 1=reverse
 */
#define LCD_CMD_SHL				0x00C0

/* Power control 0000101CRF
 * control power circuite operation
 */
#define LCD_CMD_POWERC		0x0028

/* Regulator resistor select 0000100CBA
 * select internal resistor ration
 */
#define LCD_CMD_RES				0x0020 // Regulator resistor select

/* Set static indicator 001010110S
 * S 0=off 1=on
 * next 00XXXXXXBC
 */
#define LCD_CMD_SIR				0x00AC

/* clocks */
#define LCD_CLK_LO			0x0000
#define LCD_CLK_HI			0x0800
#define LCD_CMD_LO			0x0800
#define LCD_CMD_HI			0x0B00

/* direction mode */
#define LCD_DIR_READ		0x0F00
#define LCD_DIR_WRITE		0x0FFF

///////////////////////////////////////////////////////////////////////////////
/* set direction to read */

static void lcd_set_port_read(void)
{
	iop->iop_pddir = LCD_DIR_READ;
}

///////////////////////////////////////////////////////////////////////////////
/* set direction to write */

static void lcd_set_port_write(void)
{
	iop->iop_pddat = 0x0B00;
	iop->iop_pddir = LCD_DIR_WRITE;
	//udelay(LCD_DELAY);
}

///////////////////////////////////////////////////////////////////////////////
/* send cmd */

static void lcd_send_cmd( int cmd, int flag )
{
		lcd_set_port_write();

		iop->iop_pddat = cmd | flag | LCD_CMD_LO;
		//udelay(LCD_DELAY);
		iop->iop_pddat = cmd | flag | LCD_CMD_HI;
		//udelay(LCD_DELAY);
}

///////////////////////////////////////////////////////////////////////////////
/* read status of display */

static int lcd_read_status(void)
{
	int status;

	lcd_set_port_read();

	iop->iop_pddat = LCD_READ_STATUS | LCD_CLK_LO;
	//udelay(LCD_DELAY);
	iop->iop_pddat = LCD_READ_STATUS | LCD_CLK_HI;
	//udelay(LCD_DELAY);

	status = (iop->iop_pddat & 0xF0);

	return status;
}

///////////////////////////////////////////////////////////////////////////////
/* read function */

static void lcd_read_dummy(void)
{
	lcd_set_port_read();

	// write data
	iop->iop_pddat = LCD_READ_DATA | LCD_CLK_LO;
	//udelay(LCD_DELAY);
	iop->iop_pddat = LCD_READ_DATA | LCD_CLK_HI;
	//udelay(LCD_DELAY);
}

///////////////////////////////////////////////////////////////////////////////
/* read byte */

static int lcd_read_byte(void)
{
	int data;

	lcd_read_dummy();

	data = (iop->iop_pddat & 0xFF);

	return data;
}

///////////////////////////////////////////////////////////////////////////////
/* write byte */

void lcd_write_byte( int data )
{
	lcd_set_port_write();

	// write data
	iop->iop_pddat = LCD_WRITE_DATA | (data&0xFF) | LCD_CLK_LO;
	//udelay(LCD_DELAY);
	iop->iop_pddat = LCD_WRITE_DATA | (data&0xFF) | LCD_CLK_HI;
	//udelay(LCD_DELAY);
	iop->iop_pddat = LCD_WRITE_DATA | (data&0xFF) | LCD_CLK_LO;
}

///////////////////////////////////////////////////////////////////////////////

int lcd_set_pixel( struct lcd_pixel * pix )
{
    int y=0,val,bit=0,v;

    if ( pix->y >= (LCD_ROWS*8) ) {
        return -1;
    }

    if ( pix->x >= LCD_COLS ) {
        return -1;
    }

    if (pix->y) {
        y = (pix->y/8);
    }

    bit = pix->y - (y*8);

	// set dram pointer
	lcd_set_pos( y, pix->x );
	lcd_read_dummy();

    val = lcd_read_byte();

    v = pix->v;

    // invertieren
    if ( v == 2 ) {
        v = (((~val)>>bit)&1);
    }

    if ( v == 1 ) {
        val |= (1<<bit);
    } else {
        val &= ~(1<<bit);
    }

    // set dram pointer
    lcd_set_pos( y, pix->x );
    lcd_write_byte(val);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void lcd_read_dram( unsigned char * dest )
{
	int pa,col;

	for(pa=0;pa<LCD_ROWS;pa++) {
		// set dram pointer
		lcd_set_pos( pa, 0 );

		lcd_read_dummy();

		for(col=0;col<LCD_COLS;col++) {
			dest[(pa*LCD_COLS)+col] = lcd_read_byte();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void lcd_write_dram( unsigned char * source )
{
	int pa,col;
	
	for(pa=0;pa<LCD_ROWS;pa++) {

		lcd_set_pos( pa, 0 );

		for(col=0;col<LCD_COLS;col++) {
			lcd_write_byte( source[(pa*LCD_COLS)+col] );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void lcd_set_pos( int row, int col )
{
   lcd_send_cmd( LCD_CMD_SPAGE, row );
   lcd_send_cmd( LCD_CMD_COL, (col>>4)&0x0F );
   lcd_send_cmd( 0x00, col&0x0F );
}

///////////////////////////////////////////////////////////////////////////////

void lcd_clear(void)
{
	static unsigned char d[LCD_BUFFER_SIZE];

	memset(d,0x00,LCD_BUFFER_SIZE);
	lcd_write_dram(d);
}

///////////////////////////////////////////////////////////////////////////////

void lcd_reset(void)
{
    // TODO: not work :-/
    lcd_send_cmd( LCD_CMD_RESET, 0 );
    udelay(1000*100);
	lcd_send_cmd( LCD_CMD_ON, 1 );
	lcd_send_cmd( LCD_CMD_EON, 0 );
	lcd_send_cmd( LCD_CMD_REVERSE, 0 );
	lcd_send_cmd( LCD_CMD_BIAS, 1 );
	lcd_send_cmd( LCD_CMD_ADC, 0 );
	lcd_send_cmd( LCD_CMD_SHL, 0 );
	lcd_send_cmd( LCD_CMD_POWERC, 7 );
	lcd_send_cmd( LCD_CMD_RES, 7 );
	lcd_send_cmd( LCD_CMD_SIR, 3 );
	lcd_send_cmd( LCD_CMD_SRV, 50 );
}

///////////////////////////////////////////////////////////////////////////////

int lcd_init(void)
{
    immap_t	*immap;
    int i;
    lcd_pixel p;
    unsigned char *lcd_logo;
    unsigned int size, offset = 0;


   printf("  LCD driver (KS0713) initialized\n");

  if ( ( immap = ( immap_t * ) CFG_IMMR ) == NULL )
    return -1;

	iop = (iop8xx_t *)&immap->im_ioport;

    /* reset lcd todo ;-) */
//    lcd_reset();

	   
    idxfs_file_info((unsigned char*)0x10040000, -1, "logo-lcd", &offset, &size);
    
    if (!offset) {
    
      printf("  LCD logo at: none\n");
    
      return 0;
      
    }
      
    printf("  LCD logo at: 0x%X (0x%X bytes)\n", offset, size);
    
    lcd_logo = (unsigned char*)(0x10040000 + offset);

    p.x = 0;
    p.y = 0;

    for (i=0; i<7200; i++) {

      p.v = lcd_logo[i] & 1;
      lcd_set_pixel(&p);

      p.x++;
      if (p.x >= 120) {
        p.x = 0;
       p.y++;
      }

    }


   return 0;
   
}

///////////////////////////////////////////////////////////////////////////////

