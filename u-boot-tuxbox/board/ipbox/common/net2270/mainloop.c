#include <common.h>

#ifdef CONFIG_NET2270

#include <command.h>

#include <asm/processor.h>
#include <asm/io.h>
#include <asm/global_data.h>
#ifdef CONFIG_PPC405_GPIO
#include "common/gpio.h"
#endif

#ifdef CONFIG_DGS_FRONT
#include "common/front.h"
#endif
#include "net2270.h"
#include "mainloop.h"
#include "common/usb100.h"
#include "common/chap_9.h"
#include "common/vendor.h"
#include "common/upgrade.h"

//*************************************************************************
//  Public static data
//*************************************************************************

EPPFLAGS bEPPflags =
{
value:	0,
};

/* Control endpoint TX/RX buffers */
CONTROL_XFER ControlData;

/* ISR static vars */
u8 GenEpBuf[EP1_PACKET_SIZE];

char * _NAME_USB_REQUEST_DIRECTION[] =
{
"Host_to_device",
"Device_to_host"
};

char * _NAME_USB_REQUEST_RECIPIENT[] =
{
"Device",
"Interface",
"Endpoint(0)",
"Other"
};

char * _NAME_USB_REQUEST_TYPE[] =
{
"Standard",
"Class",
"Vendor",
"Reserved"
};

char * _NAME_USB_STANDARD_REQUEST[] =
{
"GET_STATUS",
"CLEAR_FEATURE",
"RESERVED",
"SET_FEATURE",
"RESERVED",
"SET_ADDRESS",
"GET_DESCRIPTOR",
"SET_DESCRIPTOR",
"GET_CONFIGURATION",
"SET_CONFIGURATION",
"GET_INTERFACE",
"SET_INTERFACE",
"SYNC_FRAME"
};

int gusbc_debug = 0;

void help_devreq(u8 typ, u8 req)
{
	typ >>= 5;

	if(typ == USB_STANDARD_REQUEST) {
		if(bEPPflags.bits.verbose)
		{
			printf("Request Type = %s, Request = %s.\n", _NAME_USB_REQUEST_TYPE[typ],
					_NAME_USB_STANDARD_REQUEST[req]);
		}
	}
	else {
		if(bEPPflags.bits.verbose)
		{
			printf("Request Type = %s, bRequest = 0x%x.\n", _NAME_USB_REQUEST_TYPE[typ],
				req);
		}
	}
}

void on_exit(void)
{
	DISABLE;

	irq_free_handler( NET2270_IRQ );

	disconnect_USB();

	ENABLE;
}

int usb_client_relocate_code( void )
{
	DECLARE_GLOBAL_DATA_PTR;
	int a;
	unsigned long addr;

	for( a=0; a<sizeof(_NAME_USB_REQUEST_DIRECTION)/sizeof(char*); a++ )
	{
		addr = (unsigned long)_NAME_USB_REQUEST_DIRECTION[a] + gd->reloc_off;
//		printf( "_NAME_USB_REQUEST_DIRECTION 0x%08x -> 0x%08x\n", (int)_NAME_USB_REQUEST_DIRECTION[a], (int)addr );
		_NAME_USB_REQUEST_DIRECTION[a] = (char*)addr;
	}

	for( a=0; a<sizeof(_NAME_USB_REQUEST_RECIPIENT)/sizeof(char*); a++ )
	{
		addr = (unsigned long)_NAME_USB_REQUEST_RECIPIENT[a] + gd->reloc_off;
//		printf( "_NAME_USB_REQUEST_RECIPIENT 0x%08x -> 0x%08x\n", (int)_NAME_USB_REQUEST_RECIPIENT[a], (int)addr );
		_NAME_USB_REQUEST_RECIPIENT[a] = (char*)addr;
	}

	for( a=0; a<sizeof(_NAME_USB_REQUEST_TYPE)/sizeof(char*); a++ )
	{
		addr = (unsigned long)_NAME_USB_REQUEST_TYPE[a] + gd->reloc_off;
//		printf( "_NAME_USB_REQUEST_TYPE 0x%08x -> 0x%08x\n", (int)_NAME_USB_REQUEST_TYPE[a], (int)addr );
		_NAME_USB_REQUEST_TYPE[a] = (char*)addr;
	}

	for( a=0; a<sizeof(_NAME_USB_STANDARD_REQUEST)/sizeof(char*); a++ )
	{
		addr = (unsigned long)_NAME_USB_STANDARD_REQUEST[a] + gd->reloc_off;
//		printf( "_NAME_USB_STANDARD_REQUEST 0x%08x -> 0x%08x\n", (int)_NAME_USB_STANDARD_REQUEST[a], (int)addr );
		_NAME_USB_STANDARD_REQUEST[a] = (char*)addr;
	}

	return 0;
}

