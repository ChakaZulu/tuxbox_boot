#include <common.h>

#ifdef CONFIG_PDIUSB12

#include "epphal.h"
#include "d12ci.h"
#include "mainloop.h"
#include "common/usb100.h"
#include "common/vendor.h"
#include "common/upgrade.h"

extern void bus_reset(void);

extern void ep0_txdone(void);
extern void ep0_rxdone(void);

extern void ep1_txdone(void);
extern void ep1_rxdone(void);

extern void main_txdone(void);
extern void main_rxdone(void);

extern void dma_eot(void);

//*************************************************************************
//  Public static data
//*************************************************************************

EPPFLAGS bEPPflags =
{
value:	0,
};

/* Control endpoint TX/RX buffers */
extern CONTROL_XFER ControlData;

/* ISR static vars */
unsigned char GenEpBuf[EP1_PACKET_SIZE];

void usb_isr( void *arg )
{
	fn_usb_isr();
}

void fn_usb_isr( void )
{
	unsigned int i_st;

	bEPPflags.bits.in_isr = 1;

	i_st = D12_ReadInterruptRegister();
	if( bEPPflags.bits.verbose )
	{
		//printf( "int_stat 0x%04x\n", i_st );
	}

	if(i_st != 0) {
		if(i_st & D12_INT_BUSRESET) {
			bus_reset();
			bEPPflags.bits.bus_reset = 1;
		}
		else {
			if(i_st & D12_INT_EOT)
				dma_eot();

			if(i_st & D12_INT_SUSPENDCHANGE)
				bEPPflags.bits.suspend = 1;

			if(i_st & D12_INT_ENDP0IN)	// control
				ep0_txdone();
			if(i_st & D12_INT_ENDP0OUT)	// control
				ep0_rxdone();
			if(i_st & D12_INT_ENDP1IN)	// endpoint 1
				ep1_txdone();
			if(i_st & D12_INT_ENDP1OUT)	// endpoint 1
				ep1_rxdone();
			if(i_st & D12_INT_ENDP2IN)	// main
				main_txdone();
			if(i_st & D12_INT_ENDP2OUT)
				main_rxdone();
		}
	}

	bEPPflags.bits.in_isr = 0;
}

void bus_reset(void)
{
}

