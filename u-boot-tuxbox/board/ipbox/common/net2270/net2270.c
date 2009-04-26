#include <common.h>
#include <asm/io.h>

#ifdef CONFIG_NET2270

#define get_unaligned(ptr) (*(ptr))
#define put_unaligned(val, ptr) ((void)( *(ptr) = (val) ))

#include "mainloop.h"
#include "net2270.h"

extern EPPFLAGS bEPPflags;

u16 read_fifo_full_speed0(u8 * buf)
{
	u8 reg;
	u16 EpAvail, len, total_length = 0;
	

	DISABLE;

	*PAGESEL = EndPoint0;

	fprintf (stderr, "At EP0 Length");
RF0_BEGIN:	
	EpAvail = *EP_AVAIL1 << 8 | *EP_AVAIL0;

	*EP_STAT0 = (1 << DATA_PACKET_RECEIVED_INTERRUPT) |
		    (1 << DATA_OUT_TOKEN_INTERRUPT);

	if (EpAvail > 0x40) {
		fprintf (stderr, "EpAvail error : %04x\n", EpAvail);
		return total_length;
	}

//	fprintf (stderr, "At EP0 Read Length : %d\n", total_length);
	fprintf (stderr, ": %d", total_length);

	len = EpAvail;
	total_length += EpAvail;
	
#if 1
	while (len) {
		*buf = *EP_DATA;
		buf ++;	
		len --;	
	}
#else
	*REGADDRPTR = LOCCTL;
	*REGDATA    = 0x05;	
	u16 byte;
	while (len >= 2) {
		byte = inportw (0x0a);
		put_unaligned (byte, (u16 *)buf);
		buf += 2;
		len -= 2;
	}

	*REGADDRPTR = LOCCTL;
	*REGDATA    = 0x04;	

	if (len) {
		*buf = inportb(0x0a);
	}
#endif
	reg = *EP_STAT0;

	if (reg & (1 << SHORT_PACKET_TRANSFERRED_INTERRUPT))
		reg = *EP_STAT0;

	if (reg & (1 << BUFFER_EMPTY)) {
		if (reg & (1 << SHORT_PACKET_TRANSFERRED_INTERRUPT)) {
			*EP_IRQENB = 0;
			reg = *EP_DATA;

			*EP_STAT0 = (1 << DATA_PACKET_RECEIVED_INTERRUPT) |
				    (1 << DATA_OUT_TOKEN_INTERRUPT) |
				    (1 << DATA_IN_TOKEN_INTERRUPT) |
				    (1 << DATA_PACKET_TRANSMITTED_INTERRUPT) |
				    (1 << NAK_OUT_PACKETS) |
				    (1 << SHORT_PACKET_TRANSFERRED_INTERRUPT);
		}
	} else {
		goto RF0_BEGIN;
	}

	ENABLE;

	fprintf (stderr, "\n");
	return total_length;
}

u16 read_fifo(u8 ep, u8 *buf)
{
	u8 reg;
	u16 EpAvail, len, total_length = 0;
	u16 max = 0x0200;

	DISABLE;

	*PAGESEL = ep;
	if (bEPPflags.bits.full_speed)
		max = 0x0040;

RF_BEGIN:	
	EpAvail = *EP_AVAIL1 << 8 | *EP_AVAIL0;

	*EP_STAT0 = (1 << DATA_PACKET_RECEIVED_INTERRUPT) |
		    (1 << DATA_OUT_TOKEN_INTERRUPT);

	len = EpAvail;
	total_length += EpAvail;
	
#if 1
	while (len) {
		*buf = *EP_DATA;
		buf ++;	
		len --;	
	}
#else
	*REGADDRPTR = LOCCTL;
	*REGDATA    = 0x05;	
	u16 byte;
	while (len >= 2) {
		byte = inportw(0x0a);
		put_unaligned (byte, (u16 *)buf);
		buf += 2;
		len -= 2;
	}

	*REGADDRPTR = LOCCTL;
	*REGDATA    = 0x04;	

	if (len) {
		*buf = inportb(0x0a);
	}
#endif

	reg = *EP_STAT0;

	if (reg & (1 << SHORT_PACKET_TRANSFERRED_INTERRUPT))
		reg = *EP_STAT0;

	if (reg & (1 << BUFFER_EMPTY)) {
		if (reg & (1 << SHORT_PACKET_TRANSFERRED_INTERRUPT)) {
			*EP_IRQENB = 0;
			reg = *EP_DATA;

			*EP_STAT0 = (1 << DATA_PACKET_RECEIVED_INTERRUPT) |
				    (1 << DATA_OUT_TOKEN_INTERRUPT) |
				    (1 << DATA_IN_TOKEN_INTERRUPT) |
				    (1 << DATA_PACKET_TRANSMITTED_INTERRUPT) |
				    (1 << NAK_OUT_PACKETS) |
				    (1 << SHORT_PACKET_TRANSFERRED_INTERRUPT);
		}
	} else {
		goto RF_BEGIN;
	}

	ENABLE;

	return total_length;
}

void send_fifo(u8 ep, u8 * buf, u16 len)
{
	u8 EpStat0;
	u16 EpAvail, XferRemain, ByteCount;

	DISABLE;

	*PAGESEL = ep;

	*EP_TRANSFER2 = (len >> 16);
	*EP_TRANSFER1 = (len >> 8) & 0x00ff;
	*EP_TRANSFER0 = (len & 0x00ff);
	
	XferRemain = len;

	while (XferRemain) {
		*EP_STAT0 = (1 << DATA_PACKET_TRANSMITTED_INTERRUPT) |
			    (1 << DATA_IN_TOKEN_INTERRUPT);
		
		EpStat0 = *EP_STAT0;

		if ((EpStat0 & (1 << BUFFER_EMPTY)) == 0) {
			break;
		}

		EpAvail = *EP_AVAIL0 | (*EP_AVAIL1 << 8);
		
		ByteCount = min(EpAvail, XferRemain);

		XferRemain -= ByteCount;
		
		while (ByteCount) {
			*EP_DATA = *buf;
			buf ++;
			ByteCount --;
		}
	}

	if (XferRemain == 0)
		*EP_IRQENB = 0x00;
	
	*EP_IRQENB = (1 << DATA_PACKET_TRANSMITTED_INTERRUPT) |
		     (1 << DATA_IN_TOKEN_INTERRUPT);
	
	ENABLE;

	return;
}

/* Stalls/clears the selected endpoint
 * 
 * @param ep 	Selected Endpoint
 * @param mode	Stall (=1), Clear (=0) 
 *
 */
void Stall_Ep (u8 ep, u8 mode)
{
	*PAGESEL = ep;

	if (mode) 
		*EP_RSPSET = 0x01;
	else
		*EP_RSPCLR = 0x01;
}

void Standby (void)
{
	*USBCTL0 = 0xE2; // Enable Local Bus Wakeup and disable USB
	
	udelay(1*1000);

	*IRQSTAT1 = 0x76;
	
	udelay(1*1000);
	
	*IRQSTAT1 = 0x08;
	
}

/*
 * Initiate Chip WakeUp/USB WakeUp
 */
void WakeUp (void)
{
	*PAGESEL = EndPoint0;
	
	*USBCTL0 = 0xEA; // Enable USB-Controller

	udelay(1*1000);
	
	*USBCTL1 = 0x08; // Generate Resume Sequence
}
#endif
