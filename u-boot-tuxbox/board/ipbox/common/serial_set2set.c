#include <common.h>

#ifdef CONFIG_SERIAL_SET2SET
#ifndef CONFIG_DGS_FRONT
#error "CONFIG_DGS_FRONT is needed with CONFIG_SERIAL_UPGRADE."
#endif

#include <command.h>
#include "common/front.h"
#include "common/upgrade.h"
#include "common/flash_img_info.h"

#define _CMD_PC_BR		0x22
#define _CMD_PC_READY		0x05
#define _RCV_READY		0x05
#define _CMD_WHICH_FUNCTION	0xd0
#define _CMD_START_UPLOAD	0xd1
#define _RCV_REQ_HEADER		0x70

#define TIMEOUT			500

#pragma pack(1)
struct DataHeader
{
	char valid;
	char model_name[30];
	char p_or_d;
	char da;
	char f_or_e;
	unsigned int size;
	char dummy;
};
#pragma pack()

extern flash_img_info_t dgs_flash_imgs[];

/* in serial_upgrade.c */
extern int dg_getc( int to_ms );
extern void gen_crc_table(void);
extern unsigned long getCRC32(unsigned char *buff, int size);

static int send_header( unsigned int size )
{
	unsigned char buf[64];
	struct DataHeader header;
	int key;
	int a;

	header.size = size;
	strcpy( header.model_name, CFG_SERIALDOWN_HEADER );

	memset( buf, 0, 64 );
	memcpy( buf, &header, sizeof(header) );

	key = dg_getc( TIMEOUT );
	if( key != 0x70 )
		return -1;

	for( a=0; a<64; a++ )
		raw_serial_putc( buf[a] );

	return 0;
}

static int send_body( int block, int size, unsigned char *base )
{
	int a;
	unsigned long crc;

	block ++;

	if( dg_getc( TIMEOUT ) != 0x69 )
		return -1;
	if( dg_getc( TIMEOUT ) != block%10 )
		return -1;

	crc = getCRC32( base, 1024 );

	raw_serial_putc( 0x69 );
	if( (size+1023)/1024 == block )
		raw_serial_putc( 0xee );
	else
		raw_serial_putc( block % 10 );

	for( a=0; a<1024; a++ )
		raw_serial_putc( base[a] );

	raw_serial_putc( crc >> 24 & 0xff );
	raw_serial_putc( crc >> 16 & 0xff );
	raw_serial_putc( crc >>  8 & 0xff );
	raw_serial_putc( crc >>  0 & 0xff );

	return 0;
}

static int prepare_bodyheader( unsigned long start, unsigned long size, const char *name )
{
	struct _image_header header;

	header.magic = IMG_MAGIC;
	header.structure_size = sizeof(header);
	header.vendor_id = MY_VENDOR_ID;
	header.product_id = MY_PRODUCT_ID;
	header.hw_model = MY_HW_MODEL;
	header.hw_version = MY_HW_VERSION;
	strncpy( header.name, name, H_NAME_SIZE-1 );
	header.name[H_NAME_SIZE-1] = 0;
	header.start_addr = start;
	header.erase_size = size;
	header.data_offset = HEADER_SIZE;
	header.data_size = size;
	header.data_crc = crc32( 0xffffffff, (unsigned char*)start, size );;

	memset( upg_buffer, 0, HEADER_SIZE );
	memcpy( upg_buffer, &header, sizeof(header) );
	*(unsigned long*)upg_buffer = crc32( 0xffffffff, (unsigned char*)upg_buffer+4, HEADER_SIZE-4 );

	return 0;
}

