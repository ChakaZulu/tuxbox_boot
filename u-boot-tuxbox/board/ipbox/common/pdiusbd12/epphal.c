#include <common.h>

#ifdef CONFIG_PDIUSB12

#include "epphal.h"
#include "d12ci.h"
#include "mainloop.h"

#ifndef PDIUSB_BUSTYPE
#define PDIUSB_BUSTYPE unsigned char
#endif

extern EPPFLAGS bEPPflags;

void outportb(unsigned long port, unsigned char val)
{
	*((volatile PDIUSB_BUSTYPE*)(PDIUSB_IOBASE+port)) = val;
}

unsigned char inportb(unsigned long port)
{
	return *((volatile PDIUSB_BUSTYPE*)(PDIUSB_IOBASE+port));
}

#endif