/*
 * handle user key input and do host requests.
 */
int main_loop_function( void )
{
	char c;
	static int suspend = 0;
#ifdef CONFIG_DGS_FRONT
	int key;
#endif

	if( gusbc_debug )
	{
		if( tstc() )
		{
			c = getc();
			switch( c )
			{
				/*
				 * abcdefghijklmnopqrstuvwxyz
				 * 123456789abcdef0123456789a
				 */
				case 0x03:		/* ^C - break */
					Standby();
					return 1;
					break;
				case 0x12:		/* ^R - reconnect */
					puts( "^R reconnect usb.\n" );
					bEPPflags.bits.verbose = 0;	/* disable verbose print */
					break;
				case 'v':
					if(bEPPflags.bits.verbose == 0) {
						puts("Verbose = ON.\n");
						DISABLE;
						bEPPflags.bits.verbose = 1;
						ENABLE;
					}
					else {
						puts("Verbose = OFF.\n");
						DISABLE;
						bEPPflags.bits.verbose = 0;
						ENABLE;
					}
					break;
				default:
					break;
			}
		}
	}

	/*
	 * check pwr button.
	 */
#ifdef CONFIG_DGS_FRONT
	key = front_getkey();
	if( key==key_power || key==key_front_power )
	{	// got power off button.
		//while( front_getc() != 0xff );
		return 1;
	}
#endif
	/*
	 * do bottom half.
	 */
	if (bEPPflags.bits.bus_reset) {
		DISABLE;
		bEPPflags.bits.bus_reset = 0;
		ENABLE;
		if( bEPPflags.bits.verbose )
			puts("Bus reset!\n");
	} // if bus reset

	if (bEPPflags.bits.suspend) {
		DISABLE;
		bEPPflags.bits.suspend= 0;
		suspend ++;
		ENABLE;
		suspend_change();
		if( bEPPflags.bits.verbose )
			puts("Suspend change!\n");
	} // if suspend change

	if (bEPPflags.bits.setup_packet){
		DISABLE;
		bEPPflags.bits.setup_packet = 0;
		ENABLE;
		control_handler();
	} // if setup_packet

	if(bEPPflags.bits.setup_dma) {
		if( bEPPflags.bits.verbose )
			puts( "setup_dma\n" );
		DISABLE;
		bEPPflags.bits.setup_dma = 0;
		ENABLE;
		//			setup_dma();
	} // if setup_dma

#if defined(CONFIG_PPC405_GPIO) && defined(NET2270_SUSPEND_GPIO)
	{
		static unsigned long long starttime = 0;
		unsigned long gpiodat;

		/*
		 * check if it is first...
		 */
		if( starttime == 0 )
		{
			starttime = get_ticks();
		}
		else
		{
			if( !gusbc_debug )
			{
				/*
				 * check some seconds passed.
				 */
				if( get_ticks()  > (starttime + NET2270_CONNECT_TIMEOUT * get_tbclk()) )
				{
					gpio_in(0, 0x80000000>>NET2270_SUSPEND_GPIO, &gpiodat );
					if( gpiodat )
					{	/* disconnected... */
						puts( "disconnected...\n" );

						starttime = 0;
					}
					else
					{
					}
				}
			}
		}
	}
#endif

	return 0;
}

