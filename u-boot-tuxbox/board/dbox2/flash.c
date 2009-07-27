/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2002 Bastian Blank <waldi@tuxbox.org>
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
#include <mpc8xx.h>
#include <flash.h>

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips */

/*-----------------------------------------------------------------------
 * Functions
 */
#ifndef CONFIG_DBOX2_FLASH_FAKE
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static void flash_get_offsets (ulong base, flash_info_t *info);
static void flash_get_protect (flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);
#endif /* CONFIG_DBOX2_FLASH_FAKE */

/*-----------------------------------------------------------------------
 */

#ifndef CONFIG_DBOX2_FLASH_FAKE
static void flash_put (flash_info_t *info, vu_long *addr, ulong offs, ulong val)
{
	vu_short *addr16 = (vu_short *) addr;

	if (info->portwidth == FLASH_CFI_16BIT)
		addr16[offs] = (ushort) (val & 0xFFFF);
	else
		addr[offs] = val;
}

static ulong flash_get (flash_info_t *info, vu_long *addr, ulong offs)
{
	vu_short *addr16 = (vu_short *) addr;

	if (info->portwidth == FLASH_CFI_16BIT)
		return (addr16[offs] & 0xFFFF);
	else
		return addr[offs];
}

static ulong flash_mask (flash_info_t *info, ulong val)
{
	if (info->portwidth == FLASH_CFI_16BIT)
		return (val & 0xFFFF);
	else
		return val;
}
#endif /* CONFIG_DBOX2_FLASH_FAKE */

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	int i;

#ifdef CONFIG_DBOX2_FLASH_FAKE
	flash_info[0].flash_id = 0x00600000;
	flash_info[0].sector_count = 0x40;
	flash_info[0].size = 0x800000;
	flash_info[0].start[0] = FLASH_BASE_PRELIM;

	for (i = 0; i < flash_info[0].sector_count; ++i)
	{
		flash_info[0].start[i] = FLASH_BASE_PRELIM + ( ( i + 1 ) * 0x20000 );
		flash_info[0].protect[i] = 1;
	}

	printf ("(faked) ");
	return 8 * 1024 * 1024;
#else /* CONFIG_DBOX2_FLASH_FAKE */
	unsigned long size;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i)
		flash_info[i].flash_id = FLASH_UNKNOWN;

	flash_info[0].portwidth = FLASH_CFI_32BIT;

	size = flash_get_size ((vu_long *) FLASH_BASE_PRELIM, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN)
	{
		flash_info[0].portwidth = FLASH_CFI_16BIT;
		size = flash_get_size ((vu_long *) FLASH_BASE_PRELIM, &flash_info[0]);
	}

	if (flash_info[0].flash_id == FLASH_UNKNOWN)
	{
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n", size, size << 20 );
		return 0;
	}

	flash_get_offsets (FLASH_BASE_PRELIM, &flash_info[0]);

#ifdef CONFIG_SYS_FLASH_PROTECTION
	flash_get_protect (&flash_info[0]);
#endif

	return size;
#endif /* CONFIG_DBOX2_FLASH_FAKE */
}

/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */

#ifndef CONFIG_DBOX2_FLASH_FAKE
static ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	ulong value;

	flash_put (info, addr, 0x0555, 0x00AA00AA);
	flash_put (info, addr, 0x02AA, 0x00550055);
	flash_put (info, addr, 0x0555, 0x00900090);
	value = flash_get (info, addr, 0);

	if (info->portwidth == FLASH_CFI_16BIT)
		value |= value << 16;

	switch (value)
	{
		case AMD_MANUFACT:
			info->flash_id = FLASH_MAN_AMD;
			break;
		case FUJ_MANUFACT:
			info->flash_id = FLASH_MAN_FUJ;
			break;
		case INTEL_MANUFACT:
			info->flash_id = FLASH_MAN_INTEL;
			break;
		case STM_MANUFACT:
			info->flash_id = FLASH_MAN_STM;
			break;
		default:
			info->flash_id = FLASH_UNKNOWN;
			info->sector_count = 0;
			info->size = 0;
			return 0;
	}

	value = flash_get (info, addr, 1);

	if (info->portwidth == FLASH_CFI_16BIT)
		value |= value << 16;

	switch (value)
	{
		case AMD_ID_DL323B:
			info->flash_id |= FLASH_AMDL323B;
			info->sector_count = 63 + 8;
			info->size = 0x00800000;
			break;
		case STM_ID_28W320CB:
			info->flash_id |= FLASH_STM320CB;
			info->sector_count = 63 + 8;
			info->size = 0x00800000;
			break;
		case INTEL_ID_28F320C3B:
			info->flash_id |= FLASH_INTEL320B;
			info->sector_count = 63 + 8;
			info->size = 0x00800000;
			break;
		case INTEL_ID_28F320C3T:
			info->flash_id |= FLASH_INTEL320T;
			info->sector_count = 63 + 8;
			info->size = 0x00800000;
			break;
		case INTEL_ID_28F640J3A:
			info->flash_id |= FLASH_28F640J3A;
			info->sector_count = 64;
			info->size = 0x00800000;
			break;
		case INTEL_ID_28F640C3B:
			info->flash_id |= FLASH_28F640C3B;
			info->sector_count = 127 + 8;
			info->size = 0x01000000;
			memctl->memc_or0 = memctl->memc_or0 & 0xff00ffff;	// reset mask in OR0 to 16MB
			break;
		default:
			info->flash_id = FLASH_UNKNOWN;
			return 0;
	}

	flash_put (info, addr, 0, 0x00F000F0);
	return info -> size;
}

static void flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;

	if (info->flash_id & FLASH_BTYPE)
	{
		/* set sector offsets for bottom boot block type */
		info->start[0] = base;
		info->start[1] = base + 0x4000;
		info->start[2] = base + 0x8000;
		info->start[3] = base + 0xC000;
		info->start[4] = base + 0x10000;
		info->start[5] = base + 0x14000;
		info->start[6] = base + 0x18000;
		info->start[7] = base + 0x1C000;
		for (i = 8; i < info->sector_count; i++)
			info->start[i] = base + ((i - 7) * 0x20000);
	}
	else
	{
		/* set sector offsets for top boot block type */
		switch (info->flash_id&FLASH_TYPEMASK)
		{
		case FLASH_28F640J3A:	/* flash has no boot blocks */
			for (i = 0; i < info->sector_count; i++)
				info->start[i] = base + (i * 0x20000);
			break;
		default:		
			i = info->sector_count - 1;
			info->start[i--] = base + info->size - 0x4000;
			info->start[i--] = base + info->size - 0x8000;
			info->start[i--] = base + info->size - 0xC000;
			info->start[i--] = base + info->size - 0x10000;
			info->start[i--] = base + info->size - 0x14000;
			info->start[i--] = base + info->size - 0x18000;
			info->start[i--] = base + info->size - 0x1C000;
			for (; i >= 0; i--)
				info->start[i] = base + (i * 0x20000);
			break;
		}
	}
}

#ifdef CONFIG_SYS_FLASH_PROTECTION
static void flash_get_protect (flash_info_t *info)
{
	volatile unsigned long *addr;
	int i;

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL)
	{
		for (i = 0; i < info->sector_count; i++)
		{
			/* read sector protection at sector address, (A7 .. A0) = 0x02 */
			/* D0 = 1 if protected */
			addr = (volatile unsigned long *) (info->start[i]);
			/* read configuration */
			flash_put (info, addr, 0, 0x00900090);
			switch (flash_get (info, addr, 2) & 3)
		{
			case 0:
				info->protect[i] = 0;
				break;
			case 1:
				info->protect[i] = 1;
				break;
			case 3:
				info->protect[i] = 2;
				break;
		}
			/* read array */
			flash_put (info, addr, 0, 0x00FF00FF);
		}
	}
}
#endif /* CONFIG_SYS_FLASH_PROTECTION */
#endif /* CONFIG_DBOX2_FLASH_FAKE */

/*-----------------------------------------------------------------------
 */

void flash_print_info  (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN)
	{
		printf ("missing or unknown FLASH type\n");
		return;
	}

