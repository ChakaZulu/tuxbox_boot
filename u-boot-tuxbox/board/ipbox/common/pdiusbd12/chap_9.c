#include <common.h>

#ifdef CONFIG_PDIUSB12

#include "epphal.h"
#include "d12ci.h"
#include "mainloop.h"
#include "common/usb100.h"
#include "common/chap_9.h"
#include "common/vendor.h"
#include "common/upgrade.h"

#define NUM_ENDPOINTS	4

#define CONFIG_DESCRIPTOR_LENGTH    sizeof(USB_CONFIGURATION_DESCRIPTOR) \
				+ sizeof(USB_INTERFACE_DESCRIPTOR) \
				+ (NUM_ENDPOINTS * sizeof(USB_ENDPOINT_DESCRIPTOR))

extern CONTROL_XFER ControlData;
extern EPPFLAGS bEPPflags;

USB_DEVICE_DESCRIPTOR DeviceDescr =
{
	sizeof(USB_DEVICE_DESCRIPTOR),
	USB_DEVICE_DESCRIPTOR_TYPE,
	SWAP(0x0100),
	USB_CLASS_CODE_TEST_CLASS_DEVICE,
	0, 0,
	EP0_PACKET_SIZE,
	SWAP(0x4761),
	SWAP(0x0001),
	SWAP(0x0100),
	0, 0, 0,
	1
};

USB_CONFIGURATION_DESCRIPTOR ConfigDescr =
{
	sizeof(USB_CONFIGURATION_DESCRIPTOR),
	USB_CONFIGURATION_DESCRIPTOR_TYPE,
	SWAP(CONFIG_DESCRIPTOR_LENGTH),
	1,
	1,
	0,
//	0x60,
	0xe0,	// by parkhw00
//	0x1
	0x0	// by parkhw00
};

USB_INTERFACE_DESCRIPTOR InterfaceDescr =
{
	sizeof(USB_INTERFACE_DESCRIPTOR),
	USB_INTERFACE_DESCRIPTOR_TYPE,
	0,
	0,
	NUM_ENDPOINTS,
	USB_CLASS_CODE_TEST_CLASS_DEVICE,
	USB_SUBCLASS_CODE_TEST_CLASS_D12,
	USB_PROTOCOL_CODE_TEST_CLASS_D12,
	0
};

USB_ENDPOINT_DESCRIPTOR EP1_TXDescr =
{
	sizeof(USB_ENDPOINT_DESCRIPTOR),
	USB_ENDPOINT_DESCRIPTOR_TYPE,
	0x81,
	USB_ENDPOINT_TYPE_INTERRUPT,
	SWAP(EP1_PACKET_SIZE),
	10
};

USB_ENDPOINT_DESCRIPTOR EP1_RXDescr =
{
	sizeof(USB_ENDPOINT_DESCRIPTOR),
	USB_ENDPOINT_DESCRIPTOR_TYPE,
	0x1,
	USB_ENDPOINT_TYPE_INTERRUPT,
	SWAP(EP1_PACKET_SIZE),
	10
};

USB_ENDPOINT_DESCRIPTOR EP2_TXDescr =
{
	sizeof(USB_ENDPOINT_DESCRIPTOR),
	USB_ENDPOINT_DESCRIPTOR_TYPE,
	0x82,
	USB_ENDPOINT_TYPE_BULK,
	SWAP(EP2_PACKET_SIZE),
	10
};

USB_ENDPOINT_DESCRIPTOR EP2_RXDescr =
{
	sizeof(USB_ENDPOINT_DESCRIPTOR),
	USB_ENDPOINT_DESCRIPTOR_TYPE,
	0x2,
	USB_ENDPOINT_TYPE_BULK,
	SWAP(EP2_PACKET_SIZE),
	10
};

//*************************************************************************
// USB Protocol Layer
//*************************************************************************

void reserved(void)
{
	puts( "called reserved function.\n" );

	stall_ep0();
}

//*************************************************************************
// USB standard device requests
//*************************************************************************

