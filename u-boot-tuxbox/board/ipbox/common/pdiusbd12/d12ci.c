#include <common.h>

#ifdef CONFIG_PDIUSB12

#include "epphal.h"
#include "mainloop.h"
#include "d12ci.h"

extern EPPFLAGS bEPPflags;

void D12_SetAddressEnable(unsigned char bAddress, unsigned char bEnable)
{
	if(bEPPflags.bits.in_isr == 0)
		DISABLE;

	outportb(D12_COMMAND, 0xD0);
	if(bEnable)
		bAddress |= 0x80;
	outportb(D12_DATA, bAddress);

	if(bEPPflags.bits.in_isr == 0)
		ENABLE;
}

void D12_SetEndpointEnable(unsigned char bEnable)
{
	if(bEPPflags.bits.in_isr == 0)
		DISABLE;

	outportb(D12_COMMAND, 0xD8);
	if(bEnable)
		outportb(D12_DATA, 1);
	else
		outportb(D12_DATA, 0);

	if(bEPPflags.bits.in_isr == 0)
		ENABLE;
}

void D12_SetMode(unsigned char bConfig, unsigned char bClkDiv)
{
	if(bEPPflags.bits.in_isr == 0)
		DISABLE;

	outportb(D12_COMMAND, 0xF3);
	outportb(D12_DATA, bConfig);
	outportb(D12_DATA, bClkDiv);

	if(bEPPflags.bits.in_isr == 0)
		ENABLE;
}

void D12_SetDMA(unsigned char bMode)
{
	if(bEPPflags.bits.in_isr == 0)
		DISABLE;

	outportb(D12_COMMAND, 0xFB);
	outportb(D12_DATA, bMode);

	if(bEPPflags.bits.in_isr == 0)
		ENABLE;
}

unsigned short D12_ReadInterruptRegister(void)
{
	unsigned char b1;
	unsigned int j;

	outportb(D12_COMMAND, 0xF4);
	b1 = inportb(D12_DATA);
	j = inportb(D12_DATA);

	j <<= 8;
	j += b1;

	return j;
}

unsigned char D12_SelectEndpoint(unsigned char bEndp)
{
	unsigned char c;

	if(bEPPflags.bits.in_isr == 0)
		DISABLE;

	outportb(D12_COMMAND, bEndp);
	c = inportb(D12_DATA);

	if(bEPPflags.bits.in_isr == 0)
		ENABLE;

	return c;
}

unsigned char D12_ReadLastTransactionStatus(unsigned char bEndp)
{
	outportb(D12_COMMAND, 0x40 + bEndp);
	return inportb(D12_DATA);
}

unsigned char D12_ReadEndpointStatus(unsigned char bEndp)
{
	unsigned char c;

	if(bEPPflags.bits.in_isr == 0)
		DISABLE;

	outportb(D12_COMMAND, 0x80 + bEndp);
	c = inportb(D12_DATA);

	if(bEPPflags.bits.in_isr == 0)
		ENABLE;

	return c;
}

void D12_SetEndpointStatus(unsigned char bEndp, unsigned char bStalled)
{
	if(bEPPflags.bits.in_isr == 0)
		DISABLE;

	outportb(D12_COMMAND, 0x40 + bEndp);
	outportb(D12_DATA, bStalled);

	if(bEPPflags.bits.in_isr == 0)
		ENABLE;
}

void D12_SendResume(void)
{
	outportb(D12_COMMAND, 0xF6);
}

unsigned short D12_ReadCurrentFrameNumber(void)
{
	unsigned short i,j;

	if(bEPPflags.bits.in_isr == 0)
		DISABLE;

	outportb(D12_COMMAND, 0xF5);
	i= inportb(D12_DATA);
	j = inportb(D12_DATA);

	i += (j<<8);

	if(bEPPflags.bits.in_isr == 0)
		ENABLE;

	return i;
}