#ifdef CONFIG_DBOX2_FLASH_FAKE
	switch (info->flash_id)
	{
		case 0x00600000:
			printf ("faked DBOX2 flash\n");
			break;
		default:
			printf ("Unknown Vendor Unknown Chip Type\n");
	}
#else /* CONFIG_DBOX2_FLASH_FAKE */
	switch (info->flash_id  & FLASH_VENDMASK)
	{
		case FLASH_MAN_AMD:
			printf ("AMD ");
			break;
		case FLASH_MAN_FUJ:
			printf ("FUJITSU ");
			break;
		case FLASH_MAN_INTEL:
			printf ("INTEL ");
			break;
		case FLASH_MAN_STM:
			printf ("STM ");
			break;
		default:
			printf ("Unknown Vendor ");
	}

	switch (info->flash_id & FLASH_TYPEMASK)
	{
		case FLASH_AMDL323B:
			printf ("29DL323B (32M, bottom boot sect), ");
			break;
		case FLASH_INTEL320B:
			printf ("28F320C3B (32M, bottom boot sect), ");
			break;
		case FLASH_INTEL320T:
			printf ("28F320C3T (32M, top boot sect), ");
			break;
		case FLASH_28F640J3A:
			printf ("28F640J3A (64M), ");
			break;
		case FLASH_STM320CB:
			printf ("28M320CB (32M, bottom boot sect), ");
			break;
		case FLASH_28F640C3B:
			printf ("28F640C3B (64M, bottom boot sect), ");
			break;
		default:
			printf ("Unknown Chip Type, ");
	}

	switch (info->portwidth)
	{
                case FLASH_CFI_16BIT:
			printf ("16 bit\n");
			break;
                case FLASH_CFI_32BIT:
			printf ("32 bit\n");
			break;
                default:
			printf ("\n");
        }
#endif /* CONFIG_DBOX2_FLASH_FAKE */

	printf ("\n  Size: %ld kB in %d Sectors\n",
		info->size >> 10, info->sector_count);

#ifdef CONFIG_SYS_FLASH_PROTECTION
	flash_get_protect (&flash_info[0]);
#endif

	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i)
	{
		if ((i % 4) == 0)
			printf ("\n   ");
		switch (info->protect[i])
		{
			case 1:
				printf (" %08lX (RO)", info->start[i]);
				break;
			case 2:
				printf (" %08lX (LD)", info->start[i]);
				break;
			case 0:
			default:
				printf (" %08lX     ", info->start[i]);
				break;
		}
	}

	printf ("\n");
}

/*-----------------------------------------------------------------------
 */

int flash_real_protect (flash_info_t *info, long sector, int prot)
{
#ifndef CONFIG_DBOX2_FLASH_FAKE
	volatile unsigned long *addr = (volatile unsigned long *) (info->start[sector]);

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL)
	{
		uchar flashmask=3;
		int secchk, secchk_first=sector, secchk_last=sector;	// defaults to just actual sector

		if ((info->flash_id & FLASH_TYPEMASK)==FLASH_28F640J3A) flashmask = 1;

		/* clear status register */
		flash_put (info, addr, 0, 0x00500050);
		/* read configuration */
//		flash_put (info, addr, 0, 0x00900090);	// request and no read??? why?

		/* lock/unlock block */
		if (((info->flash_id & FLASH_TYPEMASK) == FLASH_28F640J3A)&&(prot == 0)){
			/* this flash clears all lock-bits simultaneously, so
				check state first to speed up an iterative clear lock-block operation	*/
			flash_put (info, addr, 0, 0x00900090);
			if ((flash_get (info, addr, 2) & flashmask) == 1){
				flash_put (info, addr, 0, 0x00600060);
				flash_put (info, addr, 0, 0x00D000D0);
			}
		} else {
			/* setup configuration */
			flash_put (info, addr, 0, 0x00600060);
			if (prot == 0) 
				flash_put (info, addr, 0, 0x00D000D0);
			else if (prot == 2 && !((info->flash_id & FLASH_TYPEMASK) == FLASH_28F640J3A))
				flash_put (info, addr, 0, 0x002F002F);
			else
				flash_put (info, addr, 0, 0x00010001);
	
		}
		/* check status */
		flash_put (info, addr, 0, 0x00700070);		// actually done automatically for 28F640J3A
		while (!(flash_get (info, addr, 0)&(1<<7)));	// check SR7 (WSM ready?)
				/* read configuration */
		flash_put (info, addr, 0, 0x00900090);
			
		if (((info->flash_id & FLASH_TYPEMASK) == FLASH_28F640J3A)&&(prot == 0)){
			/* since we clear all lock block-bits simultaneously we have to check them all */
			secchk_first = 0;
			secchk_last = info->sector_count-1;
		}
		for (secchk = secchk_first; secchk <= secchk_last; ++secchk){
			switch (flash_get (info, (vu_long*)info->start[secchk], 2) & flashmask)
			{
				case 0:
					info->protect[secchk] = 0;
					break;
				case 1:
					info->protect[secchk] = 1;
					break;
				case 3:
					info->protect[secchk] = 2;
					break;
			}
		}

		/* read status register */
		flash_put (info, addr, 0, 0x00700070);	// why?
		/* cancel read status register */
		flash_put (info, addr, 0, 0x00FF00FF);	// actually it also resets to read array
	}
	else
	{
		info->protect[sector] = prot;
	}