void get_status(void)
{
	unsigned char endp, txdat[2];
	unsigned char bRecipient = ControlData.DeviceRequest.bmRequestType & USB_RECIPIENT;
	unsigned char c;

	if (bRecipient == USB_RECIPIENT_DEVICE) {
		if(bEPPflags.bits.remote_wakeup == 1)
			txdat[0] = 3;
		else
			txdat[0] = 1;
		txdat[1]=0;
		single_transmit(txdat, 2);
	} else if (bRecipient == USB_RECIPIENT_INTERFACE) {
		txdat[0]=0;
		txdat[1]=0;
		single_transmit(txdat, 2);
	} else if (bRecipient == USB_RECIPIENT_ENDPOINT) {
		endp = (unsigned char)(ControlData.DeviceRequest.wIndex & MAX_ENDPOINTS);
		if (ControlData.DeviceRequest.wIndex & (unsigned char)USB_ENDPOINT_DIRECTION_MASK)
			c = D12_SelectEndpoint(endp*2 + 1);	/* Control-in */
		else
			c = D12_SelectEndpoint(endp*2);	/* Control-out */
		if(c & D12_STALL)
			txdat[0] = 1;
		else
			txdat[0] = 0;
		txdat[1] = 0;
		single_transmit(txdat, 2);
	} else
		stall_ep0();
}

void clear_feature(void)
{
	unsigned char endp;
	unsigned char bRecipient = ControlData.DeviceRequest.bmRequestType & USB_RECIPIENT;

	return;	// by parkhw00 for debug.

	if (bRecipient == USB_RECIPIENT_DEVICE
		&& ControlData.DeviceRequest.wValue == USB_FEATURE_REMOTE_WAKEUP) {
		DISABLE;
		bEPPflags.bits.remote_wakeup = 0;
		ENABLE;
		single_transmit(0, 0);
	}
	else if (bRecipient == USB_RECIPIENT_ENDPOINT
		&& ControlData.DeviceRequest.wValue == USB_FEATURE_ENDPOINT_STALL) {
		endp = (unsigned char)(ControlData.DeviceRequest.wIndex & MAX_ENDPOINTS);
		if (ControlData.DeviceRequest.wIndex & (unsigned char)USB_ENDPOINT_DIRECTION_MASK)
			/* clear TX stall for IN on EPn. */
			D12_SetEndpointStatus(endp*2 + 1, 0);
		else
			/* clear RX stall for OUT on EPn. */
			D12_SetEndpointStatus(endp*2, 0);
		single_transmit(0, 0);
	} else
		stall_ep0();
}

void set_feature(void)
{
	unsigned char endp;
	unsigned char bRecipient = ControlData.DeviceRequest.bmRequestType & USB_RECIPIENT;

	if (bRecipient == USB_RECIPIENT_DEVICE
		&& ControlData.DeviceRequest.wValue == USB_FEATURE_REMOTE_WAKEUP) {
		DISABLE;
		bEPPflags.bits.remote_wakeup = 1;
		ENABLE;
		single_transmit(0, 0);
	}
	else if (bRecipient == USB_RECIPIENT_ENDPOINT
		&& ControlData.DeviceRequest.wValue == USB_FEATURE_ENDPOINT_STALL) {
		endp = (unsigned char)(ControlData.DeviceRequest.wIndex & MAX_ENDPOINTS);
		if (ControlData.DeviceRequest.wIndex & (unsigned char)USB_ENDPOINT_DIRECTION_MASK)
			/* clear TX stall for IN on EPn. */
			D12_SetEndpointStatus(endp*2 + 1, 1);
		else
			/* clear RX stall for OUT on EPn. */
			D12_SetEndpointStatus(endp*2, 1);
		single_transmit(0, 0);
	} else
		stall_ep0();
}

void set_address(void)
{
//	printf( "set address %d\n", ControlData.DeviceRequest.wValue & DEVICE_ADDRESS_MASK );
	D12_SetAddressEnable((unsigned char)(ControlData.DeviceRequest.wValue &
		DEVICE_ADDRESS_MASK), 1);
	single_transmit(0, 0);
}

