#include <common.h>

#ifdef CONFIG_PDIUSB12

#include <command.h>

#include <asm/processor.h>
#include <asm/global_data.h>
#ifdef CONFIG_PPC405_GPIO
#include "common/gpio.h"
#endif

#ifdef CONFIG_DGS_FRONT
#include "common/front.h"
#endif

#include "epphal.h"
#include "d12ci.h"
#include "mainloop.h"
#include "common/usb100.h"
#include "common/chap_9.h"
#include "common/vendor.h"
#include "common/upgrade.h"

//*************************************************************************
// USB protocol function pointer arrays
//*************************************************************************
void (*StandardDeviceRequest[])(void) =
{
	get_status,		// 0
	clear_feature,		// 1
	reserved,		// 2
	set_feature,		// 3
	reserved,		// 4
	set_address,		// 5
	get_descriptor,		// 6
	reserved,		// 7
	get_configuration,	// 8
	set_configuration,	// 9
	get_interface,		// 10
	set_interface,		// 11
	reserved,		// 12
	reserved,		// 13
	reserved,		// 14
	reserved		// 15
};

void (*VendorDeviceRequest[])(void) =
{
	reserved,		// 0
	get_vendor_descript,	// 1
	reserved,		// 2
	reserved,		// 3
	reserved,		// 4
	reserved,		// 5
	reserved,		// 6
	reserved,		// 7
	reserved,		// 8
	reserved,		// 9
	reserved,		// 10
	reserved,		// 11
	vendor_req,		// 12
	reserved,		// 13
	reserved,		// 14
	reserved		// 15
};

//*************************************************************************
//  Public static data
//*************************************************************************

extern EPPFLAGS bEPPflags;
extern unsigned long ClockTicks;
extern unsigned char GenEpBuf[];

CONTROL_XFER ControlData;

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

void help_devreq(unsigned char typ, unsigned char req)
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

	irq_free_handler( PDIUSB_IRQ );

	disconnect_USB();

	ENABLE;
}

