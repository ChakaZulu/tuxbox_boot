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

#include <common.h>
#include <commproc.h>
#include <mpc8xx.h>

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

const uint sdram_table[] = {
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0x1f07fc04, 0xeeaefc04, 0x11adfc04, 0xefbbbc00,
	0x1ff77c47,					/* last */
	/*
	 * SDRAM Initialization (offset 5 in UPMA RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
	0x1ff77c34, 0xefeabc34, 0x1fb57c35,	/* last */
	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 */
	0x1f07fc04, 0xeeaefc04, 0x10adfc04, 0xf0affc00,
	0xf0affc00, 0xf1affc00, 0xefbbbc00, 0x1ff77c47,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0x1f27fc04, 0xeeaebc00, 0x01b93c04, 0x1ff77c47,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 */
	0x1f07fc04, 0xeeaebc00, 0x10ad7c00, 0xf0affc00,
	0xf0affc00, 0xe1bbbc04, 0x1ff77c47,	/* last */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPMA RAM)
	 */
	0x1ff5fc84, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	0xfffffc84, 0xfffffc07,		/* last */
	_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPMA RAM)
	 */
	0x7ffffc07,					/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 *
 * Test ID string (IP860...)
 */

int checkboard (void)
{
	unsigned char *s, *e;
	unsigned char buf[64];
	int i;

	puts ("Board: ");

	i = getenv_r ("serial#", buf, sizeof (buf));
	s = (i > 0) ? buf : NULL;

	if (!s || strncmp (s, "IP860", 5)) {
		puts ("### No HW ID - assuming IP860");
	} else {
		for (e = s; *e; ++e) {
			if (*e == ' ')
				break;
		}

		for (; s < e; ++s) {
			putc (*s);
		}
	}

	putc ('\n');

	return (0);
}

/* ------------------------------------------------------------------------- */

long int initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size;

	upmconfig (UPMA, (uint *) sdram_table,
			   sizeof (sdram_table) / sizeof (uint));

	/*
	 * Preliminary prescaler for refresh
	 */
	memctl->memc_mptpr = 0x0400;

	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller banks 2 to the SDRAM address
	 */
	memctl->memc_or2 = CFG_OR2;
	memctl->memc_br2 = CFG_BR2;

	/* IP860 boards have only one bank SDRAM */


	udelay (200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mamr = 0xC3804114;
	memctl->memc_mcr = 0x80004105;	/* run precharge pattern from loc 5 */
	udelay (1);
	memctl->memc_mamr = 0xC3804118;
	memctl->memc_mcr = 0x80004130;	/* run refresh pattern 8 times */

	udelay (1000);

	/*
	 * Check SDRAM Memory Size
	 */
	size = dram_size (CFG_MAMR, (ulong *) SDRAM_BASE, SDRAM_MAX_SIZE);

	udelay (1000);

	memctl->memc_or2 = ((-size) & 0xFFFF0000) | SDRAM_TIMING;
	memctl->memc_br2 = (CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

	udelay (10000);

	/*
	 * Also, map other memory to correct position
	 */

#if (defined(CFG_OR1) && defined(CFG_BR1_PRELIM))
	memctl->memc_or1 = CFG_OR1;
	memctl->memc_br1 = CFG_BR1;
#endif

#if defined(CFG_OR3) && defined(CFG_BR3)
	memctl->memc_or3 = CFG_OR3;
	memctl->memc_br3 = CFG_BR3;
#endif

#if defined(CFG_OR4) && defined(CFG_BR4)
	memctl->memc_or4 = CFG_OR4;
	memctl->memc_br4 = CFG_BR4;
#endif

#if defined(CFG_OR5) && defined(CFG_BR5)
	memctl->memc_or5 = CFG_OR5;
	memctl->memc_br5 = CFG_BR5;
#endif

#if defined(CFG_OR6) && defined(CFG_BR6)
	memctl->memc_or6 = CFG_OR6;
	memctl->memc_br6 = CFG_BR6;
#endif

#if defined(CFG_OR7) && defined(CFG_BR7)
	memctl->memc_or7 = CFG_OR7;
	memctl->memc_br7 = CFG_BR7;
#endif

	return (size);
}

/* ------------------------------------------------------------------------- */

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */

static long int dram_size (long int mamr_value, long int *base,
						   long int maxsize)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	volatile long int *addr;
	ulong cnt, val;
	ulong save[32];				/* to make test non-destructive */
	unsigned char i = 0;

	memctl->memc_mamr = mamr_value;

	for (cnt = maxsize / sizeof (long); cnt > 0; cnt >>= 1) {
		addr = base + cnt;		/* pointer arith! */

		save[i++] = *addr;
		*addr = ~cnt;
	}

	/* write 0 to base address */
	addr = base;
	save[i] = *addr;
	*addr = 0;

	/* check at base address */
	if ((val = *addr) != 0) {
		*addr = save[i];
		return (0);
	}

	for (cnt = 1; cnt <= maxsize / sizeof (long); cnt <<= 1) {
		addr = base + cnt;		/* pointer arith! */

		val = *addr;
		*addr = save[--i];

		if (val != (~cnt)) {
			return (cnt * sizeof (long));
		}
	}
	return (maxsize);
}

/* ------------------------------------------------------------------------- */

void reset_phy (void)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	ulong mask = PB_ENET_RESET | PB_ENET_JABD;
	ulong reg;

	/* Make sure PHY is not in low-power mode */
	immr->im_cpm.cp_pbpar &= ~(mask);	/* GPIO */
	immr->im_cpm.cp_pbodr &= ~(mask);	/* active output */

	/* Set  JABD low  (no JABber Disable),
	 * and RESET high (Reset PHY)
	 */
	reg = immr->im_cpm.cp_pbdat;
	reg = (reg & ~PB_ENET_JABD) | PB_ENET_RESET;
	immr->im_cpm.cp_pbdat = reg;

	/* now drive outputs */
	immr->im_cpm.cp_pbdir |= mask;	/* output */
	udelay (1000);
	/*
	   * Release RESET signal
	 */
	immr->im_cpm.cp_pbdat &= ~(PB_ENET_RESET);
	udelay (1000);
}

/* ------------------------------------------------------------------------- */
