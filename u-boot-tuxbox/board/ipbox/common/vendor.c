#include <common.h>

#if defined (CONFIG_PDIUSB12) || defined (CONFIG_NET2270)

#ifdef CONFIG_DGS_FRONT
#include "common/front.h"
#endif

#include "common/upgrade.h"
#include "vendor.h"
#include "usb100.h"
#ifdef CONFIG_PDIUSB12
#include "pdiusbd12/d12ci.h"
#include "pdiusbd12/mainloop.h"
#endif

#ifdef CONFIG_NET2270
#include "net2270/net2270.h"
#include "net2270/mainloop.h"
#endif

#define VENDOR_DESC_LEN (sizeof(struct _vendor_header)+sizeof(struct _vendor_desc))

extern CONTROL_XFER ControlData;
extern EPPFLAGS bEPPflags;

int usb_current_block_size = 0;
int usb_data_buffer_index;

/*
 * file_header and iorequest will be stored by PC.
 * PC will send me about file size and filename, block size...
 */
struct _FileDownHeader file_header;
struct _IO_REQUEST iorequest;

/*
 * currently not using...
 */
struct _vendor_header vh =
{
	SWAP(1),
	SWAP(VENDOR_DESC_LEN)
};

struct _vendor_desc vd =
{
	LSWAP(1),
	LSWAP(2),
	LSWAP(3)
};

void get_vendor_descript( void )
{
	unsigned char *buf;

	buf = upg_buffer;

	memcpy( &buf[0], &vh, sizeof(struct _vendor_header) );
	memcpy( &buf[sizeof(struct _vendor_header)], &vd, sizeof(struct _vendor_desc) );

	code_transmit( buf, VENDOR_DESC_LEN );
}

void vendor_req( void )
{
	u16 wIndex;
	u16 wValue;
	u16 wLength;
	u8 buf[2];
#ifdef CONFIG_NET2270
//	DISABLE;
#endif

	wIndex = ControlData.DeviceRequest.wIndex;
	wValue = ControlData.DeviceRequest.wValue;
	wLength = ControlData.DeviceRequest.wLength;

//	puts( "vendor_req." );

	if( ControlData.DeviceRequest.bmRequestType & USB_ENDPOINT_DIRECTION_MASK )
	{
		switch( wIndex )
		{
			case GET_FIRMWARE_VERSION:
				puts( "Connected.\n" );
				puts( "Select file in FlashTool and download.\n" );
#ifdef CONFIG_NET2270
				send_fifo(EndPoint0, "\x01", 1 );

				/* why??? */
				if( usb_current_block_size > 0 ) {
					//udelay(10);
					read_main_data(EndPointB);
				}
#endif

#ifdef CONFIG_PDIUSB12
				single_transmit( "x01", 1);
#endif

				break;
			case GET_FLASH_STATUS:
				/*
				 * FIXME!!!
				 * NOT COMPLETED. MAKE ME WORK!!!
				 */
				if(bEPPflags.bits.verbose)
				{
					puts( " get flash status.\n" );
				}

				buf[1] = 0;
				switch( upg_buffer_status )
				{
					case UPGBUF_UNUSED:
						buf[0] = 0x00;
						break;
					default:
						buf[0] = 0x02;
						break;
				}
#ifdef CONFIG_NET2270
				send_fifo(EndPoint0, buf, 2 );
#endif
#ifdef CONFIG_PDIUSB12
				D12_WriteEndpoint(1, buf, 2);
#endif
				break;
			default:
				printf( " unknown index. (0x%04x)\n", wIndex );
#ifdef CONFIG_PDIUSB12
				single_transmit(0, 0);
#endif
				break;
		}
	}
	else
	{
		switch( wIndex )
		{
			case SET_FILE_DOWN_HEADER:
				if(bEPPflags.bits.verbose)
				{
					puts( " set file down header.\n" );
				}
				if( set_file_down_header( (struct _FileDownHeader*)ControlData.dataBuffer, wLength ) )
				{
					puts( "download is not accepted\n" );
				}
#ifdef CONFIG_PDIUSB12
				single_transmit(0, 0);
#endif
				break;
			case SET_FILE_DOWN:
				if(bEPPflags.bits.verbose)
				{
					puts( " set file down.\n" );
				}
				set_file_down( (struct _IO_REQUEST*)ControlData.dataBuffer, wLength );
#ifdef CONFIG_PDIUSB12
				single_transmit(0, 0);
#endif
				break;
			case USB_CMD_END:
				if(bEPPflags.bits.verbose)
				{
					puts( " cmd end\n" );
				}
				download_completed();
#ifdef CONFIG_PDIUSB12
				single_transmit(0, 0);
#endif
				break;
			default:
				printf( " unknown index.(0x%04x)\n", wIndex );
#ifdef CONFIG_PDIUSB12
				single_transmit(0, 0);
#endif
				break;
		}
	}

#ifdef CONFIG_NET2270
//	ENABLE;
#endif
}

