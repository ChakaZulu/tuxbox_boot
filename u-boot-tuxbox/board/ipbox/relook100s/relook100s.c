
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
	*(ioptr)GPIO0_OR = *(ioptr)GPIO0_OR & ~0x04000000;

	/* set default value to extio
	 * bit0 : RFMOD_ON
	 * bit1 : 0V_12V_CTRL
	 * bit2 : NEN_22KHZ	(0:on, 1:off)
	 * bit3 : RST_DTUNER	(1:reset off, active high)
	 * bit4 : EN_LNB_DT
	 * bit5 : SEL_VER_HOR	(0:H, 1:V)
	 */
	*(unsigned char*)0xf2000000 = 0x80;

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
		puts ("relook100s");
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
	//return 0x02000000;

	//return 0x01600000;

#if 0
	if (mfdcr (sdram0_br1)==0)
		return 0x018a0000;
	else
		return 0x038a0000;
#endif

#if defined(CFG_MUTANT_EXTRA_MEM)
#warning "INFO: WARINING this is compiling for a SUPERMUTANT"
	return 0x038a0000;
#else
	return 0x018a0000;
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
