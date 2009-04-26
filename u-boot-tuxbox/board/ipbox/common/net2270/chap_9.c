#include <common.h>
#include <asm/io.h>

#ifdef CONFIG_NET2270

#include "net2270.h"
#include "mainloop.h"
#include "common/usb100.h"
#include "common/chap_9.h"
#include "common/vendor.h"
#include "common/upgrade.h"

#define NUM_ENDPOINTS	3

#define CONFIG_DESCRIPTOR_LENGTH    sizeof(USB_CONFIGURATION_DESCRIPTOR) \
				+ sizeof(USB_INTERFACE_DESCRIPTOR) \
				+ (NUM_ENDPOINTS * sizeof(USB_ENDPOINT_DESCRIPTOR))

extern CONTROL_XFER ControlData;
extern EPPFLAGS bEPPflags;

USB_DEVICE_DESCRIPTOR DeviceDescr =
{
	sizeof(USB_DEVICE_DESCRIPTOR),		// bLength
	USB_DEVICE_DESCRIPTOR_TYPE,		// bDesriptionType
	SWAP(0x0200),				//bcdUSB
	//USB_CLASS_CODE_TEST_CLASS_DEVICE,
	0xFF,					// bDeviceClass
	0x00, 					// bDeviceSubClass
	0x00,					// bDeiveProtocol
	EP0_PACKET_SIZE,			// bMaxPacketSize0
	SWAP(0x0525),		// Netchip Vendor ID
	SWAP(0x1000),		// idProduct
	SWAP(0x0100),		// bcdDevice
	0x00,			// iManufacturer
       	0x00,			// iProduct
       	0x00,			// iSerialNumber
	0x01			// bNumConfigurations
};

