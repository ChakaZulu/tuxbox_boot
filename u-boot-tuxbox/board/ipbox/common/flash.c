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

/*
 * Modified 4/5/2001
 * Wait for completion of each sector erase command issued
 * 4/5/2001
 * Chris Hallinan - DS4.COM, Inc. - clh@net1plus.com
 */

#include <common.h>
#include <ppc4xx.h>
#include <asm/processor.h>

flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_word (flash_info_t *info, ulong dest, ulong data);

unsigned int flash_progress_now;
unsigned int flash_progress_total;

#define ADDR0           0x5555
#define ADDR1           0x2aaa
#define FLASH_WORD_SIZE unsigned short

#ifdef FLASG_DEBUG
#define fdebug(fmt, arg...) printf( fmt, ##arg )
#else
#define fdebug(fmt, arg...) do{}while(0)
#endif

// this is not anymore valid...
#if 0
/*-----------------------------------------------------------------------
 * Flash partition for JFFS2
 */
#ifdef CFG_JFFS_CUSTOM_PART
#include <jffs2/load_kernel.h>
#include "common/flash_img_info.h"

extern flash_img_info_t dgs_flash_imgs[];

static struct part_info part;

struct part_info* jffs2_part_info(int part_num)
{
	int i;

	if(part.usr_priv==(void*)1)
		return &part;

	memset(&part, 0, sizeof(part));

	for( i=0; i<part_num; i++ )
	{
		if( !dgs_flash_imgs[i].name )
		{
			printf( "out of partition table.(%d)\n", part_num );
			return NULL;
		}
	}
	part.offset = dgs_flash_imgs[i].start;
	part.size = dgs_flash_imgs[i].size;

	printf( "partition \"%s\" selected.(%08x,%06x)\n",
			dgs_flash_imgs[i].name,
			dgs_flash_imgs[i].start,
			dgs_flash_imgs[i].size );

	/* unused in current jffs2 loader */
	part.erasesize = 0;

	/* Mark the struct as ready */
	part.usr_priv=(void*)1;

	return &part;
}

#endif
#endif

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	int i;
	unsigned long size=0;

	/* Init: no FLASHes known */
	for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	flash_get_size((vu_long *)FLASH_BASE0_PRELIM, &flash_info[0]);
	if (flash_info[0].flash_id == FLASH_UNKNOWN)
		puts ("## Unknown FLASH on Bank 0\n");

#if CFG_MAX_FLASH_BANKS > 1
	flash_get_size((vu_long *)FLASH_BASE1_PRELIM, &flash_info[1]);
	if (flash_info[1].flash_id == FLASH_UNKNOWN)
		puts ("## Unknown FLASH on Bank 1\n");
#elif CFG_MAX_FLASH_BANKS > 2
#error "unsupported flash banks..."
#endif

	for( i=0,size=0; i<CFG_MAX_FLASH_BANKS; i++ )
		size += flash_info[i].size;

	return size;
}



/*-----------------------------------------------------------------------
 */