#ifdef  USB_TEST
static void chiptest(void)
{
	u16 reg;

	u8 i;

	*REGADDRPTR = CHIPREV;
	reg = *REGDATA;

	printf ("CHIPREV is %04x\n", reg);

	*REGADDRPTR = LOCCTL;
	reg = *REGDATA;

	printf ("LOCCTL is %04x\n", reg);

	for (i = 0; i < 4; i++) {
		*PAGESEL = i;
		printf ("PAGESEL is %02x\n", *PAGESEL);
	}

	for (i = 1; i < 100; i+= 7) {
		*NET_SCRATCH = i;
		printf ("SCRATCH is %02x\n", *NET_SCRATCH);
	}

}
#endif

static int usb_client_main( int argc, char *argv[] )
{
	DECLARE_GLOBAL_DATA_PTR;
	BOOL in_loop = TRUE;
	u16	reg;

#ifdef CONFIG_DGS_FRONT
	front_puts( " USb UPGRADE" );

	/*
	 * wait power key release...
	 */
	{
		unsigned long long starttime;
		int key;

		starttime = get_ticks();

		while( get_ticks() < (starttime+3*get_tbclk()) )
		{
			if( front_tstc() )
			{
				key = front_getkey();
				if( key == key_release || key == key_front_release )
					break;
				starttime = get_ticks();
			}
		}
	}
#endif

	/*
	 * set some default value
	 */
	bEPPflags.value = 0;	/* disable verbose print. */
	/* initiate upg_buffer and status register */
	upg_buffer = (u8*)gd->dgs_upg_buffer;
	//printf( "we have 0x%08x bytes of memory at 0x%08x for download buffer.\n",
	//		NET2270_DOWN_BUFFER, (int)upg_buffer );
	upg_buffer_status = UPGBUF_UNUSED;

	/*
	 * initialize net2270
	 */

	*REGADDRPTR = CHIPREV;
	reg = *REGDATA;

	//printf ("CHIPREV is %02x\n", reg);

	*REGADDRPTR = CHIPREV;
	reg = *REGDATA;

	printf ("CHIPREV is %02x\n", reg);

	//chiptest();

	// Initialize Endpoint Zero
	*PAGESEL = EndPoint0;
	*EP_RSPSET = (1 << HIDE_STATUS_PHASE);

	// Setup chip level interrupt enables by setting these enable bits under
	// IRQENB0 and USBCTL0 registers
	*REGADDRPTR = IRQENB0;
	reg = *REGDATA;

	*REGDATA = reg | (1 << SETUP_PACKET_INTERRUPT_ENABLE) |
		         (1 << ENDPOINT_0_INTERRUPT_ENABLE) | 0;

	reg = *USBCTL0;
	*USBCTL0 = reg | (1 << USB_DETECT_ENABLE) |
		         (1 << USB_ROOT_PORT_WAKEUP_ENABLE) | 0;

	/* register interrupt handler. */
	mtdcr( uictr, mfdcr(uictr)&~(0x80000000>>NET2270_IRQ) );	// level sensitive
	mtdcr( uicpr, mfdcr(uicpr)|(0x80000000>>NET2270_IRQ) );		// negative interrupt
	irq_install_handler (NET2270_IRQ, usb_isr, NULL);

	/* reconnect usb. */
	puts("Connecting USB to PC.\n");

	if( argc>1 )
	{
		if( !strcmp( argv[1], "debug" ) )
		{
			bEPPflags.bits.verbose = 1;
			gusbc_debug = 1;
		}
	}

	/* Main program loop */

	while( in_loop ){
		switch( main_loop_function() )
		{
			case 1:
				in_loop = 0;
				break;
			default:
				break;
		}

		if( upg_buffer_status == UPGBUF_WRITE )
		{
			switch( burn_flash() )
			{
				case 1:
					in_loop = 0;
					break;
				case 2:	/* flash write error. */
					break;
				default:
					break;
			}
		}
	} // Main Loop

	on_exit();

#ifdef CONFIG_DGS_FRONT
	front_puts( " End" );
#endif

	return 0;
}

static int do_usbcmd( cmd_tbl_t *cmdtp, int flag, int argc, char **argv )
{
	return usb_client_main( argc, argv );
}

