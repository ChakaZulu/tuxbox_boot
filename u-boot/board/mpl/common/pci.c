/*-----------------------------------------------------------------------------+
|
|       This source code has been made available to you by IBM on an AS-IS
|       basis.  Anyone receiving this source is licensed under IBM
|       copyrights to use it in any way he or she deems fit, including
|       copying it, modifying it, compiling it, and redistributing it either
|       with or without modifications.  No license under IBM patents or
|       patent applications is to be implied by the copyright license.
|
|       Any user of this software should understand that IBM cannot provide
|       technical support for this software and will not be responsible for
|       any consequences resulting from the use of this software.
|
|       Any person who transfers this source code or any derivative work
|       must include the IBM copyright notice, this paragraph, and the
|       preceding two paragraphs in the transferred software.
|
|       COPYRIGHT   I B M   CORPORATION 1995
|       LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M
+-----------------------------------------------------------------------------*/
/*
 * Adapted for PIP405 03.07.01
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
 *
 * TODO: Clean-up
 */

#include <common.h>
#include <pci.h>
#include "isa.h"

#ifdef CONFIG_405GP
#ifdef CONFIG_PCI

#undef DEBUG

#include "piix4_pci.h"
#include "pci_parts.h"

void pci_pip405_write_regs(struct pci_controller *hose, pci_dev_t dev,
			   struct pci_config_table *entry)
{
	struct pci_pip405_config_entry *table;
	int i;

	table = (struct pci_pip405_config_entry*) entry->priv[0];

	for (i=0; table[i].width; i++)
	{
#ifdef DEBUG
		printf("Reg 0x%02X Value 0x%08lX Width %02d written\n",
		       table[i].index, table[i].val, table[i].width);
#endif

		switch(table[i].width)
		{
		case 1: pci_hose_write_config_byte(hose, dev, table[i].index, table[i].val); break;
		case 2: pci_hose_write_config_word(hose, dev, table[i].index, table[i].val); break;
		case 4: pci_hose_write_config_dword(hose, dev, table[i].index, table[i].val); break;
		}
	}
}


static void pci_pip405_fixup_irq(struct pci_controller *hose, pci_dev_t dev)
{
	unsigned char int_line = 0xff;
	/*
	 * Write pci interrupt line register
	 */
	if(PCI_DEV(dev)==0) /* Device0 = PPC405 -> skip */
		return;
	if(PCI_FUNC(dev)==0)
	{
		/* assuming all function 0 are using their INTA# Pin*/
		int_line=PCI_IRQ_VECTOR(dev);
		pci_hose_write_config_byte(hose, dev, PCI_INTERRUPT_LINE, int_line);
#ifdef DEBUG
		printf("Fixup IRQ: dev %d (%x) int line %d 0x%x\n",
		       PCI_DEV(dev),dev,int_line,int_line);
#endif
	}
}

extern void pci_405gp_init(struct pci_controller *hose);


static struct pci_controller hose = {
  config_table: pci_pip405_config_table,
  fixup_irq: pci_pip405_fixup_irq,
};

void pci_init(void)
{
	/*we want the ptrs to RAM not flash (ie don't use init list)*/
	hose.fixup_irq    = pci_pip405_fixup_irq;
	hose.config_table = pci_pip405_config_table;
	pci_405gp_init(&hose);
}

#endif /* CONFIG_PCI */
#endif /* CONFIG_405GP */
