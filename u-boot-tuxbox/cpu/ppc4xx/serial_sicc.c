/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*------------------------------------------------------------------------------+ */
/*
 * This source code has been made available to you by IBM on an AS-IS
 * basis.  Anyone receiving this source is licensed under IBM
 * copyrights to use it in any way he or she deems fit, including
 * copying it, modifying it, compiling it, and redistributing it either
 * with or without modifications.  No license under IBM patents or
 * patent applications is to be implied by the copyright license.
 *
 * Any user of this software should understand that IBM cannot provide
 * technical support for this software and will not be responsible for
 * any consequences resulting from the use of this software.
 *
 * Any person who transfers this source code or any derivative work
 * must include the IBM copyright notice, this paragraph, and the
 * preceding two paragraphs in the transferred software.
 *
 * COPYRIGHT   I B M   CORPORATION 1995
 * LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
 */
/*------------------------------------------------------------------------------- */

#include <common.h>
#include <commproc.h>
#include <asm/processor.h>
#include <watchdog.h>
#include "vecnum.h"

#ifdef CONFIG_SERIAL_SOFTWARE_FIFO
#include <malloc.h>
#endif

#include <ppc4xx.h>

#if defined(CONFIG_STB034xx) && defined(CFG_SICC_CONSOLE)

/*****************************************************************************/
#define SYS_CLK_FREQ	20500000

#define SICC_LSR	(*(unsigned char*)0x40000000)
#define SICC_HSR	(*(unsigned char*)0x40000002)
#define SICC_BRDH	(*(unsigned char*)0x40000004)
#define SICC_BRDL	(*(unsigned char*)0x40000005)
#define SICC_LCR	(*(unsigned char*)0x40000006)
#define SICC_RCR	(*(unsigned char*)0x40000007)
#define SICC_TxCR	(*(unsigned char*)0x40000008)
#define SICC_RBR	(*(unsigned char*)0x40000009)
#define SICC_TBR	(*(unsigned char*)0x40000009)
#define SICC_CLT2	(*(unsigned char*)0x4000000a)


int serial_init (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	int baud;

	baud = ((SYS_CLK_FREQ/gd->baudrate)>>4)-1;

	SICC_LCR = 0x38;
	SICC_RCR = 0x80;
	SICC_TxCR = 0x80;
	SICC_CLT2 = 0x80;
	SICC_BRDH = (baud&0xff00)>>8;
	SICC_BRDL = baud&0x00ff;

	return (0);
}

void serial_setbrg (void)
{
	DECLARE_GLOBAL_DATA_PTR;
	int baud;

	baud = ((SYS_CLK_FREQ/gd->baudrate)>>4)-1;

	SICC_BRDH = (baud&0xff00)>>8;
	SICC_BRDL = baud&0x00ff;
}


void serial_putc (const char c)
{
	int a;

	if( c == '\n' )
		serial_putc( '\r' );

	for( a=0; a<320; a++ )
	{
		if( (SICC_LSR&0x04) )
			break;
		udelay( 100 );
	}

	SICC_TBR = c;
}


void serial_puts (const char *s)
{
	while( *s )
	{
		serial_putc( *s );
		s++;
	}
}


int serial_getc ()
{
	while( (SICC_LSR & 0x80) == 0 );

	return SICC_RBR;
}


int serial_tstc ()
{
	return ((SICC_LSR&0x80)!=0);
}


#endif