int usb_client_relocate_code( void )
{
	DECLARE_GLOBAL_DATA_PTR;
	int a;
	unsigned long addr;

	for( a=0; a<16; a++ )
	{
		addr = (unsigned long)StandardDeviceRequest[a] + gd->reloc_off;
//		printf( "StandardDeviceRequest 0x%08x -> 0x%08x\n", (int)StandardDeviceRequest[a], (int)addr );
		StandardDeviceRequest[a] = (void (*)(void))addr;

		addr = (unsigned long)VendorDeviceRequest[a] + gd->reloc_off;
//		printf( "VendorDeviceRequest 0x%08x -> 0x%08x\n", (int)VendorDeviceRequest[a], (int)addr );
		VendorDeviceRequest[a] = (void (*)(void))addr;
	}

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
					return 1;
					break;
				case 0x12:		/* ^R - reconnect */
					puts( "^R reconnect usb.\n" );
					reconnect_USB();
					//bEPPflags.bits.verbose = 0;	/* disable verbose print */
					break;
				case 'v':
					if(bEPPflags.bits.verbose == 0) {
						puts("Verbose = ON.\n");
						bEPPflags.bits.verbose = 1;
						upg_verbose_message = 1;
					}
					else {
						puts("Verbose = OFF.\n");
						bEPPflags.bits.verbose = 0;
						upg_verbose_message = 0;
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
//		puts( "setup_packet\n" );
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

#if defined(CONFIG_PPC405_GPIO) && defined(PDIUSB_SUSPEND_GPIO)
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
				if( get_ticks()  > (starttime+PDIUSB_CONNECT_TIMEOUT*get_tbclk()) )
				{
					gpio_in( 0, 0x80000000>>PDIUSB_SUSPEND_GPIO, &gpiodat );
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

static int usb_client_main( int argc, char *argv[] )
{
	DECLARE_GLOBAL_DATA_PTR;
	BOOL in_loop = TRUE;

#ifdef CONFIG_DGS_FRONT
	front_puts( "USb UPGRADE" );

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
	/* initiate usb_data_buffer and status register */
	upg_buffer = (unsigned char*)gd->dgs_upg_buffer;
	//printf( "we have 0x%08x bytes of memory at 0x%08x for download buffer.\n",
	//		PDIUSB_DOWN_BUFFER, (int)usb_data_buffer );
	upg_buffer_status = UPGBUF_UNUSED;
	upg_buffer_errorno = 0;

	/*
	 * do chip initialize...
	 */
	if( D12_ReadChipID() != 0x1012 )
	{
		printf( "Chip ID is 0x%04x\n", D12_ReadChipID() );
		puts( "It seems that pdiusb is not working...\n" );
		return -1;
	}

	/* register interrupt handler. */
	mtdcr( uictr, mfdcr(uictr)&~(0x80000000>>PDIUSB_IRQ) );	// level sensitive
	mtdcr( uicpr, mfdcr(uicpr)|(0x80000000>>PDIUSB_IRQ) );		// negative interrupt
	irq_install_handler( PDIUSB_IRQ, usb_isr, NULL );

	/* reconnect usb. */
	puts("Connecting USB to PC.\n");
	reconnect_USB();

	if( argc>1 )
	{
		if( !strcmp( argv[1], "debug" ) )
		{
			bEPPflags.bits.verbose = 1;
			upg_verbose_message = 1;
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
	front_puts( "End" );
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

void stall_ep0(void)
{
	D12_SetEndpointStatus(0, 1);
	D12_SetEndpointStatus(1, 1);
}

void disconnect_USB(void)
{
	if( bEPPflags.bits.verbose )
		puts( "disconnect usb.\n" );
	// Initialize D12 configuration
	D12_SetMode(D12_NOLAZYCLOCK|D12_CLOCKRUNNING, D12_SETTOONE | D12_CLOCK_12M);
}

void connect_USB(void)
{
	if( bEPPflags.bits.verbose )
		puts( "connect usb.\n" );
	// reset event flags
	DISABLE;
	bEPPflags.value = 0;
	ENABLE;

	// V2.1 enable normal+sof interrupt
	D12_SetDMA(D12_ENDP4INTENABLE | D12_ENDP5INTENABLE);	// removed by parkhw00

	// Initialize D12 configuration
	D12_SetMode(D12_NOLAZYCLOCK|D12_CLOCKRUNNING|D12_SOFTCONNECT, D12_SETTOONE | D12_CLOCK_12M);
}


void reconnect_USB(void)
{
	disconnect_USB();

	udelay( 1*1000*1000 );

	connect_USB();
}

void init_unconfig(void)
{
	D12_SetEndpointEnable(0);	/* Disable all endpoints but EPP0. */
}

void init_config(void)
{
	D12_SetEndpointEnable(1);	/* Enable  generic/iso endpoints. */
}

void single_transmit(unsigned char * buf, unsigned char len)
{
	if( len <= EP0_PACKET_SIZE) {
		D12_WriteEndpoint(1, buf, len);
	}
}

void code_transmit(unsigned char * pRomData, unsigned short len)
{
	ControlData.wCount = 0;
	if(ControlData.wLength > len)
		ControlData.wLength = len;

	ControlData.pData = pRomData;
	if( ControlData.wLength >= EP0_PACKET_SIZE) {
		D12_WriteEndpoint(1, ControlData.pData, EP0_PACKET_SIZE);
		ControlData.wCount += EP0_PACKET_SIZE;

		DISABLE;
		bEPPflags.bits.control_state = USB_TRANSMIT;
		ENABLE;
	}
	else {
		D12_WriteEndpoint(1, pRomData, ControlData.wLength);
		ControlData.wCount += ControlData.wLength;
		DISABLE;
		bEPPflags.bits.control_state = USB_IDLE;
		ENABLE;
	}
}

void control_handler(void)
{
	unsigned char type, req;

	type = ControlData.DeviceRequest.bmRequestType & USB_REQUEST_TYPE_MASK;
	req = ControlData.DeviceRequest.bRequest & USB_REQUEST_MASK;

	help_devreq(type, req); // print out device request

	if (type == USB_STANDARD_REQUEST)
		(*StandardDeviceRequest[req])();
	else if (type == USB_VENDOR_REQUEST)
		(*VendorDeviceRequest[req])();
	else
		stall_ep0();
}

#endif
