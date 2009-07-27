/*
 * lcd.c: lcd driver for KS0713 and compatible
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
 * $Id: lcd.c,v 1.5 2009/07/27 14:39:01 rhabarber1848 Exp $
 */

#include <common.h>
#include <version.h>

#ifdef CONFIG_LCD_BOARD

#include "lcd.h"

#ifdef CONFIG_DBOX2_LCD_LOGO_FS
#include <cmd_fs.h>
#endif /* CONFIG_DBOX2_LCD_LOGO_FS */

#if defined(CONFIG_DBOX2_LCD_LOGO_TFTP) || defined (CONFIG_DBOX2_LCD_LOGO_NFS)
#include <net.h>
#endif /* CONFIG_DBOX2_LCD_LOGO_TFTP */

#ifdef CONFIG_DBOX2_LCD_FONT8x16
#include "font_8x16.h"
#else /* CONFIG_DBOX2_LCD_FONT8x16 */
#include "font_8x8.h"
#endif /* CONFIG_DBOX2_LCD_FONT8x16 */

#define LCD_DELAY				1

/* Read display data 11XXXXXXXX
 */
#define LCD_READ_DATA		0x0600

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
#define LCD_CMD_ON		0x00AE

/* Initial display line	0001FEDCBA
 * F-A ST5-0
 * Specify DDRAM line for COM1
 */
#define LCD_CMD_IDL		0x0040

/* Set reference voltage mode 0010000001
 */
#define LCD_CMD_SRV		0x0081

/* Set page address	001011DCBA
 * D-A P3-P1
 */
#define LCD_CMD_SPAGE		0x00B0

/* Set column address 000001DCBA
 * D-A  Y7-4
 * next Y3-0			
 */
#define LCD_CMD_COL		0x0010

/* ADC select 001010011A
 * A 0=normal direction 1=reverse
 */
#define LCD_CMD_ADC		0x00A0

/* Reverse display ON/OFF 001010011R
 * R 0=normal 1=reverse
 */
#define	LCD_CMD_REVERSE		0x00A6

/* Entire display ON/OFF 001010010E
 * E 0=normal 1=entire
 */
#define LCD_CMD_EON		0x00A4

/* LCD bias select 001010001B
 * B Bias
 */
#define LCD_CMD_BIAS		0x00A2

/* Set modify-read 0011100000
 */
#define LCD_CMD_SMR		0x00E0

/* Reset modify-read 0011101110
 */
#define LCD_CMD_RMR		0x00EE

/* Reset 0011100010
 * Initialize the internal functions
 */
#define LCD_CMD_RESET		0x00E2

/* SHL select 001100SXXX
 * S 0=normal 1=reverse
 */
#define LCD_CMD_SHL		0x00C0

/* Power control 0000101CRF
 * control power circuite operation
 */
#define LCD_CMD_POWERC		0x0028

/* Regulator resistor select 0000100CBA
 * select internal resistor ration
 */
#define LCD_CMD_RES		0x0020

/* Set static indicator 001010110S
 * S 0=off 1=on
 * next 00XXXXXXBC
 */
#define LCD_CMD_SIR		0x00AC

/* clocks */
#define LCD_CLK_LO		0x0000
#define LCD_CLK_HI		0x0800
#define LCD_CMD_LO		0x0800
#define LCD_CMD_HI		0x0B00

/* direction mode */
#define LCD_DIR_READ		0x0F00
#define LCD_DIR_WRITE		0x0FFF

volatile iop8xx_t * iop;

static short console_col = 0;
static short console_row = 0;

#if CONFIG_DBOX2_LCD_LOGO
#define	LCD_CONSOLE_COLS (LCD_COLS / FONT_WIDTH)
#define	LCD_CONSOLE_ROWS CONFIG_DBOX2_LCD_LOGO_RESERVE
#define	LCD_CONSOLE_ROWS_OFFSET (LCD_ROWS / FONT_HEIGHT - CONFIG_DBOX2_LCD_LOGO_RESERVE)
#else /* CONFIG_DBOX2_LCD_LOGO */
#define	LCD_CONSOLE_COLS (LCD_COLS / FONT_WIDTH)
#define	LCD_CONSOLE_ROWS (LCD_ROWS / FONT_HEIGHT)
#define	LCD_CONSOLE_ROWS_OFFSET 0
#endif /* CONFIG_DBOX2_LCD_LOGO */

