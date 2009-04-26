#ifndef __MAINLOOP_H__
#define __MAINLOOP_H__


//*************************************************************************
// basic #defines
//*************************************************************************
#define MAX_ENDPOINTS      (u8)0x3

#define EP0_TX_FIFO_SIZE   0x40
#define EP0_RX_FIFO_SIZE   0x40
#define EP0_PACKET_SIZE    0x40

#define EP1_TX_FIFO_SIZE   0x0200
#define EP1_RX_FIFO_SIZE   0x0200
#define EP1_PACKET_SIZE    0x0200

#define EP2_TX_FIFO_SIZE   0x0200
#define EP2_RX_FIFO_SIZE   0x0200
#define EP2_PACKET_SIZE    0x0200

#define EndPoint0	0x00
#define EndPointA	0x01
#define EndPointB	0x02
#define EndPointC	0x03

#define USB_IDLE           0
#define USB_TRANSMIT       1
#define USB_RECEIVE        2

#define USB_CLASS_CODE_TEST_CLASS_DEVICE                    0xdc
#define USB_SUBCLASS_CODE_TEST_CLASS_D12                    0xA0
#define USB_PROTOCOL_CODE_TEST_CLASS_D12                    0xB0

//*************************************************************************
// masks
//*************************************************************************

#define USB_RECIPIENT            (u8)0x1F
#define USB_RECIPIENT_DEVICE     (u8)0x00
#define USB_RECIPIENT_INTERFACE  (u8)0x01
#define USB_RECIPIENT_ENDPOINT   (u8)0x02

#define USB_REQUEST_TYPE_MASK    (u8)0x60
#define USB_STANDARD_REQUEST     (u8)0x00
#define USB_CLASS_REQUEST        (u8)0x20
#define USB_VENDOR_REQUEST       (u8)0x40

#define USB_REQUEST_MASK         (u8)0x0F

#define DEVICE_ADDRESS_MASK      0x7F

//*************************************************************************
// macros
//*************************************************************************
// FIXME:: !! Is it right???? I don`t know about endian... by parkhw00.
#define SWAP(x)		((((x) & 0xFF) << 8) | (((x) >> 8) & 0xFF))
#define LSWAP(x)			\
	(				\
	 ((((x)>>0)&0xff)<<24) |	\
	 ((((x)>>8)&0xff)<<16) |	\
	 ((((x)>>16)&0xff)<<8) |	\
	 ((((x)>>24)&0xff)<<0) 		\
	)

#define MSB(x)    (((x) >> 8) & 0xFF)
#define LSB(x)    ((x) & 0xFF)

#define FALSE   0
#define TRUE    (!FALSE)

#ifdef outb
#undef outb
#undef outw
#undef outl
#endif
#ifdef inb
#undef inb
#undef inw
#undef inl
#endif
#define outb(adr, dat)	out_be16((unsigned short*)(adr),dat)
#define outw(adr, dat)	out_le16((unsigned short*)(adr),dat)
#define outl(adr, dat)	out_le32((unsigned *)(adr),dat)
#define inb(adr)	(unsigned char)in_be16((unsigned short*)(adr))
#define inw(adr)	(unsigned short)in_le16((unsigned short*)(adr))
#define inl(adr)	(unsigned long)in_le32((unsigned *)(adr))

//*************************************************************************
// basic typedefs
//*************************************************************************
typedef unsigned char   UCHAR;
typedef unsigned short  USHORT;
typedef unsigned long   ULONG;
typedef unsigned char   BOOL;

//*************************************************************************
// structure and union definitions
//*************************************************************************
typedef union _epp_flags
{
	struct _flags
	{
		u8 timer               	: 1;
		u8 bus_reset           	: 1;
		u8 suspend             	: 1;
		u8 setup_packet   	: 1;
		u8 remote_wakeup   	: 1;
		u8 in_isr	      	: 1;
		u8 control_state	: 2;

		u8 configuration	: 1;
		u8 verbose		: 1;
		u8 full_speed		: 1;
		u8 setup_dma		: 1;
		u8 dma_state    	: 2;
		u8 dma_disable		: 1; // V2.1
	} bits;
	u16 value;
} EPPFLAGS;

typedef struct _device_request
{
	u8 bmRequestType;
	u8 bRequest;
	u16 wValue;
	u16 wIndex;
	u16 wLength;
} DEVICE_REQUEST;

#pragma pack(1)
struct _IO_REQUEST {
	u16	uAddressL;
	u8	bAddressH;
	u16	uSize;
	u8	bCommand;
};
#pragma pack()

//#define MAX_CONTROLDATA_SIZE	8
#define MAX_CONTROLDATA_SIZE	512

typedef struct _control_xfer
{
	DEVICE_REQUEST DeviceRequest;
	u16 wLength;
	u16 wCount;
	u8 * pData;
	u8 dataBuffer[MAX_CONTROLDATA_SIZE];
} CONTROL_XFER;

//*************************************************************************
// USB utility functions
//*************************************************************************
extern int main_loop_function( void );
extern void usb_isr( void* );

extern void net2270_interrupt( void );

extern void suspend_change(void);
extern void disconnect_USB(void);
extern void stall_ep0(void);
extern void code_transmit(u8 * pRomData, u16 len);

extern void control_handler(void);
extern void check_key_LED(void);
extern void setup_dma(void);

extern void Ep0_Handler(void);
extern void EpA_Handler(void);
extern void EpB_Handler(void);
extern void EpC_Handler(void);
extern void read_main_data(u8 ep);
extern void Setup_Packet_Handler (void);
extern void Configuration_Handler (void);

#define IN_TOKEN_DMA 	1
#define OUT_TOKEN_DMA 	0

#define DMA_IDLE	0
#define DMA_RUNNING	1
#define DMA_PENDING	2

#define SETUP_DMA_REQUEST 	0x0471
#define GET_FIRMWARE_VERSION    0x0472
#define GET_SET_TWAIN_REQUEST   0x0473

typedef struct _TWAIN_FILEINFO {
	u8	bPage;    // bPage bit 7 - 5 map to uSize bit 18 - 16
	u8	uSizeH;    // uSize bit 15 - 8
	u8	uSizeL;    // uSize bit 7 - 0
} TWAIN_FILEINFO, *PTWAIN_FILEINFO;

#endif