void get_descriptor(void)
{
	unsigned char bDescriptor = MSB(ControlData.DeviceRequest.wValue);

	if (bDescriptor == USB_DEVICE_DESCRIPTOR_TYPE) {
		code_transmit((unsigned char *)&DeviceDescr, sizeof(USB_DEVICE_DESCRIPTOR));
#if 0
		if( bEPPflags.bits.verbose )
		{
			int a;
			for( a=0; a<sizeof(USB_DEVICE_DESCRIPTOR); )
			{
				printf( " %02x", ((unsigned char*)&DeviceDescr)[a] );
				a ++;
				if( !(a%16) )
					puts( "\n" );
			}
			puts( "\n" );
		}
#endif
	} else if (bDescriptor == USB_CONFIGURATION_DESCRIPTOR_TYPE) {
//		unsigned char buf[CONFIG_DESCRIPTOR_LENGTH];
		unsigned char *buf = upg_buffer;	// we use usb data buffer for temp.
							// usb_data_buffer dont have any valid data yet.


		memcpy( &buf[0],
				&ConfigDescr,
				sizeof(USB_CONFIGURATION_DESCRIPTOR) );
		memcpy( &buf[sizeof(USB_CONFIGURATION_DESCRIPTOR)],
				&InterfaceDescr,
				sizeof(USB_INTERFACE_DESCRIPTOR) );
		memcpy( &buf[sizeof(USB_CONFIGURATION_DESCRIPTOR)+sizeof(USB_INTERFACE_DESCRIPTOR)+0*sizeof(USB_ENDPOINT_DESCRIPTOR)],
				&EP1_TXDescr,
				sizeof(USB_ENDPOINT_DESCRIPTOR) );
		memcpy( &buf[sizeof(USB_CONFIGURATION_DESCRIPTOR)+sizeof(USB_INTERFACE_DESCRIPTOR)+1*sizeof(USB_ENDPOINT_DESCRIPTOR)],
				&EP1_RXDescr,
				sizeof(USB_ENDPOINT_DESCRIPTOR) );
		memcpy( &buf[sizeof(USB_CONFIGURATION_DESCRIPTOR)+sizeof(USB_INTERFACE_DESCRIPTOR)+2*sizeof(USB_ENDPOINT_DESCRIPTOR)],
				&EP2_TXDescr,
				sizeof(USB_ENDPOINT_DESCRIPTOR) );
		memcpy( &buf[sizeof(USB_CONFIGURATION_DESCRIPTOR)+sizeof(USB_INTERFACE_DESCRIPTOR)+3*sizeof(USB_ENDPOINT_DESCRIPTOR)],
				&EP2_RXDescr,
				sizeof(USB_ENDPOINT_DESCRIPTOR) );

#if 0
		if( bEPPflags.bits.verbose )
		{
			int a;
			for( a=0; a<CONFIG_DESCRIPTOR_LENGTH; )
			{
				printf( " %02x", buf[a] );
				a ++;
				if( !(a%16) )
					puts( "\n" );
			}
			puts( "\n" );
		}
#endif

		code_transmit(buf, CONFIG_DESCRIPTOR_LENGTH);
	} else
	{
		puts( "stall ep0\n" );
		stall_ep0();
	}
}

void get_configuration(void)
{
	unsigned char c = bEPPflags.bits.configuration;

	single_transmit(&c, 1);
}

void set_configuration(void)
{
	if (ControlData.DeviceRequest.wValue == 0) {
		/* put device in unconfigured state */
		single_transmit(0, 0);
		DISABLE;
		bEPPflags.bits.configuration = 0;
		ENABLE;
		init_unconfig();
	} else if (ControlData.DeviceRequest.wValue == 1) {
		/* Configure device */
		single_transmit(0, 0);

		init_unconfig();
		init_config();

		DISABLE;
		bEPPflags.bits.configuration = 1;
		ENABLE;
	} else
		stall_ep0();
}

void get_interface(void)
{
	unsigned char txdat = 0;        /* Only/Current interface = 0 */
	single_transmit(&txdat, 1);
}

void set_interface(void)
{
	if (ControlData.DeviceRequest.wValue == 0 && ControlData.DeviceRequest.wIndex == 0)
		single_transmit(0, 0);
	else
		stall_ep0();
}

#endif