static void lcd_drawchars (unsigned short x, unsigned short y, unsigned char *str, int count);

/* set direction to read */

static void lcd_set_port_read (void)
{
	iop->iop_pddir = LCD_DIR_READ;
}

/* set direction to write */

static void lcd_set_port_write (void)
{
	iop->iop_pddat = 0x0B00;
	iop->iop_pddir = LCD_DIR_WRITE;
	udelay(LCD_DELAY);
}

/* send cmd */

static void lcd_send_cmd (int cmd, int flag)
{
	lcd_set_port_write ();

	iop->iop_pddat = cmd | flag | LCD_CMD_LO;
	udelay(LCD_DELAY);
	iop->iop_pddat = cmd | flag | LCD_CMD_HI;
	udelay(LCD_DELAY);
}

static void lcd_set_pos (int row, int col)
{
	lcd_send_cmd (LCD_CMD_SPAGE, row);
	lcd_send_cmd (LCD_CMD_COL, (col >> 4) & 0x0F);
	lcd_send_cmd (0x00, col & 0x0F);
}

/* read status of display */

/*static int lcd_read_status (void)
{
	lcd_set_port_read ();

	iop->iop_pddat = LCD_READ_STATUS | LCD_CLK_LO;
	iop->iop_pddat = LCD_READ_STATUS | LCD_CLK_HI;

	return (iop->iop_pddat & 0xF0);
}*/

/* read byte */

static int lcd_read_byte (void)
{
	lcd_set_port_read ();

	iop->iop_pddat = LCD_READ_DATA | LCD_CLK_LO;
	udelay(LCD_DELAY);
	iop->iop_pddat = LCD_READ_DATA | LCD_CLK_HI;
	udelay(LCD_DELAY);

	return (iop->iop_pddat & 0xFF);
}

/* write byte */

static void lcd_write_byte (int data)
{
	lcd_set_port_write ();

	// write data
	iop->iop_pddat = LCD_WRITE_DATA | (data & 0xFF) | LCD_CLK_LO;
	udelay(LCD_DELAY);
	iop->iop_pddat = LCD_WRITE_DATA | (data & 0xFF) | LCD_CLK_HI;
	udelay(LCD_DELAY);
	iop->iop_pddat = LCD_WRITE_DATA | (data & 0xFF) | LCD_CLK_LO;
}

static void lcd_clear (void)
{
	int i, j;

	for (i = 0; i < LCD_ROWS / 8; i++)
	{
		lcd_set_pos (i, 0);
		for (j = 0; j < LCD_COLS; j++)
			lcd_write_byte (0);
	}
}

static void lcd_reset_init (void)
{
	unsigned char *hwi = (unsigned char *) (CONFIG_SYS_FLASH_BASE + CONFIG_SYS_HWINFO_OFFSET);

	char *s;
	int lcd_contrast = 15;
	int lcd_inverse = 0;
	int lcd_bias = 0;

	if ((s = getenv("lcd_contrast")) != NULL)
	{
		lcd_contrast = (int)simple_strtoul(s,NULL,8);
	}
	if ((s = getenv("lcd_inverse")) != NULL)
	{
		lcd_inverse = (int)simple_strtoul(s,NULL,8);
	}
	if ((s = getenv("lcd_bias")) != NULL)
	{
		lcd_bias = (int)simple_strtoul(s,NULL,8);
	}

	lcd_send_cmd (LCD_CMD_RESET, 0);

	udelay (1000*100);

	lcd_send_cmd (LCD_CMD_ON, 1);
	lcd_send_cmd (LCD_CMD_RES, 7);
	lcd_send_cmd (LCD_CMD_SRV, 1);
	lcd_send_cmd (0x00, lcd_contrast);

	switch (hwi[0])
	{
		case 2:
			lcd_send_cmd (LCD_CMD_BIAS, lcd_bias == 0 ? 0 : 1); // Philips: rev 2 LCDs need 1, rev 4 need 0
			break;
		default:
			lcd_send_cmd (LCD_CMD_BIAS, 1);
			break;
	}	

	lcd_send_cmd (LCD_CMD_POWERC, 7);
	lcd_send_cmd (LCD_CMD_SIR, 3);
	lcd_send_cmd (LCD_CMD_ADC, 0);
	lcd_send_cmd (LCD_CMD_SHL, 0);
	lcd_send_cmd (LCD_CMD_EON, 0);
	lcd_send_cmd (LCD_CMD_REVERSE, lcd_inverse);
	lcd_send_cmd (LCD_CMD_IDL, 0);
}