unsigned short D12_ReadChipID(void)
{
	unsigned short i,j;

	if(bEPPflags.bits.in_isr == 0)
		DISABLE;

	outportb(portbase+D12_COMMAND, 0xFD);
	i=inportb(portbase+D12_DATA);
	j=inportb(portbase+D12_DATA);
	i += (j<<8);

	if(bEPPflags.bits.in_isr == 0)
		ENABLE;

	return i;
}

unsigned char D12_ReadEndpoint(unsigned char endp, unsigned char * buf, unsigned char len)
{
	unsigned char i, j;

	if(bEPPflags.bits.in_isr == 0)
		DISABLE;

	outportb(D12_COMMAND, endp);
	if((inportb(D12_DATA) & D12_FULLEMPTY) == 0) {
		if(bEPPflags.bits.in_isr == 0)
			ENABLE;
		return 0;
	}

	outportb(D12_COMMAND, 0xF0);
	j = inportb(D12_DATA);
	j = inportb(D12_DATA);

	if(j > len)
		j = len;

	for(i=0; i<j; i++)
		*(buf+i) = inportb(D12_DATA);

	outportb(D12_COMMAND, 0xF2);

	if(bEPPflags.bits.in_isr == 0)
		ENABLE;

	return j;
}

// D12_ReadMainEndpoint() added by V2.2 to support double-buffering.
// Caller should assume maxium 128 bytes of returned data.
unsigned char D12_ReadMainEndpoint(unsigned char * buf)
{
	unsigned char i, j, k = 0, bDblBuf = 1;

	if(bEPPflags.bits.in_isr == 0)
		DISABLE;

	outportb(D12_COMMAND, 0x84);
	if( (inportb(D12_DATA) & 0x60) == 0x60)
		bDblBuf = 2;

	while(bDblBuf) {
		outportb(D12_COMMAND, 4);
		if((inportb(D12_DATA) & D12_FULLEMPTY) == 0)
			break;

		outportb(D12_COMMAND, 0xF0);
		j = inportb(D12_DATA);
		j = inportb(D12_DATA);

		for(i=0; i<j; i++)
			*(buf+i+k) = inportb(D12_DATA);

		k += j;

		outportb(D12_COMMAND, 0xF2);

		bDblBuf --;
	}

	if(bEPPflags.bits.in_isr == 0)
		ENABLE;

	return k;
}

unsigned char D12_WriteEndpoint(unsigned char endp, unsigned char * buf, unsigned char len)
{
	unsigned char i;

#if 0
	{
		int a;

		printf( "WE%d (%3d)", endp, len );
		for( a=0; a<len; a++ )
			printf( " %02x", buf[a] );
		puts( "\n" );
	}
#endif

	if(bEPPflags.bits.in_isr == 0)
		DISABLE;

	outportb(D12_COMMAND, endp);
	inportb(D12_DATA);

	outportb(D12_COMMAND, 0xF0);
	outportb(D12_DATA, 0);
	outportb(D12_DATA, len);

	for(i=0; i<len; i++)
		outportb(D12_DATA, *(buf+i));

#if 0
	{
		int a;
		int len;
		unsigned char tmp;

		outportb( D12_COMMAND, 0xF0 );
		inportb( D12_DATA );
		len = inportb( D12_DATA );

//		printf( "v %2d", len );

		for( a=0; a<len; a++ )
		{
//			printf( " %02x", inportb(D12_DATA) );
			tmp = inportb( D12_DATA );
			if( tmp != buf[a] )
			{
				printf( "Oops!! written data is differ. %02x!=%02x. %d\n", tmp, buf[a], a );
			}
		}
//		puts( "\n" );
	}
#endif

	outportb(D12_COMMAND, 0xFA);

	if(bEPPflags.bits.in_isr == 0)
		ENABLE;

	return len;
}

void D12_AcknowledgeEndpoint(unsigned char endp)
{
	outportb(D12_COMMAND, endp);
	outportb(D12_COMMAND, 0xF1);
	if(endp == 0)
		outportb(D12_COMMAND, 0xF2);
}

#endif
