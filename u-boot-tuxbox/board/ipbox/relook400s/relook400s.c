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

#define GPIO_BASE	0x40060000

#include <common.h>
#include <asm/processor.h>
#include <ppc4xx.h>

#include "common/front_dev.h"

typedef volatile unsigned long* ioptr;

int misc_init_f (void)
{
	front_init();

	/* release reset ethernet chip */
	*(ioptr)GPIO0_OR = *(ioptr)GPIO0_OR & ~0x00002000;

	return 0;
}


/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	unsigned char *s = getenv ("serial#");
	unsigned char *e;

	/* We have to change here.... */

	puts ("Board: ");

	if ( !s ) {
		puts ("relook400s(?)");
	} else {
		for (e = s; *e; ++e) {
			if (*e == ' ')
				break;
		}
		for (; s < e; ++s) {
			putc (*s);
		}
	}
	puts ("\n");

	return (0);
}


/* -------------------------------------------------------------------------
  initdram(int board_type) reads EEPROM via I2c. EEPROM contains all of
  the necessary info for SDRAM controller configuration
   ------------------------------------------------------------------------- */
long int initdram (int board_type)
{
#if 0
	return  spd_sdram (0);
#else
        return 0x038a0000;

	return 0x04000000;	// We ware already initilized sdram.
#endif
}

/* ------------------------------------------------------------------------- */

int testdram (void)
{
	/* TODO: XXX XXX XXX */
	puts ("test: xxx MB - ok\n");

	return (0);
}

/* ------------------------------------------------------------------------- */