U_BOOT_CMD(
		usbupg, 2, 0, do_usbcmd,
		"usbupg  - excute usb update program.\n",
		"usbupg <debug>\n     - appand \"debug\" to debug the device.\n" );

void suspend_change(void)
{
}

void disconnect_USB(void)
{
	if( bEPPflags.bits.verbose )
		puts( "disconnect usb.\n" );

	// Disconnect NET2270
	*USBCTL0 = *USBCTL0 & ~(1<<USB_DETECT_ENABLE);
	*REGADDRPTR = IRQENB0;
	*REGDATA = 0x00;
	*REGDATA = 0x00;
}

void stall_ep0(void)
{
	*PAGESEL = EndPoint0;

	*EP_RSPSET = 0x01;
}

void code_transmit(u8 * pRomData, u16 len)
{
	DISABLE;
	ControlData.wCount = 0;
	if (ControlData.wLength > len)
		ControlData.wLength = len;

	ControlData.pData = pRomData;
	if( ControlData.wLength >= EP0_PACKET_SIZE) {
		send_fifo(EndPoint0, ControlData.pData, EP0_PACKET_SIZE);
		ControlData.wCount += EP0_PACKET_SIZE;

		bEPPflags.bits.control_state = USB_TRANSMIT;
		//ENABLE;
	}
	else {
		send_fifo(EndPoint0, pRomData, ControlData.wLength);
		ControlData.wCount += ControlData.wLength;
		//DISABLE;
		bEPPflags.bits.control_state = USB_IDLE;
	}
	ENABLE;
}

void control_handler(void)
{
	u8 type, req;

	type = ControlData.DeviceRequest.bmRequestType & USB_REQUEST_TYPE_MASK;
	req = ControlData.DeviceRequest.bRequest & USB_REQUEST_MASK;

	help_devreq(type, req); // print out device request

	if (type == USB_VENDOR_REQUEST)
		switch (req) {
			case 1:
				get_vendor_descript();
				break;
			case 12:
				vendor_req();
				break;
			default:
				reserved();
				break;
		}
	else
		stall_ep0();
}

//! Setup Packet Interrupt Handler
/*!
*   No parameters/return values
*/