void ep0_rxdone(void)
{
	unsigned char ep_last, i;

	ep_last = D12_ReadLastTransactionStatus(0); // Clear interrupt flag
#if 0
	if( bEPPflags.bits.verbose )
		printf( "rxdone last transaction 0x%02x\n", ep_last );
#endif
	if( ep_last & 0x1e )
		printf( "%s error 0x%02x\n", (ep_last & 0x1e) >> 1 );

	if (ep_last & D12_SETUPPACKET)
	{

		ControlData.wLength = 0;
		ControlData.wCount = 0;

		if( D12_ReadEndpoint(0, (unsigned char *)(&(ControlData.DeviceRequest)),
			sizeof(ControlData.DeviceRequest)) != sizeof(DEVICE_REQUEST) ) {

			D12_SetEndpointStatus(0, 1);
			D12_SetEndpointStatus(1, 1);
			bEPPflags.bits.control_state = USB_IDLE;

			return;
		}

		ControlData.DeviceRequest.wValue =
			SWAP(ControlData.DeviceRequest.wValue);
		ControlData.DeviceRequest.wIndex =
			SWAP(ControlData.DeviceRequest.wIndex);
		ControlData.DeviceRequest.wLength =
			SWAP(ControlData.DeviceRequest.wLength);

		if( bEPPflags.bits.verbose )
		{
			printf( "Device request bmRequestType 0x%02x\n",
					ControlData.DeviceRequest.bmRequestType );
			printf( "Device request bRequest      0x%02x\n",
					ControlData.DeviceRequest.bRequest );
			printf( "Device request value  0x%04x\n",
					ControlData.DeviceRequest.wValue );
			printf( "Device request index  0x%04x\n",
					ControlData.DeviceRequest.wIndex );
			printf( "Device request length 0x%04x\n",
					ControlData.DeviceRequest.wLength );
		}

		// Acknowledge setup here to unlock in/out endp
		D12_AcknowledgeEndpoint(0);
		D12_AcknowledgeEndpoint(1);

		ControlData.wLength = ControlData.DeviceRequest.wLength;
		ControlData.wCount = 0;

		if (ControlData.DeviceRequest.bmRequestType & (unsigned char)USB_ENDPOINT_DIRECTION_MASK) {
			if(
					((ControlData.DeviceRequest.bmRequestType&USB_REQUEST_TYPE_MASK)==USB_VENDOR_REQUEST)&&
					((ControlData.DeviceRequest.bRequest&USB_REQUEST_MASK)==12)&&
					(ControlData.DeviceRequest.wIndex==GET_FLASH_STATUS)
			  )
			{
				char buf[2];
				extern unsigned int flash_progress_now;
				extern unsigned int flash_progress_total;


				if(bEPPflags.bits.verbose)
				{
					puts( "get flash status. " );
				}
				/*
				 * send flash status...
				 */
				buf[1] = 0;
				switch( upg_buffer_status )
				{
					case UPGBUF_UNUSED:
						if( upg_buffer_errorno == 0 )
						{
							if( bEPPflags.bits.verbose )
								puts( "do nothing...\n" );
							buf[0] = 0;
							break;
						}
						else
						{
							if( bEPPflags.bits.verbose )
							{
								printf( "error...(%d)\n", upg_buffer_errorno );
							}
							buf[0] = 3;
							buf[1] = upg_buffer_errorno;
							upg_buffer_errorno = 0;	/* clear error status... */
							break;
						}
					case UPGBUF_ERASE:
						if( bEPPflags.bits.verbose )
						{
							printf( "erasing...(%d/%d)\n",
									flash_progress_now, flash_progress_total );
						}
						buf[0] = 1;
						if( flash_progress_total )
						{
							if( flash_progress_total >= flash_progress_now )
							{
								buf[1] = flash_progress_now * 100 / flash_progress_total;
							}
						}
						break;
					case UPGBUF_WRITE:
						if( bEPPflags.bits.verbose )
						{
							printf( "writing...(%d/%d)\n",
									flash_progress_now, flash_progress_total );
						}
						buf[0] = 2;
						if( flash_progress_total )
						{
							if( flash_progress_total >= flash_progress_now )
							{
								buf[1] = flash_progress_now * 100 / flash_progress_total;
							}
						}
						break;
					default:
						if( bEPPflags.bits.verbose )
						{
							printf( "%d: unknown...\n", upg_buffer_status );
						}
						buf[0] = 4;
						break;
				}
				D12_WriteEndpoint( 1, buf, 2 );
			}
			else
			{
				bEPPflags.bits.setup_packet = 1;
				bEPPflags.bits.control_state = USB_IDLE;		/* get command */
			}
		}
		else {
			if (ControlData.DeviceRequest.wLength == 0) {
				bEPPflags.bits.setup_packet = 1;
				bEPPflags.bits.control_state = USB_IDLE;		/* set command */
			}
			else {
				if(ControlData.DeviceRequest.wLength > MAX_CONTROLDATA_SIZE) {
					bEPPflags.bits.control_state = USB_IDLE;
					D12_SetEndpointStatus(0, 1);
					D12_SetEndpointStatus(1, 1);
				}
				else {
					//printf( "set command with OUT token\n" );
					bEPPflags.bits.control_state = USB_RECEIVE;	/* set command with OUT token */
				}
			} // set command with data
		} // else set command
	} // if setup packet

	else if (bEPPflags.bits.control_state == USB_RECEIVE) {
		if( bEPPflags.bits.verbose )
		{
			puts( "receive control packet.\n" );
		}
		i =	D12_ReadEndpoint(0, ControlData.dataBuffer + ControlData.wCount,
			EP0_PACKET_SIZE);

		ControlData.wCount += i;
		if( i != EP0_PACKET_SIZE || ControlData.wCount >= ControlData.wLength) {
			bEPPflags.bits.setup_packet = 1;
			bEPPflags.bits.control_state = USB_IDLE;
		}
	}

	else {
		if( bEPPflags.bits.verbose )
		{
			puts( "receive is IDLE\n" );
		}
		bEPPflags.bits.control_state = USB_IDLE;
	}
}

void ep0_txdone(void)
{
	short i = ControlData.wLength - ControlData.wCount;
	unsigned char ep_last;

	ep_last = D12_ReadLastTransactionStatus(1); // Clear interrupt flag
	if( ep_last & 0x1e )
		printf( "%s error 0x%02x\n", (ep_last & 0x1e) >> 1 );

	if (bEPPflags.bits.control_state != USB_TRANSMIT)
		return;

	if( i >= EP0_PACKET_SIZE) {
		//puts( "channed\n" );
		D12_WriteEndpoint(1, ControlData.pData + ControlData.wCount, EP0_PACKET_SIZE);
		ControlData.wCount += EP0_PACKET_SIZE;

		bEPPflags.bits.control_state = USB_TRANSMIT;
	}
	else if( i != 0) {
		//puts( "last\n" );
		D12_WriteEndpoint(1, ControlData.pData + ControlData.wCount, i);
		ControlData.wCount += i;

		bEPPflags.bits.control_state = USB_IDLE;
	}
	else if (i == 0){
		//puts( "send zero\n" );
		D12_WriteEndpoint(1, 0, 0); // Send zero packet at the end ???

		bEPPflags.bits.control_state = USB_IDLE;
	}
}