#endif /* CONFIG_DBOX2_FLASH_FAKE */

	return 0;
}

/*-----------------------------------------------------------------------
 */

int flash_erase (flash_info_t *info, int s_first, int s_last)
{
#ifndef CONFIG_DBOX2_FLASH_FAKE
	volatile unsigned long *addr = (volatile unsigned long *) (info->start[0]);
	int flag, prot, sect, l_sect;
	ulong start, now, last;
#endif /* CONFIG_DBOX2_FLASH_FAKE */

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("- can't erase unknown flash type - aborted\n");
		return 1;
	}

#ifdef CONFIG_DBOX2_FLASH_FAKE
	printf ("- fake flash mode - can't erase - aborted\n");
	return 1;
#else /* CONFIG_DBOX2_FLASH_FAKE */
	if ((s_first < 0) || (s_first > s_last))
		printf ("- no sectors to erase\n");

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect)
		if (info->protect[sect])
			prot++;

	if (prot)
		printf ("- Warning: %d protected sectors will not be erased!\n", prot);
	else
		printf ("\n");

	l_sect = -1;

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL)
	{
		/* clear status register */
		flash_put (info, addr, 0, 0x00500050);

		for (sect = s_first; sect <= s_last; sect++)
		{
			if (!info->protect[sect])
			{
				addr = (volatile unsigned long *) (info->start[sect]);
				printf("Sector %d, address 0x%08lx     \r", sect, addr);
				/* erase setup */
				flash_put (info, addr, 0, 0x00200020);
				/* erase confirm */
				flash_put (info, addr, 0, 0x00D000D0);
				l_sect = sect;
				/* read status register */
				flash_put (info, addr, 0, 0x00700070);

				last = start = get_timer (0);

				while ((flash_get (info, addr, 0) & 0x00800080) != flash_mask (info, 0x00800080))
				{
					if ((now = get_timer (start)) > CONFIG_SYS_FLASH_ERASE_TOUT)
					{
						flash_put (info, addr, 0, 0x00FF00FF);
						printf ("Timeout\n");
						return 1;
					}
					/*
					if ((now - last) > 1000)
					{
						putc ('.');
						last = now;
					}
					*/
				}
			}
		}

		/* reset to read mode */
		addr = (volatile unsigned long *) (info->start[0]);

		/* read array */
		flash_put (info, addr, 0, 0x00FF00FF);
	}
	else
	{
		flag = disable_interrupts ();

		flash_put (info, addr, 0x0555, 0x00AA00AA);
		flash_put (info, addr, 0x02AA, 0x00550055);
		flash_put (info, addr, 0x0555, 0x00800080);
		flash_put (info, addr, 0x0555, 0x00AA00AA);
		flash_put (info, addr, 0x02AA, 0x00550055);

		for (sect = s_first; sect <= s_last; sect++)
			if (!info->protect[sect])
			{
				addr = (volatile unsigned long *) (info->start[sect]);
				flash_put (info, addr, 0, 0x00300030);
				l_sect = sect;
			}

		if (flag)
			enable_interrupts ();

		/* wait at least 80us - let's wait 1 ms */
		udelay (1000);

		if (l_sect < 0)
			goto DONE_AMD;

		last = start = get_timer (0);
		addr = (volatile unsigned long *) (info->start[l_sect]);

		while ((addr[0] & 0xFFFFFFFF) != 0xFFFFFFFF)
		{
			if ((now = get_timer (start)) > CONFIG_SYS_FLASH_ERASE_TOUT)
			{
				printf ("Timeout\n");
				return 1;
			}
			if ((now - last) > 1000)
			{
				putc ('.');
				last = now;
			}
		}

DONE_AMD:
		/* reset to read mode */
		addr = (volatile unsigned long *) (info->start[0]);
		/* reset bank */
		flash_put (info, addr, 0, 0x00F000F0);
	}

	printf ("done                                         \n");