void Setup_Packet_Handler (void)
{
	u8 i;
	u8 setup[8];
	u8 type, req;

	DISABLE;

	*PAGESEL = EndPoint0;
	*EP_IRQENB = 0x00;    // Clear Interrupt Status Bits
	// Clear Interrupt Status Bits
	*EP_STAT0 = (1 << NAK_OUT_PACKETS) |
		    (1 << SHORT_PACKET_TRANSFERRED_INTERRUPT) |
		    (1 << DATA_PACKET_RECEIVED_INTERRUPT) |
		    (1 << DATA_PACKET_TRANSMITTED_INTERRUPT) |
		    (1 << DATA_OUT_TOKEN_INTERRUPT) |
		    (1 << DATA_IN_TOKEN_INTERRUPT);

	for (i=0; i<8; i++)
	{
		*REGADDRPTR = (SETUP0 + i);
		setup [i] = *REGDATA;
	}

	ControlData.DeviceRequest.bmRequestType = setup[0];
	ControlData.DeviceRequest.bRequest = setup[1];
        ControlData.DeviceRequest.wValue = setup[3] << 8 | setup[2];
        ControlData.DeviceRequest.wIndex = setup[5] << 8 | setup[4];
        ControlData.DeviceRequest.wLength = setup[7] << 8 | setup[6];

	if( bEPPflags.bits.verbose )
	{
		printf( "Setup Packet R%02x.%02x V%04x I%04x L%04x\n",
				ControlData.DeviceRequest.bmRequestType,
				ControlData.DeviceRequest.bRequest,
				ControlData.DeviceRequest.wValue,
				ControlData.DeviceRequest.wIndex,
				ControlData.DeviceRequest.wLength);
	}

	type = ControlData.DeviceRequest.bmRequestType & USB_REQUEST_TYPE_MASK;
	req = ControlData.DeviceRequest.bRequest & USB_REQUEST_MASK;

	help_devreq(type, req); // print out device request

	// Enable next Setup Packet
	*IRQSTAT0 = (1 << SETUP_PACKET_INTERRUPT);

	ControlData.wLength = ControlData.DeviceRequest.wLength;
	ControlData.wCount = 0;

	if (ControlData.DeviceRequest.bmRequestType & USB_ENDPOINT_DIRECTION_MASK) {
		if( ((ControlData.DeviceRequest.bmRequestType&USB_REQUEST_TYPE_MASK)==USB_VENDOR_REQUEST)&&
	            ((ControlData.DeviceRequest.bRequest&USB_REQUEST_MASK)==12)&&
		    (ControlData.DeviceRequest.wIndex==GET_FLASH_STATUS)
		  )
		{
			u8 buf[2];
			extern u32 flash_progress_now;
			extern u32 flash_progress_total;

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
						{
							puts( "do nothing...\n" );
						}
						buf[0] = 0x00;
						break;
					}
					else
					{
						if( bEPPflags.bits.verbose )
						{
							printf( "error...(%d)\n", upg_buffer_errorno );
						}
						buf[0] = 0x03;
						buf[1] = upg_buffer_errorno;
						upg_buffer_errorno = 0;	/* clear error status... */
						break;
					}
				case UPGBUF_ERASE:
					if( bEPPflags.bits.verbose )
					{
						printf( "erasing...(%d/%d)\n", flash_progress_now, flash_progress_total );
					}
					buf[0] = 0x01;
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
						printf( "writing...(%d/%d)\n", flash_progress_now, flash_progress_total );
					}
					buf[0] = 0x02;
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
					buf[0] = 0x04;
					break;
			}
			send_fifo(EndPoint0, buf, 2);
		}
		else
		{
			bEPPflags.bits.setup_packet = 0;
			if (type == USB_STANDARD_REQUEST) {
				switch (req) {
					case 0:
						get_status();
						break;
					case 1:
						clear_feature();
						break;
					case 3:
						set_feature();
						break;
					case 6:
						get_descriptor();
						break;
					case 8:
						get_configuration();
						break;
					case 10:
						get_interface();
						break;
					default:
						stall_ep0();
						break;
				}
			} else if (type == USB_VENDOR_REQUEST) {
				switch (req) {
				case 1:
					get_vendor_descript();
					break;
				case 12:
					vendor_req();
					break;
				default:
					reserved();
					break;
				}
			}
			bEPPflags.bits.control_state = USB_IDLE;		/* get command */
		}
	}
	else {
		if (ControlData.DeviceRequest.wLength == 0) {
			bEPPflags.bits.setup_packet = 0;
			if (type == USB_STANDARD_REQUEST) {
				switch (req) {
					case 1:
						clear_feature();
						break;
					case 3:
						set_feature();
						break;
					case 5:
						set_address();
						break;
					case 9:
						set_configuration();
						break;
					case 11:
						set_interface();
						break;
					default:
						stall_ep0();
						break;
				}
			} else if (type == USB_VENDOR_REQUEST) {
				switch (req) {
				case 1:
					get_vendor_descript();
					break;
				case 12:
					vendor_req();
					break;
				default:
					reserved();
					break;
				}
			}

			bEPPflags.bits.control_state = USB_IDLE;		/* set command */
		}
		else {
			if(ControlData.DeviceRequest.wLength > MAX_CONTROLDATA_SIZE) {
				bEPPflags.bits.control_state = USB_IDLE;
			}
			else {
				if( bEPPflags.bits.verbose ) {
					puts( "set command with OUT token\n" );
				}

				bEPPflags.bits.control_state = USB_RECEIVE;	/* set command with OUT token */

				u16 len;

				if( bEPPflags.bits.verbose ) {
					puts( "receive control packet.\n" );
				}

				if(bEPPflags.bits.full_speed) {
					len = read_fifo_full_speed0( ControlData.dataBuffer + ControlData.wCount);
				} else {
					len = read_fifo( EndPoint0, ControlData.dataBuffer + ControlData.wCount);
				}

				ControlData.wCount += len;
				if( len != EP0_PACKET_SIZE || ControlData.wCount >= ControlData.wLength) {
					bEPPflags.bits.setup_packet = 1;
					bEPPflags.bits.control_state = USB_IDLE;
				}

				*PAGESEL = EndPoint0;
				*EP_STAT0 = (1 << DATA_PACKET_RECEIVED_INTERRUPT);
				*EP_IRQENB = (1 << DATA_PACKET_RECEIVED_INTERRUPT_ENABLE);
			}
		} // set command with data
	} // else set command


	*PAGESEL = EndPoint0;
	// Clear Control Status Phase Handshake
	*EP_RSPCLR = (1 << CONTROL_STATUS_PHASE_HANDSHAKE);

	ENABLE;

}