void flash_print_info  (flash_info_t *info)
{
	int i;
	int k;
	int size;
	int erased;
	volatile unsigned long *flash;

	if (info->flash_id == FLASH_UNKNOWN) {
		puts ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
#ifdef CFG_FLASH_SUP_AMD
		case FLASH_MAN_AMD:	puts ("AMD ");		break;
#endif
#ifdef CFG_FLASH_SUP_SPA
		case FLASH_MAN_SPA:	puts ("Spansion ");		break;
#endif
#ifdef CFG_FLASH_SUP_FUJ
		case FLASH_MAN_FUJ:	puts ("FUJITSU ");		break;
#endif
#ifdef CFG_FLASH_SUP_SST
		case FLASH_MAN_SST:	puts ("SST ");		break;
#endif
#ifdef CFG_FLASH_SUP_STM
		case FLASH_MAN_STM:	puts ("STM ");		break;
#endif
#ifdef CFG_FLASH_SUP_MX
		case FLASH_MAN_MX:	puts ("MX ");			break;
#endif
#ifdef CFG_FLASH_SUP_EON
		case FLASH_MAN_EON:	puts ("EON ");		break;
#endif
		default:		puts ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
#ifdef CFG_FLASH_SUP_AMD_F040B
		case FLASH_AM040:	puts ("AM29F040 (512 Kbit, uniform sector size)\n");
					break;
#endif
#ifdef CFG_FLASH_SUP_AMD_LV400T
		case FLASH_AM400T:	puts ("AM29LV400T (4 Mbit, top boot sector)\n");
					break;
#endif
#ifdef CFG_FLASH_SUP_AMD_LV400B
		case FLASH_AM400B:	puts ("AM29LV400B (4 Mbit, bottom boot sect)\n");
					break;
#endif
#ifdef CFG_FLASH_SUP_AMD_LV800B
		case FLASH_AM800B:	puts ("AM29LV800B (8 Mbit, bottom boot sect)\n");
					break;
#endif
#ifdef CFG_FLASH_SUP_AMD_LV800T
		case FLASH_AM800T:	puts ("AM29LV800T (8 Mbit, top boot sector)\n");
					break;
#endif
#ifdef CFG_FLASH_SUP_AMD_LV160B
		case FLASH_AM160B:	puts ("AM29LV160B (16 Mbit, bottom boot sect)\n");
					break;
#endif
#ifdef CFG_FLASH_SUP_AMD_LV160T
		case FLASH_AM160T:	puts ("AM29LV160T (16 Mbit, top boot sector)\n");
					break;
#endif
#ifdef CFG_FLASH_SUP_AMD_LV320B
		case FLASH_AM320B:	puts ("AM29LV320B (32 Mbit, bottom boot sect)\n");
					break;
#endif
#ifdef CFG_FLASH_SUP_AMD_LV320T
		case FLASH_AM320T:	puts ("AM29LV320T (32 Mbit, top boot sector)\n");
					break;
#endif
#ifdef CFG_FLASH_SUP_SST_xF800A
		case FLASH_SST800A:	puts ("SST39LF/VF800 (8 Mbit, uniform sector size)\n");
					break;
#endif
#ifdef CFG_FLASH_SUP_SST_xF160A
		case FLASH_SST160A:	puts ("SST39LF/VF160 (16 Mbit, uniform sector size)\n");
					break;
#endif
#ifdef CFG_FLASH_SUP_STM_29W320DB
		case FLASH_STMW320DB:	puts ("STM29W320DB (32 Mbit, bottom sector size)\n");
					break;
#endif
#ifdef CFG_FLASH_SUP_STM_29W640DT
		case FLASH_STMW640DT:	puts("STM29W640DT (64M bit, top sector size)\n");
					break;
#endif
#ifdef CFG_FLASH_SUP_MX_29LV320AB
		case FLASH_MXLV320B:	puts ("MX29LV320AB (32 Mbit, bottom sector size)\n");
				break;
#endif
#ifdef CFG_FLASH_SUP_MX_29LV640MT
		case FLASH_MXLV640MT:	puts ("MX29LV640MT (64 Mbit, top sector size)\n");
					break;
#endif
#ifdef CFG_FLASH_SUP_MX_29LV640BT
		case FLASH_MXLV640BT:	puts ("MX29LV640BT/DT (64 Mbit, top sector size)\n");
					break;
#endif
#ifdef CFG_FLASH_SUP_SPA_S29GL064MR3
		case FLASH_SPA064MR3:	puts ("SPA20GL064MR3 (64 Mbit, top sector size)\n");
				break;
#endif
#ifdef CFG_FLASH_SUP_SPA_S29GL064MR4
		case FLASH_SPA064MR4:	puts ("SPA20GL064MR4 (64 Mbit, bottom sector size)\n");
				break;
#endif
#ifdef CFG_FLASH_SUP_EON_29LV640B
		case FLASH_EN29LV640B:	puts ("EN29LV640B (64 Mbit, bottom sector size)\n");
				break;
#endif
		default:		puts ("Unknown Chip Type\n");
					break;
	}

	printf ("  Size: %ld KB in %d Sectors\n",
			info->size >> 10, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
		/*
		 * Check if whole sector is erased
		 */
		if (i != (info->sector_count-1))
			size = info->start[i+1] - info->start[i];
		else
			size = info->start[0] + info->size - info->start[i];
		erased = 1;
		flash = (volatile unsigned long *)info->start[i];
		size = size >> 2;        /* divide by 4 for longword access */
		for (k=0; k<size; k++)
		{
			if (*flash++ != 0xffffffff)
			{
				erased = 0;
				break;
			}
		}

		if ((i % 5) == 0)
			puts ("\n   ");
			printf (" %08lX%s%s",
			info->start[i],
			erased ? " E" : "  ",
			info->protect[i] ? "RO " : "   "
		);
	}
	puts ("\n");
	return;
}