int set_file_down_header( struct _FileDownHeader *header, int len )
{
	if( len != sizeof(struct _FileDownHeader) )
	{
		printf( "header size is differ. needed %d, but got %d\n", sizeof(struct _FileDownHeader), len );
		return -1;
	}

	memcpy( &file_header, header, len );
	if(bEPPflags.bits.verbose)
	{
		printf( "file_name    %s\n", file_header.file_name );
		printf( "file_size    %d\n", (int)file_header.file_size );	// It`s swaped...
		printf( "down_type    %d\n", (int)file_header.down_type );
		printf( "down_media   %d\n", (int)file_header.down_media );
		printf( "down_command %d\n", (int)file_header.down_command );
	}

	if( file_header.file_size > CFG_DGS_UPGRADE_BUFSIZE )
	{
		puts( "file is too big...\n" );
		return -1;
	}

	if( upg_buffer_status != UPGBUF_UNUSED )
	{
		printf( "buffer is using by other.(status:%d)\n", upg_buffer_status );
		puts( "go any way...\n" );
//		return -1;
	}

	upg_buffer_status = UPGBUF_DOWNLOAD;
	upg_buffer_len = 0;

	return 0;
}

void set_file_down( struct _IO_REQUEST *request, int len )
{
	if( len != sizeof(struct _IO_REQUEST) )
	{
		printf( "iorequest size is differ. needed %d, but got %d\n", sizeof(struct _IO_REQUEST), len );
		return;
	}

	memcpy( &iorequest, request, len );
	iorequest.uSize = SWAP( iorequest.uSize );
	if(bEPPflags.bits.verbose)
	{
		printf( "uAddressL 0x%04x\n", iorequest.uAddressL );
		printf( "bAddressH 0x%02x\n", iorequest.bAddressH );
		printf( "uSize     0x%04x\n", iorequest.uSize );
		printf( "bCommand  0x%02x\n", iorequest.bCommand );
	}

	/*
	 * set block size.
	 * will be used in main_rxdone.
	 */
	if( usb_current_block_size )
 		printf( "do we lost some data at last transfer\?\?\?(bs:%d)\n",
				usb_current_block_size );
	usb_current_block_size = iorequest.uSize;
#ifdef CONFIG_NET2270
//	if(bEPPflags.bits.full_speed) {
//		udelay(1);
//	}

	read_main_data(EndPointB);
#endif
}

int download_completed( void )
{
	if( upg_buffer_status == UPGBUF_DOWNLOAD )
	{
		if( usb_current_block_size )
			printf( "do we lost some data at last transfer\?\?\?(bs:%d)\n",
					usb_current_block_size );
		upg_buffer_status = UPGBUF_WRITE;
		usb_current_block_size = 0;

		if( upg_buffer_len != file_header.file_size )
		{
			printf( "vendor.c: %d lost data?(%d!=%d)\n", __LINE__,
					upg_buffer_len, (int)file_header.file_size );
			goto error;
		}
	}
	else
	{
		puts( "download completed with unknown stat.\n" );
	}

	return 0;

error:
	upg_buffer_status = UPGBUF_UNUSED;

	return -1;

}

#endif