USB_CONFIGURATION_DESCRIPTOR ConfigDescr =
{
	sizeof(USB_CONFIGURATION_DESCRIPTOR),
	USB_CONFIGURATION_DESCRIPTOR_TYPE,
	SWAP(CONFIG_DESCRIPTOR_LENGTH),
	1,
	1,
	0,
	0xe0,
	0x0
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

USB_ENDPOINT_DESCRIPTOR EP_A_Descr =
{
	sizeof(USB_ENDPOINT_DESCRIPTOR),
	USB_ENDPOINT_DESCRIPTOR_TYPE,
	0x81,
	USB_ENDPOINT_TYPE_BULK,
	SWAP(EP1_PACKET_SIZE),
	0
};

USB_ENDPOINT_DESCRIPTOR EP_B_Descr =
{
	sizeof(USB_ENDPOINT_DESCRIPTOR),
	USB_ENDPOINT_DESCRIPTOR_TYPE,
	0x02,
	USB_ENDPOINT_TYPE_BULK,
	SWAP(EP2_PACKET_SIZE),
	0
};

USB_ENDPOINT_DESCRIPTOR EP_C_Descr =
{
	sizeof(USB_ENDPOINT_DESCRIPTOR),
	USB_ENDPOINT_DESCRIPTOR_TYPE,
	0x03,
	USB_ENDPOINT_TYPE_BULK,
	SWAP(EP0_PACKET_SIZE),
	0
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
        u8 endp, txdat[2];
        u8 bRecipient = ControlData.DeviceRequest.bmRequestType & USB_RECIPIENT;
        //u8 c;

        if (bRecipient == USB_RECIPIENT_DEVICE) {
                if(bEPPflags.bits.remote_wakeup == 1)
                        txdat[0] = 3;
                else
                        txdat[0] = 1;
                txdat[1] = 0;
                send_fifo(EndPoint0, txdat, 2);
        } else if (bRecipient == USB_RECIPIENT_INTERFACE) {
                txdat[0]=0;
                txdat[1]=0;
                send_fifo(EndPoint0, txdat, 2);
        } else if (bRecipient == USB_RECIPIENT_ENDPOINT) {
                endp = (u8)(ControlData.DeviceRequest.wIndex & MAX_ENDPOINTS);
                txdat[0] = 1;
                txdat[1] = 0;
                send_fifo(EndPoint0, txdat, 2);
        } else
                stall_ep0();
}

void clear_feature(void)
{
	u8 endp;
	u8 bRecipient = ControlData.DeviceRequest.bmRequestType & USB_RECIPIENT;

	if (bRecipient == USB_RECIPIENT_DEVICE
		&& ControlData.DeviceRequest.wValue == USB_FEATURE_REMOTE_WAKEUP) {
		DISABLE;
		bEPPflags.bits.remote_wakeup = 0;
		ENABLE;
	}
	else if (bRecipient == USB_RECIPIENT_ENDPOINT
		&& ControlData.DeviceRequest.wValue == USB_FEATURE_ENDPOINT_STALL) {
		endp = (u8)(ControlData.DeviceRequest.wIndex & MAX_ENDPOINTS);
		//clear_halt

	        /* ep0 and bulk/intr endpoints */
		*PAGESEL = endp;
		*EP_RSPCLR = (1 << ENDPOINT_HALT) |
	        	     (1 << ENDPOINT_TOGGLE) |
	        	     (1 << ALT_NAK_OUT_PACKETS) |
			     (1 << CONTROL_STATUS_PHASE_HANDSHAKE) |
			     (1 << ALT_NAK_OUT_PACKETS) |
			     (1 << NAK_OUT_PACKETS_MODE);
	} else
		stall_ep0();
}

void set_feature(void)
{
	u8 endp;
	u8 bRecipient = ControlData.DeviceRequest.bmRequestType & USB_RECIPIENT;

	if (bRecipient == USB_RECIPIENT_DEVICE
		&& ControlData.DeviceRequest.wValue == USB_FEATURE_REMOTE_WAKEUP) {
		DISABLE;
		bEPPflags.bits.remote_wakeup = 1;
		ENABLE;
	}
	else if (bRecipient == USB_RECIPIENT_ENDPOINT
		&& ControlData.DeviceRequest.wValue == USB_FEATURE_ENDPOINT_STALL) {
		endp = (u8)(ControlData.DeviceRequest.wIndex & MAX_ENDPOINTS);
		*PAGESEL = endp;
		*EP_RSPSET = (1 << CONTROL_STATUS_PHASE_HANDSHAKE)
	         	   | (1 << ALT_NAK_OUT_PACKETS)
	             	   | (1 << NAK_OUT_PACKETS_MODE);
		//set_halt
	} else
		stall_ep0();
}

void set_address(void)
{
	if(bEPPflags.bits.verbose) {
		printf( "set address %d\n", ControlData.DeviceRequest.wValue & DEVICE_ADDRESS_MASK );
	}

	if(bEPPflags.bits.in_isr == 0)
		DISABLE;

	*REGADDRPTR = OURADDR;
	*REGDATA = (ControlData.DeviceRequest.wValue & DEVICE_ADDRESS_MASK);

	if(bEPPflags.bits.in_isr == 0)
		ENABLE;
}

void get_descriptor(void)
{
	u8 bDescriptor = MSB(ControlData.DeviceRequest.wValue);

	if (bDescriptor == USB_DEVICE_DESCRIPTOR_TYPE) {
		code_transmit((u8 *)&DeviceDescr, sizeof(USB_DEVICE_DESCRIPTOR));
	} else if (bDescriptor == USB_CONFIGURATION_DESCRIPTOR_TYPE) {

		u8 reg = *USBCTL1;
		if (reg & (1 << USB_HIGH_SPEED)) {
			if(bEPPflags.bits.verbose) {
				puts ("Connected for High Speed\n");
			}
			EP_A_Descr.wMaxPacketSize = SWAP(0x0200);
			EP_B_Descr.wMaxPacketSize = SWAP(0x0200);
			bEPPflags.bits.full_speed = 0;
		} else {
			if(bEPPflags.bits.verbose) {
				puts ("Connected for Full Speed\n");
			}
			EP_A_Descr.wMaxPacketSize = SWAP(0x0040);
			EP_B_Descr.wMaxPacketSize = SWAP(0x0040);
			bEPPflags.bits.full_speed = 1;
		}

		u8 *buf = upg_buffer;	// we use usb data buffer for temp.
					// upg_buffer dont have any valid data yet.

		memcpy( &buf[0],
				&ConfigDescr,
				sizeof(USB_CONFIGURATION_DESCRIPTOR) );
		memcpy( &buf[sizeof(USB_CONFIGURATION_DESCRIPTOR)],
				&InterfaceDescr,
				sizeof(USB_INTERFACE_DESCRIPTOR) );
		memcpy( &buf[sizeof(USB_CONFIGURATION_DESCRIPTOR)+
			     sizeof(USB_INTERFACE_DESCRIPTOR)+
			     0*sizeof(USB_ENDPOINT_DESCRIPTOR)],
				&EP_A_Descr,
				sizeof(USB_ENDPOINT_DESCRIPTOR) );
		memcpy( &buf[sizeof(USB_CONFIGURATION_DESCRIPTOR)+
			     sizeof(USB_INTERFACE_DESCRIPTOR)+
			     1*sizeof(USB_ENDPOINT_DESCRIPTOR)],
				&EP_B_Descr,
				sizeof(USB_ENDPOINT_DESCRIPTOR) );
		memcpy( &buf[sizeof(USB_CONFIGURATION_DESCRIPTOR)+
			     sizeof(USB_INTERFACE_DESCRIPTOR)+
			     2*sizeof(USB_ENDPOINT_DESCRIPTOR)],
				&EP_C_Descr,
				sizeof(USB_ENDPOINT_DESCRIPTOR) );


		code_transmit(buf, CONFIG_DESCRIPTOR_LENGTH);
	} else
	{
		puts( "stall ep0\n" );
		stall_ep0();
	}
}

void get_configuration(void)
{
	u8 c = bEPPflags.bits.configuration;
	send_fifo(EndPoint0,&c, 1);
}

void set_configuration(void)
{
	if (ControlData.DeviceRequest.wValue == 0) {
		/* put device in unconfigured state */
		DISABLE;
		bEPPflags.bits.configuration = 0;
		ENABLE;
	} else if (ControlData.DeviceRequest.wValue == 1) {
		/* Configure device */

		Configuration_Handler();

		DISABLE;
		bEPPflags.bits.configuration = 1;
		ENABLE;
	} else
		stall_ep0();
}

void get_interface(void)
{
	u8 txdat = 0;        /* Only/Current interface = 0 */
	send_fifo(EndPoint0,&txdat, 1);
}

void set_interface(void)
{
	if (ControlData.DeviceRequest.wValue == 0 && ControlData.DeviceRequest.wIndex == 0) {
	} else {
		stall_ep0();
	}
}

void Configuration_Handler (void)
{
	u16 byte;

	byte = EP_A_Descr.wMaxPacketSize;

	*PAGESEL = EndPointA;

	*REGADDRPTR = EP_CFG;
	*REGDATA = (1 << ENDPOINT_ENABLE) |
		   (BULK << ENDPOINT_TYPE) |
		   (1 << ENDPOINT_DIR) |
		   (1 << ENDPOINT_NUM);

	*REGADDRPTR = IRQENB0;
	*REGDATA = (1 << ENDPOINT_0_INTERRUPT_ENABLE) |
//		   (1 << ENDPOINT_A_INTERRUPT_ENABLE) |
		   (1 << ENDPOINT_B_INTERRUPT_ENABLE) |
		   (1 << ENDPOINT_C_INTERRUPT_ENABLE) |
		   (1 << SETUP_PACKET_INTERRUPT_ENABLE);

	*REGADDRPTR = EP_MAXPKT0;
	*REGDATA = byte >> 8;
	*REGADDRPTR = EP_MAXPKT1;
	*REGDATA = byte & 0x00ff;
	*EP_STAT1 = (1 << BUFFER_FLUSH);

	*PAGESEL = EndPointB;

	*REGADDRPTR = EP_CFG;
	*REGDATA = (1 << ENDPOINT_ENABLE) |
		   (BULK << ENDPOINT_TYPE) |
		   (0 << ENDPOINT_DIR) |
		   (2 << ENDPOINT_NUM);
	*EP_STAT1 = (1 << BUFFER_FLUSH);

	*REGADDRPTR = EP_MAXPKT0;
	*REGDATA = byte >> 8;
	*REGADDRPTR = EP_MAXPKT1;
	*REGDATA = byte & 0x00ff;

	*PAGESEL = EndPointC;

	*REGADDRPTR = EP_CFG;
	*REGDATA = (1 << ENDPOINT_ENABLE) |
		   (BULK << ENDPOINT_TYPE) |
		   (0 << ENDPOINT_DIR) |
		   (3 << ENDPOINT_NUM);
	*EP_STAT1 = (1 << BUFFER_FLUSH);

	*REGADDRPTR = EP_MAXPKT1;
	*REGDATA = 0x00;
	*REGADDRPTR = EP_MAXPKT0;
	*REGDATA = 0x40;

	*PAGESEL = EndPointA;
	*EP_IRQENB = (1 << DATA_IN_TOKEN_INTERRUPT_ENABLE);

	*PAGESEL = EndPointB;
	*EP_IRQENB = 0x00;
	*EP_STAT0 = (1 << NAK_OUT_PACKETS) |
		    (1 << SHORT_PACKET_TRANSFERRED_INTERRUPT) |
		    (1 << DATA_PACKET_RECEIVED_INTERRUPT) |
		    (1 << DATA_PACKET_TRANSMITTED_INTERRUPT) |
		    (1 << DATA_OUT_TOKEN_INTERRUPT) |
		    (1 << DATA_IN_TOKEN_INTERRUPT);

	*PAGESEL = EndPointC;
	*EP_IRQENB = 0x00;
	*EP_STAT0 = (1 << NAK_OUT_PACKETS) |
		    (1 << SHORT_PACKET_TRANSFERRED_INTERRUPT) |
		    (1 << DATA_PACKET_RECEIVED_INTERRUPT) |
		    (1 << DATA_PACKET_TRANSMITTED_INTERRUPT) |
		    (1 << DATA_OUT_TOKEN_INTERRUPT) |
		    (1 << DATA_IN_TOKEN_INTERRUPT);


	return;
}
#endif