#endif /* CONFIG_DBOX2_FLASH_FAKE */
	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
#ifdef CONFIG_DBOX2_FLASH_FAKE
	return 0;
#else /* CONFIG_DBOX2_FLASH_FAKE */
	ulong cp, wp, data;
	int i, l, rc;

	/* get lower word aligned address */
	wp = (addr & ~(info->portwidth - 1));

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0)
	{
		data = 0;
		for (i = 0, cp = wp; i < l; ++i, ++cp)
			data = (data << 8) | (*(uchar *) cp);
		for (; i < info->portwidth && cnt > 0; ++i)
		{
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt == 0 && i < info->portwidth; ++i, ++cp)
			data = (data << 8) | (*(uchar *) cp);

		if ((rc = write_word (info, wp, data)) != 0)
			return rc;
		wp += info->portwidth;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= info->portwidth)
	{
		data = 0;
		for (i = 0; i < info->portwidth; ++i)
			data = (data << 8) | *src++;
		if ((rc = write_word (info, wp, data)) != 0)
			return rc;
		wp  += info->portwidth;
		cnt -= info->portwidth;
	}

	if (cnt == 0)
		return 0;

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i = 0, cp = wp; i < info->portwidth && cnt > 0; ++i, ++cp)
	{
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i < info->portwidth; ++i, ++cp)
		data = (data << 8) | (*(uchar *) cp);

	return (write_word (info, wp, data));
#endif /* CONFIG_DBOX2_FLASH_FAKE */
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
#ifndef CONFIG_DBOX2_FLASH_FAKE
static int write_word (flash_info_t *info, ulong dest, ulong data)
{
	volatile unsigned long *addr = (volatile unsigned long *) dest;
	volatile unsigned long *faddr = (volatile unsigned long *) (info->start[0]);
	int start, flag;

	/* check if flash is (sufficiently) erased */
	if ((flash_get (info, addr, 0) & data) != flash_mask (info, data))
		return 2;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL)
	{
		flash_put (info, addr, 0, 0x00400040);
		flash_put (info, addr, 0, data);
	}
	else
	{
		flash_put (info, faddr, 0x0555, 0x00AA00AA);
		flash_put (info, faddr, 0x02AA, 0x00550055);
		flash_put (info, faddr, 0x0555, 0x00A000A0);
		flash_put (info, addr, 0, data);
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL)
	{
		flash_put (info, addr, 0, 0x00700070);
		start = get_timer (0);
		while ((flash_get (info, faddr, 0) & 0x00800080) != flash_mask (info, 0x00800080))
			if (get_timer (start) > CONFIG_SYS_FLASH_WRITE_TOUT)
			{
				flash_put (info, faddr, 0, 0x00FF00FF);
				return 1;
			}

		flash_put (info, faddr, 0, 0x00FF00FF);
	}
	else
	{
		/* data polling for D7 */
		start = get_timer (0);
		while ((flash_get (info, addr, 0) & 0x00800080) != (data & flash_mask (info, 0x00800080)))
			if (get_timer (start) > CONFIG_SYS_FLASH_WRITE_TOUT)
				return 1;
	}

	return 0;
}
#endif /* CONFIG_DBOX2_FLASH_FAKE */

/*-----------------------------------------------------------------------
 */
