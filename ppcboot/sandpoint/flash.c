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

#include <ppcboot.h>
#include <mpc8240.h>
#include <asm/processor.h>
#include <asm/pci_io.h>
#include "w83c553f.h"

flash_info_t    flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips    */

/*-----------------------------------------------------------------------
 * Functions
 */
ulong flash_get_size (vu_long *addr, flash_info_t *info);

int flash_write (uchar *, ulong, ulong);
flash_info_t *addr2info (ulong);

static int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt);
static int write_word (flash_info_t *info, ulong dest, ulong data);
static void flash_get_offsets (ulong base, flash_info_t *info);
static int  flash_protect (int flag, ulong from, ulong to, flash_info_t *info);

/*-----------------------------------------------------------------------
 * Protection Flags:
 */
#define FLAG_PROTECT_SET    0x01
#define FLAG_PROTECT_CLEAR  0x02

/*flash command address offsets*/

#define ADDR0           (0x555)
#define ADDR1           (0x2AA)
#define ADDR3           (0x001)

#define FLASH_WORD_SIZE unsigned char

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
    unsigned long size;
    int i;
    unsigned long base;
    register unsigned long temp;

    /* Init: no FLASHes known */
    for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
        flash_info[i].flash_id = FLASH_UNKNOWN;
    }

    /* Static FLASH Bank configuration here - FIXME XXX */

    /*Enable writes to Sandpoint flash*/
    printf("setting flash write enable\n");
    CONFIG_READ_BYTE(CFG_WINBOND_ISA_CFG_ADDR + WINBOND_CSCR, temp);
    temp &= 0xDF; /* clear BIOSWP bit */
    CONFIG_WRITE_BYTE(CFG_WINBOND_ISA_CFG_ADDR + WINBOND_CSCR, temp);

    size = flash_get_size((vu_long *)FLASH_BASE0_PRELIM, &flash_info[0]);

    if (flash_info[0].flash_id == FLASH_UNKNOWN) {
        printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
            size, size<<20);
    }


    flash_get_offsets (base, &flash_info[0]);

    /* monitor protection ON by default */
    (void)flash_protect(FLAG_PROTECT_SET,
                base + size-CFG_MONITOR_LEN,
                base + size - 1,
                &flash_info[0]);


    flash_info[1].flash_id = FLASH_UNKNOWN;
    flash_info[1].sector_count = -1;

    flash_info[0].size = size;

    return (size);
}

/*-----------------------------------------------------------------------
 * Check or set protection status for monitor sectors
 *
 * The monitor always occupies the _first_ part of the _first_ Flash bank.
 */
static int  flash_protect (int flag, ulong from, ulong to, flash_info_t *info)
{
    ulong b_end = info->start[0] + info->size - 1;  /* bank end address */
    int rc    =  0;
    int first = -1;
    int last  = -1;
    int i;

    if (to < info->start[0]) {
        return (0);
    }

    for (i=0; i<info->sector_count; ++i) {
        ulong end;      /* last address in current sect */
        short s_end;

        s_end = info->sector_count - 1;

        end = (i == s_end) ? b_end : info->start[i + 1] - 1;

        if (from > end) {
            continue;
        }
        if (to < info->start[i]) {
            continue;
        }

        if (from == info->start[i]) {
            first = i;
            if (last < 0) {
                last = s_end;
            }
        }
        if (to  == end) {
            last  = i;
            if (first < 0) {
                first = 0;
            }
        }
    }

    for (i=first; i<=last; ++i) {
        if (flag & FLAG_PROTECT_CLEAR) {
            info->protect[i] = 0;
        } else if (flag & FLAG_PROTECT_SET) {
            info->protect[i] = 1;
        }
        if (info->protect[i]) {
            rc = 1;
        }
    }
    return (rc);
}