int serial_set2set( int argc, char **argv )
{
	DECLARE_GLOBAL_DATA_PTR;

	int a;
	int key;
	int ret;
	int imgnum;
	int imgmax;
	char buf[16];
	unsigned long start, size;

#ifdef CONFIG_DGS_FRONT_7SEG
	front_puts( "S2S" );
#else
	front_puts( "set 2 set" );
#endif

	gen_crc_table();

	upg_buffer = (unsigned char*)gd->dgs_upg_buffer;

	{
		int keys[2];

		keys[0] = key_release;
		keys[1] = key_front_release;
		keys[2] = key_null;

		front_waitkey( keys, 5 );
	}

	while( 1 )
	{
		/* select flash partition */
		puts( "\npower to reboot.\n" );
		for( a=0; dgs_flash_imgs[a].name; a++ )
		{
			printf( "%2d. %08x - %06x \"%s\"\n", a+1,
					dgs_flash_imgs[a].start,
					dgs_flash_imgs[a].size,
					dgs_flash_imgs[a].name );
		}
		imgmax = a-1;

		imgnum = 0;
		puts( "image :   " );
		while( 1 )
		{
			if( imgnum < 0 )
				imgnum = 0;
			if( imgnum > imgmax )
				imgnum = imgmax;

			sprintf( buf, "%d", imgnum+1 );
			front_puts( buf );
			printf( "\b\b%2d", imgnum+1 );
			key = front_singlekey( 0 );
			if( key == key_ok || key == key_front_ok )
				break;
			if(
					key == key_down ||
					key == key_front_down ||
					key == key_left ||
					key == key_front_left
			  )
			{
				imgnum --;
				continue;
			}
			if(
					key == key_up ||
					key == key_front_up ||
					key == key_right ||
					key == key_front_right
			  )
			{
				imgnum ++;
				continue;
			}
			if( key_0 <= key && key <= key_9 )
			{
				imgnum = key-key_0-1;
				continue;
			}
			if( key == key_power || key == key_front_power )
				return 0;
		}

		start = dgs_flash_imgs[imgnum].start;
		size = dgs_flash_imgs[imgnum].size;
		prepare_bodyheader( start, size, dgs_flash_imgs[imgnum].name );
		printf( "\n\"%s\" ready.\n", dgs_flash_imgs[imgnum].name );
		front_puts( "redy" );

		/* let`s start */
		gd->baudrate = 9600;
		serial_setbrg();

		while( dg_getc(0) != _CMD_PC_BR );
		puts( "start downloading...\n" );
		front_puts( "send" );
		raw_serial_putc( _CMD_PC_BR );

#if 1
		raw_serial_putc( 6 );
		gd->baudrate = 230400;
#else
		raw_serial_putc( 5 );
		gd->baudrate = 115200;
#endif
		udelay( 100000 );
		serial_setbrg();
		udelay( 100000 );

		for( a=0; a<4; a++ )
		{
			if( dg_getc( TIMEOUT ) == _CMD_PC_READY )
				break;;
		}
		if( a == 4 )
		{
			printf( "protocal error.%d\n", __LINE__ );
			front_puts( "failed" );
			continue;
		}
		raw_serial_putc( _CMD_PC_READY );

		if( dg_getc( TIMEOUT ) != _CMD_WHICH_FUNCTION )
		{
			printf( "protocal error.%d\n", __LINE__ );
			front_puts( "failed" );
			continue;
		}
		raw_serial_putc( 7 );
		if( dg_getc( TIMEOUT ) != 7 )
		{
			printf( "protocal error.%d\n", __LINE__ );
			front_puts( "failed" );
			continue;
		}

		size += HEADER_SIZE;

		/* send header */
		ret = send_header( size );
		if( ret < 0 )
		{
			printf( "protocal error.%d\n", __LINE__ );
			front_puts( "failed" );
			continue;
		}

		/* send body header */
		for( a=0; a<4; a++ )
		{
			send_body( a, size, (unsigned char *)(upg_buffer+1024*a) );
			if( ret < 0 )
				break;
		}
		if( a != 4 )
		{
			printf( "protocal error.%d\n", __LINE__ );
			front_puts( "failed" );
			continue;
		}

		/* send body */
		for( a=4; a<((size+1023)/1024); a++ )
		{
			send_body( a, size, (unsigned char *)(start+1024*(a-4)) );
			if( ret < 0 )
				break;
		}
		if( a != (size+1023)/1024 )
		{
			printf( "protocal error.%d\n", __LINE__ );
			front_puts( "failed" );
			continue;
		}

		puts( "successed.\n" );
	}
}

int do_serial_set2set( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] )
{
	return serial_set2set( argc, argv );
}

U_BOOT_CMD(
		serial_set2set, 14, 0, do_serial_set2set,
		"serial_set2set- send flash image to another.\n",
		"start set2set through serial port.\n" );

#endif