void usb_isr( void *arg )
{
	net2270_interrupt();
}

void net2270_interrupt( void )
{
	bEPPflags.bits.in_isr = 1;

	u8 reg;

	reg = *IRQSTAT1;

	if (reg & (1 << VBUS_INTERRUPT))
	{
		reg = *USBCTL1;
		*IRQSTAT1 = (1 << VBUS_INTERRUPT);
		if (reg & (1 << VBUS_PIN)) {
			puts("VBUS pin is powered\n");
		} else {
			puts("VBUS pin is not powered\n");
		}
	}
	else
        {
		reg = *IRQSTAT0;

		if (reg & (1 << SETUP_PACKET_INTERRUPT))	// Setup packet received
		{
			Setup_Packet_Handler ();
		}

		if (reg & (1 << ENDPOINT_0_INTERRUPT))		// endpoint 0 interrupt
		{
			Ep0_Handler ();
		}

		if (reg & (1 << ENDPOINT_A_INTERRUPT))		// endpoint A interrupt
		{
			EpA_Handler();
		}

		if (reg & (1 << ENDPOINT_B_INTERRUPT))		// endpoint B interrupt
		{
			EpB_Handler();
		}

		if (reg & (1 << ENDPOINT_C_INTERRUPT))		// endpoint C interrupt
		{
			EpC_Handler();
		}


	}

	bEPPflags.bits.in_isr = 0;
}

void Ep0_Handler()
{
	u8 reg;
	u16 len;

	*PAGESEL = EndPoint0;

	reg = *EP_STAT0;

	if( bEPPflags.bits.verbose ) {
		printf("Ep0_Handler. EP_STAT0 : %02x\n", reg);
	}

	if (reg & (1 << DATA_PACKET_RECEIVED_INTERRUPT)) {
		if( bEPPflags.bits.verbose ) {
			puts ("Ep0_Handler Interrupt: Packet Received\n");
		}

		if (bEPPflags.bits.control_state == USB_RECEIVE) {
			if( bEPPflags.bits.verbose ) {
				puts( "receive control packet.\n" );
			}
			//if(bEPPflags.bits.full_speed) {
			//	len = read_fifo_full_speed0( ControlData.dataBuffer + ControlData.wCount);
			//} else {
				len = read_fifo( EndPoint0, ControlData.dataBuffer + ControlData.wCount);
			//}

			ControlData.wCount += len;
			if( len != EP0_PACKET_SIZE || ControlData.wCount >= ControlData.wLength) {
				bEPPflags.bits.setup_packet = 1;
				bEPPflags.bits.control_state = USB_IDLE;
			}
		} else {
			if( bEPPflags.bits.verbose )
			{
				puts( "receive is IDLE\n" );
			}
			bEPPflags.bits.control_state = USB_IDLE;

		}
		*EP_STAT0 = (1 << DATA_PACKET_RECEIVED_INTERRUPT);
		*EP_IRQENB = (1 << DATA_PACKET_RECEIVED_INTERRUPT_ENABLE);

	} else if (reg & (1 << DATA_PACKET_TRANSMITTED_INTERRUPT)) {
		if( bEPPflags.bits.verbose ) {
			puts ("Ep0_Handler interrupt: Packet Sent\n");
		}

		len = ControlData.wLength - ControlData.wCount;

		if (bEPPflags.bits.control_state != USB_TRANSMIT) {
			*EP_IRQENB = 0x00;
			return;
		}

		if( len >= EP0_PACKET_SIZE) {
			puts( "channed\n" );
			send_fifo(EndPoint0, ControlData.pData + ControlData.wCount, EP0_PACKET_SIZE);
			ControlData.wCount += EP0_PACKET_SIZE;

			bEPPflags.bits.control_state = USB_TRANSMIT;
		}
		else if( len != 0) {
			puts( "last\n" );
			send_fifo(EndPoint0, ControlData.pData + ControlData.wCount, len);
			ControlData.wCount += len;

			bEPPflags.bits.control_state = USB_IDLE;
		}
		else if (len == 0){
			puts( "send zero\n" );
			*EP_IRQENB = 0x00;
			bEPPflags.bits.control_state = USB_IDLE;
		}
	}
}