void lcd_status (int y, unsigned char percent)
{
	int x;

	lcd_set_pos (y, 0);

	for (x = 0; x < 10; x++)
		lcd_write_byte (0xFF);

	if (percent > 0)
		for (x = 0; x < percent; x++)
			lcd_write_byte (0x81);
	for (x = 0; x < 110 - percent; x++)
		lcd_write_byte (0xFF);
}

int lcd_init (void)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;
	iop = &immr->im_ioport;

	lcd_reset_init ();
	lcd_clear ();

	printf ("LCD:   ready\n");

	return 0;
}

#ifdef CONFIG_DBOX2_LCD_LOGO
int lcd_load (void)
{
	unsigned char *lcd_logo = (unsigned char *) CONFIG_SYS_LOAD_ADDR;
	int size;
	int x, y, y2, pix;

#ifdef CONFIG_DBOX2_LCD_LOGO_FS
	size = fs_fsload ((unsigned long) lcd_logo, CONFIG_DBOX2_LCD_LOGO_FS);

	if (size <= 0)
	{
# ifdef CONFIG_DBOX2_LCD_LOGO_TFTP
		printf ("ready - can't find logo in flash - try network\n");
# else /* CONFIG_DBOX2_LCD_LOGO_TFTP */
		printf ("ready - can't find logo in flash\n");
		return 0;
# endif /* CONFIG_DBOX2_LCD_LOGO_TFTP */
	}
	else
		goto load_logo;
#endif /* CONFIG_DBOX2_LCD_LOGO_FS */
#if defined(CONFIG_DBOX2_LCD_LOGO_TFTP) || defined (CONFIG_DBOX2_LCD_LOGO_NFS)
	NetLoop (BOOTP);
	netboot_update_env();
#ifdef CONFIG_DBOX2_LCD_LOGO_TFTP
	copy_filename (BootFile, CONFIG_DBOX2_LCD_LOGO_TFTP, sizeof (BootFile));
	size = NetLoop (TFTP);
#elif defined(CONFIG_DBOX2_LCD_LOGO_NFS)
	ip_to_string (NetServerIP, BootFile);
	strncat(BootFile, ":", sizeof (BootFile));
	strncat(BootFile, NetOurRootPath, sizeof (BootFile));
	strncat(BootFile, CONFIG_DBOX2_LCD_LOGO_NFS, sizeof (BootFile));
	size = NetLoop (NFS);
#else
	size = -1;
#endif
	if (size <= 0)
	{
		printf ("can't find LCD logo\n");
		return 0;
	}
	else
		goto load_logo;
#endif /* CONFIG_DBOX2_LCD_LOGO_TFTP */
load_logo:
	for (y = 0; y < (LCD_ROWS - CONFIG_DBOX2_LCD_LOGO_RESERVE * FONT_HEIGHT) / 8; y++)
	{
		lcd_set_pos (y, 0);

		for (x = 0; x < LCD_COLS; x++)
		{
			pix = 0;

			for (y2 = 0; y2 < 8; ++y2)
				pix |= ((lcd_logo[(y * 8 + y2) * LCD_COLS + x] >> (7 - y2)) & 1) << y2;

			lcd_write_byte(pix);
                }
	}

#ifdef CONFIG_DBOX2_LCD_INFO
# if CONFIG_DBOX2_LCD_LOGO_RESERVE >= 1
	lcd_puts (U_BOOT_VERSION_SHORT);
# endif /* CONFIG_DBOX2_LCD_LOGO_RESERVE */
#endif /* CONFIG_DBOX2_LCD_INFO */

	return 0;
}
#endif /* CONFIG_DBOX2_LCD_LOGO */

