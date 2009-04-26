#ifndef __EPPHAL_H__
#define __EPPHAL_H__

#include <common.h>

#define portbase 0

#define D12_COMMAND	(0x1<<(31-PDIUSB_AD_ADDRESS))
#define D12_DATA	(0)

extern void outportb(unsigned long, unsigned char);
extern unsigned char inportb(unsigned long);

#define DISABLE	disable_interrupts()
#define ENABLE	enable_interrupts()

#endif