/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t *info)
{
    int i;

    /* set up sector start adress table */
        if (info->flash_id & FLASH_MAN_SST)
          {
            for (i = 0; i < info->sector_count; i++)
              info->start[i] = base + (i * 0x00010000);
          }
        else
    if (info->flash_id & FLASH_BTYPE) {
        /* set sector offsets for bottom boot block type    */
        info->start[0] = base + 0x00000000;
        info->start[1] = base + 0x00004000;
        info->start[2] = base + 0x00006000;
        info->start[3] = base + 0x00008000;
        for (i = 4; i < info->sector_count; i++) {
            info->start[i] = base + (i * 0x00010000) - 0x00030000;
        }
    } else {
        /* set sector offsets for top boot block type       */
        i = info->sector_count - 1;
        info->start[i--] = base + info->size - 0x00004000;
        info->start[i--] = base + info->size - 0x00006000;
        info->start[i--] = base + info->size - 0x00008000;
        for (; i >= 0; i--) {
            info->start[i] = base + i * 0x00010000;
        }
    }

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
        printf ("missing or unknown FLASH type\n");
        return;
    }

    switch (info->flash_id & FLASH_VENDMASK) {
    case FLASH_MAN_AMD: printf ("AMD ");        break;
    case FLASH_MAN_FUJ: printf ("FUJITSU ");        break;
    case FLASH_MAN_SST: printf ("SST ");        break;
    default:        printf ("Unknown Vendor "); break;
    }

    switch (info->flash_id & FLASH_TYPEMASK) {
    case FLASH_AM400B:  printf ("AM29LV400B (4 Mbit, bottom boot sect)\n");
                break;
    case FLASH_AM400T:  printf ("AM29LV400T (4 Mbit, top boot sector)\n");
                break;
    case FLASH_AM800B:  printf ("AM29LV800B (8 Mbit, bottom boot sect)\n");
                break;
    case FLASH_AM800T:  printf ("AM29LV800T (8 Mbit, top boot sector)\n");
                break;
    case FLASH_AM160B:  printf ("AM29LV160B (16 Mbit, bottom boot sect)\n");
                break;
    case FLASH_AM160T:  printf ("AM29LV160T (16 Mbit, top boot sector)\n");
                break;
    case FLASH_AM320B:  printf ("AM29LV320B (32 Mbit, bottom boot sect)\n");
                break;
    case FLASH_AM320T:  printf ("AM29LV320T (32 Mbit, top boot sector)\n");
                break;
    case FLASH_SST800A: printf ("SST39LF/VF800 (8 Mbit, uniform sector size)\n");
                break;
    case FLASH_SST160A: printf ("SST39LF/VF160 (16 Mbit, uniform sector size)\n");
                break;
    default:        printf ("Unknown Chip Type\n");
                break;
    }

    printf ("  Size: %ld MB in %d Sectors\n",
        info->size >> 20, info->sector_count);

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
            printf ("\n   ");
#if 0 /* test-only */
        printf (" %08lX%s",
            info->start[i],
            info->protect[i] ? " (RO)" : "     "
#else
        printf (" %08lX%s%s",
            info->start[i],
            erased ? " E" : "  ",
            info->protect[i] ? "RO " : "   "
#endif
        );
    }
    printf ("\n");
}

/*-----------------------------------------------------------------------
 */


/*-----------------------------------------------------------------------
 */

