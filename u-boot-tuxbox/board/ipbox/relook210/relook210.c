
#define GPIO_BASE	0x40060000

#include <common.h>
#include <asm/processor.h>
#include <ppc4xx.h>

#include "common/front_dev.h"

typedef volatile unsigned long* ioptr;

int misc_init_f (void)
{
	front_init();

	/* release ethernet chip reset */
	*(ioptr)GPIO0_OR = *(ioptr)GPIO0_OR & ~0x00000008;

	/* set default value to extio
	 * bit0 : HDD_ON
	 * bit1 : 0V_12V_CTRL
	 * bit2 : NEN_22KHZ	(0:on, 1:off)
	 * bit3 : RST_DTUNER	(1:reset off, active high)
	 * bit4 : EN_LNB_DT
	 * bit5 : SEL_VER_HOR	(0:H, 1:V)
	 */
	*(unsigned char*)0xf1000000 = 0x80 | 0x10 | 0x08;
	udelay(200000);
	*(ioptr)GPIO0_OR = *(ioptr)GPIO0_OR & (~0x01000000);	/*  ide reset on */
	udelay(300000);
	*(ioptr)GPIO0_OR = *(ioptr)GPIO0_OR | 0x01000000;	/* ide reset off */

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
		puts ("cubecafe-prime");
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
	// We ware already initilized sdram.
	// just return the size.
	return 0x02000000;
}

/* ------------------------------------------------------------------------- */

int testdram (void)
{
	/* TODO: XXX XXX XXX */
	puts ("test: xxx MB - ok\n");

	return (0);
}

void show_boot_progress(int status)
{
	printf("+++ boot : %d\n", status);
}
/* ------------------------------------------------------------------------- */
