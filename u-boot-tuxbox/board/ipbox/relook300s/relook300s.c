
#include <common.h>
#include <asm/processor.h>
#include <ppc4xx.h>

#include "relook300s/front_dev.h"

int misc_init_f( void )
{
	front_init();

	/*
	 * do hard disk reset.
	 */
	mtdcr( sgpo, 0x00100000 );

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

	if (!s) {
		puts ("relook300s(?)");
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
	return 0x01000000;	// We ware already initilized sdram.
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

