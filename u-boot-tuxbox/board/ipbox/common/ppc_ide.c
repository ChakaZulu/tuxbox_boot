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
 *
 */
/* ide.c - ide support functions */


/*

detected IOADDR: f2000000/f4000000
DMA addr: fce00000
using irq 25

*/


#include <common.h>
#if defined(CFG_CMD_IDE)
#include <ata.h>
#include <ide.h>
#include <pci.h>
#include <asm/processor.h>


#define DCRN_EBIMC_BASE         0x070

#define DCRN_BRCRH0     (DCRN_EBIMC_BASE + 0x0) /* Bus Region Config High 0 */
#define DCRN_BRCRH1     (DCRN_EBIMC_BASE + 0x1) /* Bus Region Config High 1 */
#define DCRN_BRCRH2     (DCRN_EBIMC_BASE + 0x2) /* Bus Region Config High 2 */
#define DCRN_BRCRH3     (DCRN_EBIMC_BASE + 0x3) /* Bus Region Config High 3 */
#define DCRN_BRCRH4     (DCRN_EBIMC_BASE + 0x4) /* Bus Region Config High 4 */
#define DCRN_BRCRH5     (DCRN_EBIMC_BASE + 0x5) /* Bus Region Config High 5 */
#define DCRN_BRCRH6     (DCRN_EBIMC_BASE + 0x6) /* Bus Region Config High 6 */
#define DCRN_BRCRH7     (DCRN_EBIMC_BASE + 0x7) /* Bus Region Config High 7 */
#define DCRN_BRCR0      (DCRN_EBIMC_BASE + 0x10)        /* BRC 0 */
#define DCRN_BRCR1      (DCRN_EBIMC_BASE + 0x11)        /* BRC 1 */
#define DCRN_BRCR2      (DCRN_EBIMC_BASE + 0x12)        /* BRC 2 */
#define DCRN_BRCR3      (DCRN_EBIMC_BASE + 0x13)        /* BRC 3 */
#define DCRN_BRCR4      (DCRN_EBIMC_BASE + 0x14)        /* BRC 4 */
#define DCRN_BRCR5      (DCRN_EBIMC_BASE + 0x15)        /* BRC 5 */
#define DCRN_BRCR6      (DCRN_EBIMC_BASE + 0x16)        /* BRC 6 */
#define DCRN_BRCR7      (DCRN_EBIMC_BASE + 0x17)        /* BRC 7 */
#define DCRN_BEAR0      (DCRN_EBIMC_BASE + 0x20)        /* Bus Error Address Register */
#define DCRN_BESR0      (DCRN_EBIMC_BASE + 0x21)        /* Bus Error Status Register */
#define DCRN_BIUCR      (DCRN_EBIMC_BASE + 0x2A)        /* Bus Interfac Unit Ctrl Reg */


extern ulong ide_bus_offset[CFG_IDE_MAXBUS];

int ide_preinit (void)
{
	int status;

	pci_dev_t devbusfn;
	int l;

	status = 0;
	for (l = 0; l < CFG_IDE_MAXBUS; l++) {
		ide_bus_offset[l] = -ATA_STATUS;
	}

	ide_bus_offset[0] = ((mfdcr(DCRN_BRCR2) & 0xff000000) >> 4) | 0xf0000000;
	printf("IDE detected on IOADDR: %08x",ide_bus_offset[0]);

#if CFG_IDE_MAXBUS > 1
	ide_bus_offset[1] = ((mfdcr(DCRN_BRCR3) & 0xff000000) >> 4) | 0xf0000000;
	printf("/%08x",ide_bus_offset[1]);
#endif

	puts("\n");
	return (status);
}

void ide_set_reset (int flag) {
	puts ("ide reset not yet implemented\n");
	return;
}

#endif /* of CONFIG_CMDS_IDE */