static void console_scrollup (void)
{
	int i, j;
	unsigned char buf[LCD_COLS];

	for (i = LCD_CONSOLE_ROWS_OFFSET; i < (LCD_ROWS - FONT_HEIGHT) / 8; i++)
	{
#if FONT_HEIGHT == 8
		lcd_set_pos (i + 1, 0);
#elif FONT_HEIGHT == 16
		lcd_set_pos (i + 2, 0);
#endif /* FONT_HEIGHT */
		/* dummy */
		lcd_read_byte ();
		for (j = 0; j < LCD_COLS; j++)
			buf[j] = lcd_read_byte ();
		lcd_set_pos (i, 0);
		for (j = 0; j < LCD_COLS; j++)
			lcd_write_byte (buf[j]);
	}

	for (i = (LCD_ROWS - FONT_HEIGHT) / 8; i < LCD_ROWS / 8; i++)
	{
		lcd_set_pos (i, 0);
		for (j = 0; j < LCD_COLS; j++)
			lcd_write_byte (0);
	}
}

static inline void console_back (void)
{
	unsigned char *w = (unsigned char *)' ';

	if (--console_col < 0) {
		console_col = LCD_CONSOLE_COLS-1 ;
		if (--console_row < 0)
			console_row = 0;
	}

	lcd_drawchars (console_col, console_row, w, 1);
}

static inline void console_newline (void)
{
	++console_row;
	console_col = 0;

	/* Check if we need to scroll the terminal */
	if (console_row >= LCD_CONSOLE_ROWS) {
		/* Scroll everything up */
		console_scrollup () ;
		--console_row;
	}
}

void lcd_putc (const char c)
{
	switch (c)
	{
		case '\r':
			console_col = 0;
			return;
		case '\n':
			console_newline ();
			return;
		case '\t':
			/* Tab (8 chars alignment) */
			console_col |=  8;
			console_col &= ~7;
			return;
		case '\b':
			console_back ();
			return;
		default:
			if (console_col >= LCD_CONSOLE_COLS)
				console_newline();

			lcd_drawchars (console_col++, console_row, (unsigned char *) &c, 1);
			return;
	}
}

void lcd_puts (const char *s)
{
	while (*s)
		lcd_putc (*s++);
}

void lcd_printf (const char *fmt, ...)
{
	va_list args;
	char buf[CONFIG_SYS_PBSIZE];

	va_start (args, fmt);
	vsprintf (buf, fmt, args);
	va_end (args);

	lcd_puts (buf);
}

static void lcd_drawchars (unsigned short x, unsigned short y, unsigned char *str, int count)
{
	unsigned short col = x * FONT_WIDTH, row = (LCD_CONSOLE_ROWS_OFFSET + y) * FONT_HEIGHT;
	unsigned short font_col, font_row;
	int i;
	unsigned char *s = str;
	unsigned char c;
#if FONT_HEIGHT == 8
	unsigned char w;
#elif FONT_HEIGHT == 16
	unsigned short w;
#endif /* FONT_HEIGHT */
	
	for (i = 0; i < count; ++i)
	{
		c = *s++;

		for (font_col = 0; font_col < FONT_WIDTH; font_col++)
		{
			w = 0;
			for (font_row = 0; font_row < FONT_HEIGHT; ++font_row)
				w |= ((fontdata[c * FONT_HEIGHT + font_row] >> (FONT_WIDTH - font_col)) & 1) << font_row;

			lcd_set_pos (row / 8, col + font_col);
#if FONT_HEIGHT == 8
			lcd_write_byte (w);
#elif FONT_HEIGHT == 16
			lcd_write_byte (w & 0xff);
			lcd_set_pos (row / 8 + 1, col + font_col);
			lcd_write_byte (w >> 8);
#endif /* FONT_HEIGHT */
		}
		col += FONT_WIDTH;
	}
}

#endif /* CONFIG_LCD_BOARD */