void EpA_Handler()
{
	u8 reg;

	*PAGESEL = EndPointA;

	reg = *EP_IRQENB;

	if( bEPPflags.bits.verbose ) {
		printf("EpA_Handler. EP_IRQENB : %02x\n", reg);
	}

	if ( reg & (1 << DATA_IN_TOKEN_INTERRUPT_ENABLE)) {
		send_fifo(EndPointA, "\x01", 1);
	} else {
		*EP_IRQENB = 0;
	}
	*EP_RSPCLR = (1 << ENDPOINT_HALT) |
        	     (1 << ENDPOINT_TOGGLE) |
        	     (1 << ALT_NAK_OUT_PACKETS) |
		     (1 << CONTROL_STATUS_PHASE_HANDSHAKE) |
		     (1 << ALT_NAK_OUT_PACKETS) |
		     (1 << NAK_OUT_PACKETS_MODE);

}

void read_main_data(u8 ep)
{
	u16 len;

	*PAGESEL = ep;

	if( upg_buffer_status == UPGBUF_DOWNLOAD )
	{

		len = read_fifo(ep, &upg_buffer[upg_buffer_len]);

		upg_buffer_len += len;
		usb_current_block_size -= len;

		if( usb_current_block_size > 0 ) {
		//	puts( "\n" );
			read_main_data(ep);
		}
		else if( usb_current_block_size == 0 ) {
		//	puts( " block completed.\n" );
		}
		else {
			printf( "%d, we got unexpected data.(br:%d)\n", ep, usb_current_block_size );
		}

		if( upg_buffer_len > file_header.file_size ) {
			printf( " we got more data.(in:%d)\n", upg_buffer_len );
		}

	} else {
		printf( " unknown data....(st:%d)\n", upg_buffer_status );
	}

}

void EpB_Handler()
{
	u8 reg;

	*PAGESEL = EndPointB;

	reg = *(EP_IRQENB);

	if( bEPPflags.bits.verbose ) {
		printf("EpB_Handler. EP_IRQENB : %02x\n", reg);
	}

//	if ( reg & (1 << DATA_PACKET_RECEIVED_INTERRUPT_ENABLE)) {
//		read_main_data(EndPointB);
//	} else {
//		read_main_data(EndPointB);
//	}

	read_main_data(EndPointB);
}

void EpC_Handler()
{
	u8 reg;
	u16 len;

	*PAGESEL = EndPointC;

	reg = *EP_STAT0;

	if( bEPPflags.bits.verbose ) {
		printf("EpC_Handler. EP_STAT0 : %02x\n", reg);
	}

	if (reg & (1 << DATA_PACKET_RECEIVED_INTERRUPT)) {
		if( bEPPflags.bits.verbose ) {
			printf ("EpC_Handler Interrupt: Packet Received\n");
		}

		if( upg_buffer_status == UPGBUF_DOWNLOAD )
		{
			len = read_fifo(EndPointC, &upg_buffer[upg_buffer_len]);

			printf( "Read Data Length is %04x\n", len );
			upg_buffer_len += len;
			usb_current_block_size -= len;

			if( usb_current_block_size > 0 )
			{
				printf( "\n" );
			}
			else if( usb_current_block_size == 0 )
			{
				puts( " block completed.\n" );
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

	} else if (reg & (1 << DATA_PACKET_TRANSMITTED_INTERRUPT)) {
		if( bEPPflags.bits.verbose ) {
			puts ("EpC_Handler interrupt: Packet Sent\n");
		}

	}
}

#endif