/*-----------------------------------------------------------------------
 */


/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
	short i;
	FLASH_WORD_SIZE manufacture_id;
	FLASH_WORD_SIZE device_id1, device_id2, device_id3;
	ulong base = (ulong)addr;
	volatile FLASH_WORD_SIZE *addr2 = (FLASH_WORD_SIZE *)addr;
	int fail = 0;

	/* Write auto select command: read Manufacturer ID */
	addr2[0]	= (FLASH_WORD_SIZE)0x00f000f0;	/* some device need reset for reading */
	addr2[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
	addr2[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
	addr2[ADDR0] = (FLASH_WORD_SIZE)0x00900090;
	manufacture_id = addr2[0];

	/* Write auto select command: read device ID */
	addr2[0]	= (FLASH_WORD_SIZE)0x00f000f0;	/* some device need reset for reading */
	addr2[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
	addr2[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
	addr2[ADDR0] = (FLASH_WORD_SIZE)0x00900090;
	device_id1 = addr2[0x02>>1];
	device_id2 = addr2[0x1c>>1];
	device_id3 = addr2[0x1e>>1];

	switch (manufacture_id)
	{
#if defined(CFG_FLASH_SUP_MX)
		case (FLASH_WORD_SIZE)MX_MANUFACT :
			info->flash_id = FLASH_MAN_MX;

			if(0)
			{
				// dummy
			}
#if defined(CFG_FLASH_SUP_MX_29LV640MT)
			else if(device_id1 == (FLASH_WORD_SIZE)MX_ID_LV640MT &&	/* 29LV640MT */
					device_id2 == 0x2210 &&		/* must be 0x10 */
					device_id3 == 0x2201)		/* TOP : 0x01 */
			{
				info->flash_id += FLASH_MXLV640MT;
				info->sector_count = 135;
				info->size = 0x00800000;

				/* set sector offset for top boot block type */
				for(i=0; i<127;i++)
					info->start[i] = base + i*0x10000;
				for(; i<info->sector_count; i++)
					info->start[i] = base + 0x7f0000  + (i-127)*0x2000;
			}
#endif
#if defined(CFG_FLASH_SUP_MX_29LV640DT)
			else if(device_id1 == (FLASH_WORD_SIZE)MX_ID_LV640DT)
			{
				info->flash_id += FLASH_MXLV640DT;
				info->sector_count = 135;
				info->size = 0x00800000;

				/* set sector offset for top boot block type */
				for(i=0; i<127;i++)
					info->start[i] = base + i*0x10000;
				for(; i<info->sector_count; i++)
					info->start[i] = base + 0x7f0000  + (i-127)*0x2000;
			}
#endif
			else
			{
				fail++;
			}
			break;
#endif
#if defined(CFG_FLASH_SUP_STM)
		case (FLASH_WORD_SIZE)STM_MANUFACT:
			info->flash_id = FLASH_MAN_STM;

			if(0)
			{

			}
#if defined(CFG_FLASH_SUP_STM_29W640DT)
			else if(device_id1 == (FLASH_WORD_SIZE)STM_ID_29W640DT)
			{
				info->flash_id += FLASH_STMW640DT;
				info->sector_count = 135;
				info->size = 0x00800000;

				/* set sector offset for top boot block type */
				for(i=0; i<127;i++)
					info->start[i] = base + i*0x10000;
				for(; i<info->sector_count; i++)
					info->start[i] = base + 0x7f0000  + (i-127)*0x2000;
			}
#endif
			else
			{
				fail++;
			}

			break;
#endif
#if defined(CFG_FLASH_SUP_SPA)
		case (FLASH_WORD_SIZE)SPA_MANUFACT :
			info->flash_id = FLASH_MAN_SPA;

			if(0)
			{

			}
#if defined(CFG_FLASH_SUP_SPA_S29GL064MR4)
			else if(device_id1 == (FLASH_WORD_SIZE)SPA_ID_S29GL064M &&
					device_id2 == 0x2210 &&
					device_id3 == 0x2200 ) /* BOTTOM : 0x00 */
			{
				info->flash_id += FLASH_SPA064MR4;
				info->sector_count = 135;
				info->size = 0x00800000;

				for(i=0; i<8; i++)
					info->start[i] = base + i*0x2000;
				for(; i<info->sector_count; i++)
					info->start[i] = base + 8*0x2000 + (i-8)*0x10000;
			}
#endif
			else
			{
				fail++;
			}
			break;
#endif
#if defined(CFG_FLASH_SUP_EON)
		case (FLASH_WORD_SIZE)EON_MANUFACT: 
			info->flash_id = FLASH_MAN_EON;
			
			if(0)
			{

			}
#if defined(CFG_FLASH_SUP_EON_29LV640B)
			else if(device_id1 == (FLASH_WORD_SIZE)EON_ID_EN29LV640B )
			{
				/* bottom */
				info->flash_id += FLASH_EN29LV640B;
				info->sector_count = 135;
				info->size = 0x00800000;

				for(i=0; i<8; i++)
					info->start[i] = base + i*0x2000;
				for(; i<info->sector_count; i++)
					info->start[i] = base + 8*0x2000 + (i-8)*0x10000;
			}

#endif
			else
			{
				fail++;
			}
			break;
#endif
		default :
			fail++;
	}


	if(fail)
	{
		printf( "unknown.(manufacture:%04x, device:%04x)\n", manufacture_id, device_id1 );
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */
	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr2 = (volatile FLASH_WORD_SIZE *)(info->start[i]);
		addr2[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
		addr2[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
		addr2[ADDR0] = (FLASH_WORD_SIZE)0x00900090;
		info->protect[i] = addr2[2] & 1;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr2 = (FLASH_WORD_SIZE *)info->start[0];
		*addr2 = (FLASH_WORD_SIZE)0x00F000F0;	/* reset bank */
	}

	return (info->size);
}

#if 0
/*
 * The following code cannot be run from FLASH!
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
	short i;
	FLASH_WORD_SIZE value;
	FLASH_WORD_SIZE man_id;
	ulong base = (ulong)addr;
	volatile FLASH_WORD_SIZE *addr2 = (FLASH_WORD_SIZE *)addr;

	/* Write auto select command: read Manufacturer ID */
	addr2[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
	addr2[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
	addr2[ADDR0] = (FLASH_WORD_SIZE)0x00900090;
	value = addr2[0];
	man_id = value;

	fdebug( "flash manufacturer : " );
	switch (value) {
#ifdef CFG_FLASH_SUP_AMD
	case (FLASH_WORD_SIZE)AMD_MANUFACT:
		fdebug( "amd\n" );
		info->flash_id = FLASH_MAN_AMD;
		break;
#endif
#ifdef CFG_FLASH_SUP_SPA
	case (FLASH_WORD_SIZE)SPA_MANUFACT:
		fdebug( "spa\n" );
		info->flash_id = FLASH_MAN_SPA;
		break;
#endif
#ifdef CFG_FLASH_SUP_FUJ
	case (FLASH_WORD_SIZE)FUJ_MANUFACT:
		fdebug( "fuj\n" );
		info->flash_id = FLASH_MAN_FUJ;
		break;
#endif
#ifdef CFG_FLASH_SUP_SST
	case (FLASH_WORD_SIZE)SST_MANUFACT:
		fdebug( "sst\n" );
		info->flash_id = FLASH_MAN_SST;
		break;
#endif
#ifdef CFG_FLASH_SUP_STM
	case (FLASH_WORD_SIZE)STM_MANUFACT:
		fdebug( "stm\n" );
		info->flash_id = FLASH_MAN_STM;
		break;
#endif
#ifdef CFG_FLASH_SUP_MX
	case (FLASH_WORD_SIZE)MX_MANUFACT:
		fdebug( "mx\n" );
		info->flash_id = FLASH_MAN_MX;
		break;
#endif
	default:
		printf( "unknown.(manufactor:%02x)\n", value );
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* no or unknown flash	*/
	}

	/* Write auto select command: read device ID */
	addr2[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
	addr2[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
	addr2[ADDR0] = (FLASH_WORD_SIZE)0x00900090;
	value = addr2[1];

	fdebug( "flash device :" );
	switch (value) {
#ifdef CFG_FLASH_SUP_AMD_F040B
	case (FLASH_WORD_SIZE)AMD_ID_F040B:
		fdebug( "AMD_ID_F040B\n" );
	        info->flash_id += FLASH_AM040;
		info->sector_count = 8;
		info->size = 0x0080000; /* => 512 ko */
		break;
#endif
#ifdef CFG_FLASH_SUP_AMD_LV400T
	case (FLASH_WORD_SIZE)AMD_ID_LV400T:
		fdebug( "AMD_ID_LV400T\n" );
		info->flash_id += FLASH_AM400T;
		info->sector_count = 11;
		info->size = 0x00080000;
		break;				/* => 0.5 MB		*/
#endif
#ifdef CFG_FLASH_SUP_AMD_LV400B
	case (FLASH_WORD_SIZE)AMD_ID_LV400B:
		fdebug( "AMD_ID_LV400B\n" );
		info->flash_id += FLASH_AM400B;
		info->sector_count = 11;
		info->size = 0x00080000;
		break;				/* => 0.5 MB		*/
#endif
#ifdef CFG_FLASH_SUP_AMD_LV800T
	case (FLASH_WORD_SIZE)AMD_ID_LV800T:
		fdebug( "AMD_ID_LV800T\n" );
		info->flash_id += FLASH_AM800T;
		info->sector_count = 19;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/
#endif
#ifdef CFG_FLASH_SUP_AMD_LV800B
	case (FLASH_WORD_SIZE)AMD_ID_LV800B:
		fdebug( "AMD_ID_LV800B\n" );
		info->flash_id += FLASH_AM800B;
		info->sector_count = 19;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/
#endif
#ifdef CFG_FLASH_SUP_AMD_LV160T
	case (FLASH_WORD_SIZE)AMD_ID_LV160T:
		fdebug( "AMD_ID_LV160T\n" );
		info->flash_id += FLASH_AM160T;
		info->sector_count = 35;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/
#endif
#ifdef CFG_FLASH_SUP_AMD_LV160B
	case (FLASH_WORD_SIZE)AMD_ID_LV160B:
		fdebug( "AMD_ID_LV160B\n" );
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/
#endif
#ifdef CFG_FLASH_SUP_AMD_LV320T
	case (FLASH_WORD_SIZE)AMD_ID_LV320T:
		fdebug( "AMD_ID_LV320T\n" );
		info->flash_id += FLASH_AM320T;
		info->sector_count = 71;
		info->size = 0x00400000;
		break;				/* => 4 MB		*/
#endif
#ifdef CFG_FLASH_SUP_AMD_LV320B
	case (FLASH_WORD_SIZE)AMD_ID_LV320B:
		fdebug( "AMD_ID_LV320B\n" );
		info->flash_id += FLASH_AM320B;
		info->sector_count = 71;
		info->size = 0x00400000;

		/* set sector offsets for bottom boot block type	*/
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00002000;
		info->start[2] = base + 0x00004000;
		info->start[3] = base + 0x00006000;
		info->start[4] = base + 0x00008000;
		info->start[5] = base + 0x0000a000;
		info->start[6] = base + 0x0000c000;
		info->start[7] = base + 0x0000e000;
		for (i = 8; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00010000) - 0x00070000;
		}
		break;				/* => 4 MB		*/
#endif
#ifdef CFG_FLASH_SUP_SST_xF800A
	case (FLASH_WORD_SIZE)SST_ID_xF800A:
		fdebug( "SST_ID_xF800A\n" );
		info->flash_id += FLASH_SST800A;
		info->sector_count = 16;
		info->size = 0x00100000;
		break;				/* => 1 MB		*/
#endif
#ifdef CFG_FLASH_SUP_SST_xF160A
	case (FLASH_WORD_SIZE)SST_ID_xF160A:
		fdebug( "SST_ID_xF160A\n" );
		info->flash_id += FLASH_SST160A;
		info->sector_count = 32;
		info->size = 0x00200000;
		break;				/* => 2 MB		*/
#endif
#ifdef CFG_FLASH_SUP_STM_29W320DB
	case (FLASH_WORD_SIZE)STM_ID_29W320DB:
		fdebug( "STM_ID_29W320DB\n" );
		info->flash_id += FLASH_STMW320DB;
		info->sector_count = 67;
		info->size = 0x00400000;

		/* set sector offsets for bottom boot block type	*/
		info->start[0] = base + 0x00000000;
		info->start[1] = base + 0x00004000;
		info->start[2] = base + 0x00006000;
		info->start[3] = base + 0x00008000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = base + (i * 0x00010000) - 0x00030000;
		}
		break;				/* ST 4MB		*/
#endif
#ifdef CFG_FLASH_SUP_STM_29W640DT
	case (FLASH_WORD_SIZE)STM_ID_29W640DT:
		fdebug("STM_ID_29W640DT\n");
		info->flash_id += FLASH_STMW640DT;
		info->sector_count = 135;
		info->size = 0x00800000;

		/* set sector offset for top boot block type */
		for(i=0; i<127;i++)
		{
			info->start[i] = base + i*0x10000;
		}
		for(; i<info->sector_count; i++)
		{
			info->start[i] = base + 0x7f0000  + (i-127)*0x2000;
		}

		break;				/* ST 8MB	*/
#endif
#ifdef CFG_FLASH_SUP_MX_29LV320AB
	case (FLASH_WORD_SIZE)MX_ID_LV320B:
		fdebug( "MX_ID_29LV320AB\n" );
		info->flash_id += FLASH_MXLV320B;
		info->sector_count = 71;
		info->size = 0x00400000;

		/* set sector offsets for bottom boot block type	*/
		for( i=0; i<8; i++ )
			info->start[i] = base + (i * 0x00002000);
		for (; i<info->sector_count; i++) {
			info->start[i] = base + (i * 0x00010000) - 0x00070000;
		}
		break;				/* MX 4MB		*/
#endif
#if defined(CFG_FLASH_SUP_MX_29LV640MT) || defined(CFG_FLASH_SUP_SPA_S29GL064MR4)
	case (FLASH_WORD_SIZE)MX_ID_LV640MT:
		if(man_id == (FLASH_WORD_SIZE)MX_MANUFACT)
		{
			fdebug("MX_ID_29LV640MT\n");
			info->flash_id += FLASH_MXLV640MT;
			info->sector_count = 135;
			info->size = 0x00800000;

			/* set sector offset for top boot block type */
			for(i=0; i<=126;i++)
			{
				info->start[i] = base + i*0x10000;
			}
			for(; i<info->sector_count; i++)
			{
				info->start[i] = base + 0x7f0000  + (i-127)*0x2000;
			}
		}
		else if(man_id == (FLASH_WORD_SIZE)SPA_MANUFACT)
		{
			cycle2 = addr[0x1c>>1];
			cycle3 = addr[0x1e>>1];
			fdebug("SPA_ID_S29GL064MR4\n");
			info->flash_id += FLASH_SPA064MR4;
			info->sector_count = 135;
			info->size = 0x00800000;
			for(i=0; i<8; i++)
				info->start[i] = b
		}

		break;
#endif
#ifdef CFG_FLASH_SUP_MX_29LV640BT
	case (FLASH_WORD_SIZE)MX_ID_LV640BT:
		fdebug("MX_ID_29LV640BT/DT\n");
		info->flash_id += FLASH_MXLV640BT;
		info->sector_count=135;
		info->size = 0x00800000;

		/* set sector offset for top boot block type */

		for(i=0; i<=126;i++)
		{
			info->start[i] = base + i*0x10000;
		}
		for(; i<info->sector_count; i++)
		{
			info->start[i] = base + 0x7f0000  + (i-127)*0x2000;
		}
		break;
#endif
#ifdef CFG_FLASH_SUP_SPA_S29GL064MR3
	case (FLASH_WORD_SIZE)SPA_ID_S29GL064M:
		/* this device has same sector size with MX29LV640M */

		fdebug( "SPA_ID_S29GL064M\n" );
		/* get autoselect cycle2 */
		value = addr2[0x0e];
		switch( value )
		{
		case (FLASH_WORD_SIZE)0x2210:
			/* get autoselect cycle3 */
			value = addr2[0x0f];
			switch( value )
			{
			case (FLASH_WORD_SIZE)0x2201:
				info->flash_id += FLASH_SPA064MR3;
				info->sector_count = 135;
				info->size = 0x00800000;

				for( i=0; i<127; i++ )
					info->start[i] = base + (i * 0x00010000);
				for( ; i<info->sector_count; i++ )
					info->start[i] = base + ((i-127) * 0x00002000) + (127 * 0x00010000);
				break;				/* SPA 8MB		*/
			default:
				printf( "unknown SPA_29GL064M cycle3 %x\n", value );
				info->flash_id = FLASH_UNKNOWN;
				return 0;
			}
			break;
		default:
			printf( "unknown SPA_29GL064M cycle2 %x\n", value );
			info->flash_id = FLASH_UNKNOWN;
			return 0;
		}
		break;
#endif
	default:
		printf( "unknown.(device:%02x)\n", value );
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */

	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		/* D0 = 1 if protected */
		addr2 = (volatile FLASH_WORD_SIZE *)(info->start[i]);
		addr2[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
		addr2[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
		addr2[ADDR0] = (FLASH_WORD_SIZE)0x00900090;
		info->protect[i] = addr2[2] & 1;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr2 = (FLASH_WORD_SIZE *)info->start[0];
		*addr2 = (FLASH_WORD_SIZE)0x00F000F0;	/* reset bank */
	}

	return (info->size);
}
#endif

int wait_for_DQ7(flash_info_t *info, int sect)
{
	ulong start, now, last;
	volatile FLASH_WORD_SIZE *addr = (FLASH_WORD_SIZE *)(info->start[sect]);

	start = get_timer (0);
	last  = start;
	while ((addr[0] & (FLASH_WORD_SIZE)0x00800080) != (FLASH_WORD_SIZE)0x00800080) {
		if ((now = get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
			puts ("Timeout\n");
			return -1;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {  /* every second */
			putc ('.');
			last = now;
		}
	}
	return 0;
}

/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	volatile FLASH_WORD_SIZE *addr = (FLASH_WORD_SIZE *)(info->start[0]);
	volatile FLASH_WORD_SIZE *addr2;
	int flag, prot, sect, l_sect;
	int i;

	/*
	 * announce progress of erase.
	 */
	flash_progress_now = 0;
	flash_progress_total = s_last-s_first+1;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			puts ("- missing\n");
		} else {
			puts ("- no sectors to erase\n");
		}
		return 1;
	}

	if (info->flash_id == FLASH_UNKNOWN) {
		puts ("Can't erase unknown flash type - aborted\n");
		return 1;
	}

	prot = 0;
	for (sect=s_first; sect<=s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf ("- Warning: %d protected sectors will not be erased!\n",
			prot);
	} else {
		puts ("\n");
	}

	l_sect = -1;

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			/* Disable interrupts which might cause a timeout here */
			flag = disable_interrupts();

			addr2 = (FLASH_WORD_SIZE *)(info->start[sect]);
			//		    printf("Erasing sector %p\n", addr2);	/* CLH */

			if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_SST) {
				addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
				addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
				addr[ADDR0] = (FLASH_WORD_SIZE)0x00800080;
				addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
				addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
				addr2[0] = (FLASH_WORD_SIZE)0x00500050;  /* block erase */
				for (i=0; i<50; i++)
					udelay(1000);  /* wait 1 ms */
			} else {
				addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
				addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
				addr[ADDR0] = (FLASH_WORD_SIZE)0x00800080;
				addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
				addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
				addr2[0] = (FLASH_WORD_SIZE)0x00300030;  /* sector erase */
			}
			l_sect = sect;
			/*
			 * Wait for each sector to complete, it's more
			 * reliable.  According to AMD Spec, you must
			 * issue all erase commands within a specified
			 * timeout.  This has been seen to fail, especially
			 * if printf()s are included (for debug)!!
			 */
			wait_for_DQ7(info, sect);

			/* set progress */
			flash_progress_now = sect - s_first + 1;

			/* re-enable interrupts if necessary */
			if (flag)
				enable_interrupts();
		}
	}

	/* wait at least 80us - let's wait 1 ms */
	udelay (1000);

#if 0
	/*
	 * We wait for the last triggered sector
	 */
	if (l_sect < 0)
		goto DONE;
	wait_for_DQ7(info, l_sect);

DONE:
#endif
	/* reset to read mode */
	addr = (FLASH_WORD_SIZE *)info->start[0];
	addr[0] = (FLASH_WORD_SIZE)0x00F000F0;	/* reset bank */

	puts ("\n");
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
	ulong cp, wp, data;
	int i, l, rc;
	unsigned long total_cnt = cnt;

	wp = (addr & ~3);	/* get lower word aligned address */

	/*
	 * announce progress of writing... flash.
	 */
	flash_progress_now = 0;
	flash_progress_total = cnt;

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i=0, cp=wp; i<l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}
		for (; i<4 && cnt>0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt==0 && i<4; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}

		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp += 4;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= 4) {
		data = 0;
		for (i=0; i<4; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp  += 4;
		cnt -= 4;

		/* set progress */
		flash_progress_now = total_cnt - cnt;
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i=0, cp=wp; i<4 && cnt>0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i<4; ++i, ++cp) {
		data = (data << 8) | (*(uchar *)cp);
	}

	return (write_word(info, wp, data));
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word (flash_info_t * info, ulong dest, ulong data)
{
	volatile FLASH_WORD_SIZE *addr2 = (FLASH_WORD_SIZE *) (info->start[0]);
	volatile FLASH_WORD_SIZE *dest2 = (FLASH_WORD_SIZE *) dest;
	volatile FLASH_WORD_SIZE *data2 = (FLASH_WORD_SIZE *) & data;
	ulong start;
	int i;

	/* Check if Flash is (sufficiently) erased */
	if ((*((volatile FLASH_WORD_SIZE *) dest) &
	    (FLASH_WORD_SIZE) data) != (FLASH_WORD_SIZE) data) {
		printf( "is not erased... 0x%08x:0x%x\n", (unsigned int)dest, *((volatile FLASH_WORD_SIZE*)dest) );
		return 2;
	}

	for (i = 0; i < 4 / sizeof (FLASH_WORD_SIZE); i++)
	{
		int flag;

		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts ();

		addr2[ADDR0] = (FLASH_WORD_SIZE) 0x00AA00AA;
		addr2[ADDR1] = (FLASH_WORD_SIZE) 0x00550055;
		addr2[ADDR0] = (FLASH_WORD_SIZE) 0x00A000A0;

		dest2[i] = data2[i];

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts ();

		/* data polling for D7 */
		start = get_timer (0);
		while ((dest2[i] & (FLASH_WORD_SIZE) 0x00800080) != (data2[i] & (FLASH_WORD_SIZE) 0x00800080))
		{

			if (get_timer (start) > CFG_FLASH_WRITE_TOUT)
			{
				return (1);
			}
		}
	}

	return (0);
}

/*-----------------------------------------------------------------------
 */