/*
 * The following code cannot be run from FLASH!
 */
ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
   short i;
    FLASH_WORD_SIZE value;
    ulong base = (ulong)addr;
        volatile FLASH_WORD_SIZE *addr2 = (FLASH_WORD_SIZE *)addr;

    printf("flash_get_size: \n");
    /* Write auto select command: read Manufacturer ID */
    eieio();
    addr2[ADDR0] = (FLASH_WORD_SIZE)0xAA;
    addr2[ADDR1] = (FLASH_WORD_SIZE)0x55;
    addr2[ADDR0] = (FLASH_WORD_SIZE)0x90;
    value = addr2[0];

    switch (value) {
    case (FLASH_WORD_SIZE)AMD_MANUFACT:
        info->flash_id = FLASH_MAN_AMD;
        break;
    case (FLASH_WORD_SIZE)FUJ_MANUFACT:
        info->flash_id = FLASH_MAN_FUJ;
        break;
    case (FLASH_WORD_SIZE)SST_MANUFACT:
        info->flash_id = FLASH_MAN_SST;
        break;
    default:
        info->flash_id = FLASH_UNKNOWN;
        info->sector_count = 0;
        info->size = 0;
        return (0);         /* no or unknown flash  */
    }
    printf("recognised manufacturer");

    value = addr2[ADDR3];          /* device ID        */
        //        printf("\ndev_code=%x\n", value);

    switch (value) {
    case (FLASH_WORD_SIZE)AMD_ID_LV400T:
        info->flash_id += FLASH_AM400T;
        info->sector_count = 11;
        info->size = 0x00080000;
        break;              /* => 0.5 MB        */

    case (FLASH_WORD_SIZE)AMD_ID_LV400B:
        info->flash_id += FLASH_AM400B;
        info->sector_count = 11;
        info->size = 0x00080000;
        break;              /* => 0.5 MB        */

    case (FLASH_WORD_SIZE)AMD_ID_LV800T:
        info->flash_id += FLASH_AM800T;
        info->sector_count = 19;
        info->size = 0x00100000;
        break;              /* => 1 MB      */

    case (FLASH_WORD_SIZE)AMD_ID_LV800B:
        info->flash_id += FLASH_AM800B;
        info->sector_count = 19;
        info->size = 0x00100000;
        break;              /* => 1 MB      */

    case (FLASH_WORD_SIZE)AMD_ID_LV160T:
        info->flash_id += FLASH_AM160T;
        info->sector_count = 35;
        info->size = 0x00200000;
        break;              /* => 2 MB      */

    case (FLASH_WORD_SIZE)AMD_ID_LV160B:
        info->flash_id += FLASH_AM160B;
        info->sector_count = 35;
        info->size = 0x00200000;
        break;              /* => 2 MB      */
#if 0   /* enable when device IDs are available */
    case (FLASH_WORD_SIZE)AMD_ID_LV320T:
        info->flash_id += FLASH_AM320T;
        info->sector_count = 67;
        info->size = 0x00400000;
        break;              /* => 4 MB      */

    case (FLASH_WORD_SIZE)AMD_ID_LV320B:
        info->flash_id += FLASH_AM320B;
        info->sector_count = 67;
        info->size = 0x00400000;
        break;              /* => 4 MB      */
#endif
    case (FLASH_WORD_SIZE)SST_ID_xF800A:
        info->flash_id += FLASH_SST800A;
        info->sector_count = 16;
        info->size = 0x00100000;
        break;              /* => 1 MB      */

    case (FLASH_WORD_SIZE)SST_ID_xF160A:
        info->flash_id += FLASH_SST160A;
        info->sector_count = 32;
        info->size = 0x00200000;
        break;              /* => 2 MB      */

    case (FLASH_WORD_SIZE)AMD_ID_F040B:
        info->flash_id += FLASH_AM040B;
        info->sector_count = 8;
        info->size = 0x00080000;
        break;              /* => 0.5 MB      */

    default:
        info->flash_id = FLASH_UNKNOWN;
        return (0);         /* => no or unknown flash */

    }

    printf("flash id %lx; sector count %x, size %lx\n", info->flash_id,info->sector_count,info->size);
    /* set up sector start adress table */
        if (info->flash_id & FLASH_MAN_SST)
          {
            for (i = 0; i < info->sector_count; i++)
              info->start[i] = base + (i * 0x00010000);
          }
        else
    if (info->flash_id & FLASH_BTYPE) {
        /* set sector offsets for bottom boot block type    */
        info->start[0] = base + 0x00000000;
        info->start[1] = base + 0x00004000;
        info->start[2] = base + 0x00006000;
        info->start[3] = base + 0x00008000;
        for (i = 4; i < info->sector_count; i++) {
            info->start[i] = base + (i * 0x00010000) - 0x00030000;
        }
    } else {
        /* set sector offsets for top boot block type       */
        i = info->sector_count - 1;
        info->start[i--] = base + info->size - 0x00004000;
        info->start[i--] = base + info->size - 0x00006000;
        info->start[i--] = base + info->size - 0x00008000;
        for (; i >= 0; i--) {
            info->start[i] = base + i * 0x00010000;
        }
    }

    /* check for protected sectors */
    for (i = 0; i < info->sector_count; i++) {
        /* read sector protection at sector address, (A7 .. A0) = 0x02 */
        /* D0 = 1 if protected */
        addr2 = (volatile FLASH_WORD_SIZE *)(info->start[i]);
                if (info->flash_id & FLASH_MAN_SST)
                  info->protect[i] = 0;
                else
                  info->protect[i] = addr2[2] & 1;
    }

    /*
     * Prevent writes to uninitialized FLASH.
     */
    if (info->flash_id != FLASH_UNKNOWN) {
       addr2 = (FLASH_WORD_SIZE *)info->start[0];
        *addr2 = (FLASH_WORD_SIZE)0x00F000F0;   /* reset bank */
    }

    return (info->size);
}