void dma_eot(void)
{
#if 0		// we will not use dma... by parkhw00
	if(bEPPflags.bits.dma_state == DMA_PENDING)
		bEPPflags.bits.setup_dma = 1;
	else
		bEPPflags.bits.dma_state = DMA_IDLE;
#endif
}

void ep1_txdone(void)
{
	unsigned char ep_last;

	ep_last = D12_ReadLastTransactionStatus(3); /* Clear interrupt flag */
	puts( "ep1 txdone.\n" );
	if( ep_last & 0x1e )
		printf( "%s error 0x%02x\n", (ep_last & 0x1e) >> 1 );
}

void ep1_rxdone(void)
{
	unsigned char len;
	unsigned char ep_last;

	ep_last = D12_ReadLastTransactionStatus(2); /* Clear interrupt flag */
	puts( "ep1 rxdone." );
	if( ep_last & 0x1e )
		printf( "%s error 0x%02x\n", (ep_last & 0x1e) >> 1 );

	if( 0 )
	{
		len = D12_ReadEndpoint(2, GenEpBuf, sizeof(GenEpBuf));
		printf( " %2d\n", len );

		{
			int a;
			for( a=0; a<len; a++ )
				printf( " %02x", GenEpBuf[a] );
			puts( "\n" );
			for( a=0; a<len; a++ )
				printf( "%c", ((' '<=GenEpBuf[a])&&(GenEpBuf[a]<='~'))?GenEpBuf[a]:'.' );
			puts( "\n" );
		}

		if(len != 0)
			bEPPflags.bits.ep1_rxdone = 1;
	}
	else
	{
		puts( " unknown data.\n" );
	}
}

void main_txdone(void)
{
#if 0
#ifndef __C51__
	unsigned short len;
	unsigned char *fp;
	unsigned short seg, off;
#endif
#endif
	unsigned char ep_last;

	ep_last = D12_ReadLastTransactionStatus(5); /* Clear interrupt flag */
	puts( "main_tx_done....\n" );
	if( ep_last & 0x1e )
		printf( "%s error 0x%02x\n", (ep_last & 0x1e) >> 1 );

#if 0
#ifndef __C51__
	seg = (ioBuffer + ioCount)>>4;
	off = (ioBuffer + ioCount)&0xf;
	fp = MK_FP(seg, off);

	len = ioSize - ioCount;
	if(len == 0) {
		if(bEPPflags.bits.dma_state == DMA_PENDING)
			bEPPflags.bits.setup_dma = 1;
		else
			bEPPflags.bits.dma_state = DMA_IDLE;
	}
	else {
		if(len > 64)
			len = 64;
		len = D12_WriteEndpoint(5, fp, len);
		ioCount += len;
	}
#endif
#endif
}

void main_rxdone(void)
{
	int len;
	unsigned char ep_last;

	ep_last = D12_ReadLastTransactionStatus(4); /* Clear interrupt flag */
	//puts( "main_rxdone." );
	if( ep_last & 0x1e )
		printf( "%s error 0x%02x\n", (ep_last & 0x1e) >> 1 );

	if( upg_buffer_status == UPGBUF_DOWNLOAD )
	{
		len = D12_ReadMainEndpoint( &upg_buffer[upg_buffer_len] );
		//printf( " %3x", len );
		if( len == 0 )
			printf( "empty main rx.(%02x)\n", ep_last );

		upg_buffer_len += len;
		usb_current_block_size -= len;

		if( usb_current_block_size > 0 )
		{
			//puts( "\n" );
		}
		else if( usb_current_block_size == 0 )
		{
			//puts( " block completed.\n" );
		}
		else
		{
			printf( " we got unexpected data.(br:%d)\n", usb_current_block_size );
		}

		if( upg_buffer_len > file_header.file_size )
		{
			printf( " we got more data.(in:%d)\n", upg_buffer_len );
		}
	}
	else
	{
		printf( " unknown data....(st:%d)\n", upg_buffer_status );
	}
}

#endif
