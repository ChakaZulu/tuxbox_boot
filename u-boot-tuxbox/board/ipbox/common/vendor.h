#ifndef _VENDOR_H
#define _VENDOR_H

#ifdef CONFIG_NET2270
#include "net2270/mainloop.h"
#endif

#ifdef CONFIG_PDIUSB12
#include "pdiusbd12/mainloop.h"
#endif

#define USB_BS_NOT_USE		0		/* buffer is not using...
						   any one can use usb_data_buffer */
#define USB_BS_DOWN		1		/* buffer is using for download */
#define USB_BS_WRITE		2		/* buffer is using for writeing flash */

/*
 * status of bootloader for usb client.
 * will be used for announce status to user(PC application).
 */
#define USB_ST_DO_NOTHING	0
#define USB_ST_DOWNLOAD		1
#define USB_ST_ERASE		2
#define USB_ST_WRITE		3
#define USB_ST_ERROR		4

/*
 * index of vendor request.
 * used in vendor_req function
 */
#define GET_FIRMWARE_VERSION	0x0472
#define GET_FLASH_STATUS	0x0473
#define SET_FILE_DOWN_HEADER	3
#define SET_FILE_DOWN		4
#define USB_CMD_END		7

struct _vendor_header
{
	unsigned short request;                   
	unsigned short len;
};                                                         

struct _vendor_desc                                                       
{                                     
	unsigned long product;

	unsigned long sw_model;
	unsigned long sw_version;
};

#pragma pack(1)
struct _FileDownHeader{
	unsigned char file_name[256];
	unsigned long file_size;
//	unsigned long file_time;
//	unsigned long file_date;
	unsigned char down_type;
	unsigned char down_media;
	unsigned char down_command;
};
#pragma pack()

extern void get_vendor_descript( void );
extern void vendor_req( void );
extern int set_file_down_header( struct _FileDownHeader*, int );
extern void set_file_down( struct _IO_REQUEST*, int );
extern int download_completed( void );
extern int burn_flash( void );

extern int usb_current_block_size;
extern struct _FileDownHeader file_header;
extern struct _IO_REQUEST iorequest;
#endif