/*-----------------------------------------------------------------------
 */

void    flash_erase (flash_info_t *info, int s_first, int s_last)
{
    volatile FLASH_WORD_SIZE *addr = (FLASH_WORD_SIZE *)(info->start[0]);
    int flag, prot, sect, l_sect;
    ulong start, now, last;

    if ((s_first < 0) || (s_first > s_last)) {
        if (info->flash_id == FLASH_UNKNOWN) {
            printf ("- missing\n");
        } else {
            printf ("- no sectors to erase\n");
        }
        return;
    }

    if ((info->flash_id == FLASH_UNKNOWN) ||
        (info->flash_id > FLASH_AMD_COMP)) {
        printf ("Can't erase unknown flash type - aborted\n");
        return;
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
        printf ("\n");
    }

    l_sect = -1;

    /* Disable interrupts which might cause a timeout here */
    flag = disable_interrupts();

    addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
    addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
    addr[ADDR0] = (FLASH_WORD_SIZE)0x00800080;
    addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
    addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;

    /* Start erase on unprotected sectors */
    for (sect = s_first; sect<=s_last; sect++) {
        if (info->protect[sect] == 0) { /* not protected */
            addr = (FLASH_WORD_SIZE *)(info->start[sect]);
                        if (info->flash_id & FLASH_MAN_SST)
                          {
                            addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
                            addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
                            addr[ADDR0] = (FLASH_WORD_SIZE)0x00800080;
                            addr[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
                            addr[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
                            addr[0] = (FLASH_WORD_SIZE)0x00500050;  /* block erase */
                            udelay(30000);  /* wait 30 ms */
                          }
                        else
                          addr[0] = (FLASH_WORD_SIZE)0x00300030;  /* sector erase */
            l_sect = sect;
        }
    }

    /* re-enable interrupts if necessary */
    if (flag)
        enable_interrupts();

    /* wait at least 80us - let's wait 1 ms */
    udelay (1000);

    /*
     * We wait for the last triggered sector
     */
    if (l_sect < 0)
        goto DONE;

    start = get_timer (0);
    last  = start;
    addr = (FLASH_WORD_SIZE *)(info->start[l_sect]);
    while ((addr[0] & (FLASH_WORD_SIZE)0x00800080) != (FLASH_WORD_SIZE)0x00800080) {
        if ((now = get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
            printf ("Timeout\n");
            return;
        }
        /* show that we're waiting */
        if ((now - last) > 1000) {  /* every second */
            serial_putc ('.');
            last = now;
        }
    }

DONE:
    /* reset to read mode */
    addr = (FLASH_WORD_SIZE *)info->start[0];
    addr[0] = (FLASH_WORD_SIZE)0x00F000F0;  /* reset bank */

    printf (" done\n");
}

/*-----------------------------------------------------------------------
 */

flash_info_t *addr2info (ulong addr)
{
    flash_info_t *info;
    int i;

    for (i=0, info=&flash_info[0]; i<CFG_MAX_FLASH_BANKS; ++i, ++info) {
        if ((addr >= info->start[0]) &&
            (addr <= (info->start[0] + info->size - 1)) ) {
            return (info);
        }
    }

    return (NULL);
}

/*-----------------------------------------------------------------------
 * Copy memory to flash.
 * Make sure all target addresses are within Flash bounds,
 * and no protected sectors are hit.
 * Returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 4 - target range includes protected sectors
 * 8 - target address not in Flash memory
 */
int flash_write (uchar *src, ulong addr, ulong cnt)
{
    int i;
    ulong         end        = addr + cnt - 1;
    flash_info_t *info_first = addr2info (addr);
    flash_info_t *info_last  = addr2info (end );
    flash_info_t *info;

    if (cnt == 0) {
        return (0);
    }

    if (!info_first || !info_last) {
        return (8);
    }

    for (info = info_first; info <= info_last; ++info) {
        ulong b_end = info->start[0] + info->size;  /* bank end addr */
        short s_end = info->sector_count - 1;
        for (i=0; i<info->sector_count; ++i) {
            ulong e_addr = (i == s_end) ? b_end : info->start[i + 1];

            if ((end >= info->start[i]) && (addr < e_addr) &&
                (info->protect[i] != 0) ) {
                return (4);
            }
        }
    }

    /* finally write data to flash */
    for (info = info_first; info <= info_last && cnt>0; ++info) {
        ulong len;

        len = info->start[0] + info->size - addr;
        if (len > cnt)
            len = cnt;
        if ((i = write_buff(info, src, addr, len)) != 0) {
            return (i);
        }
        cnt  -= len;
        addr += len;
        src  += len;
    }
    return (0);
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

static int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
    ulong cp, wp, data;
    int i, l, rc;

    wp = (addr & ~3);   /* get lower word aligned address */

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
static int write_word (flash_info_t *info, ulong dest, ulong data)
{
        volatile FLASH_WORD_SIZE *addr2 = (FLASH_WORD_SIZE *)(info->start[0]);
        volatile FLASH_WORD_SIZE *dest2 = (FLASH_WORD_SIZE *)dest;
        volatile FLASH_WORD_SIZE *data2 = (FLASH_WORD_SIZE *)&data;
    ulong start;
    int flag;
        int i;

    /* Check if Flash is (sufficiently) erased */
    if ((*((volatile FLASH_WORD_SIZE *)dest) &
             (FLASH_WORD_SIZE)data) != (FLASH_WORD_SIZE)data) {
        return (2);
    }
    /* Disable interrupts which might cause a timeout here */
    flag = disable_interrupts();

        for (i=0; i<4/sizeof(FLASH_WORD_SIZE); i++)
          {
            addr2[ADDR0] = (FLASH_WORD_SIZE)0x00AA00AA;
            addr2[ADDR1] = (FLASH_WORD_SIZE)0x00550055;
            addr2[ADDR0] = (FLASH_WORD_SIZE)0x00A000A0;

            dest2[i] = data2[i];

            /* re-enable interrupts if necessary */
            if (flag)
              enable_interrupts();

            /* data polling for D7 */
            start = get_timer (0);
            while ((dest2[i] & (FLASH_WORD_SIZE)0x00800080) !=
                   (data2[i] & (FLASH_WORD_SIZE)0x00800080)) {
              if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
                return (1);
              }
            }
          }

    return (0);
}

/*-----------------------------------------------------------------------
 */
