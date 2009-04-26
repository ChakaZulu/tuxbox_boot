#include <common.h>

#ifdef CONFIG_SERIAL_UPGRADE
#ifndef CONFIG_DGS_FRONT
#error "CONFIG_DGS_FRONT is needed with CONFIG_SERIAL_UPGRADE."
#endif

#include <command.h>
#include <malloc.h>
#include "common/front.h"

#include "common/upgrade.h"

#define dprintf(a,b...) printf(a,##b)
unsigned char gzipped =0;

//--------------------------------------//
//          Command Define              //
//--------------------------------------//
#define   _CMD_PC_BR             0x22
#define   _CMD_PC_READY          0x05
#define   _RCV_READY             0x05
#define   _CMD_WHICH_FUNCTION    0xd0
#define   _CMD_START_UPLOAD      0xd1
#define   _RCV_REQ_HEADER        0x70

#define TIMEOUT 500
#define RETRY 2

#define CRC32 0x04c11db7

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

struct _serial_image_header
{
	unsigned long header_crc32;
	unsigned long image_size;
};
#pragma pack()

/*
 * variables for dgstation serial download...
 */
#if 0
unsigned char Flash_is_Writed_Sequence[64] = {
	10, 63, 18, 53,  2, 44, 19, 34,
	 0, 12, 20, 26, 45, 37, 46,  8,
	38, 14, 27,  7, 57, 33, 11, 17,
	35, 16, 29,  4,  5,  9, 21, 56,
	58, 55, 25, 48, 49, 50, 43, 13,
	 3, 36, 39,  6, 23, 51, 61, 59,
	15, 41, 62, 24, 28, 22, 30, 42,
	 1, 40, 47, 52, 31, 60, 32, 54,
};

unsigned char Scramble_Number_Flash_is_Writed[64] = {
	0xc9, 0x9c, 0x2a, 0x41, 0x43, 0x6d, 0xc0, 0xe2,
	0xf6, 0x8c, 0x11, 0xf9, 0x73, 0xca, 0x35, 0x38,
	0xe0, 0x1c, 0x72, 0x4c, 0xb9, 0xd7, 0x61, 0x61,
	0xee, 0x2f, 0x14, 0x41, 0x44, 0x9a, 0xe1, 0xff,
	0x2b, 0xcf, 0xff, 0x00, 0xf0, 0xbe, 0xe4, 0xf9,
	0x6a, 0x68, 0x6d, 0xbd, 0x2b, 0xde, 0x67, 0xda,
	0xcd, 0xe0, 0x1f, 0x57, 0xaf, 0xd3, 0x40, 0x9b,
	0xea, 0x9e, 0xda, 0x1e, 0x7d, 0x3c, 0xd3, 0xa0,
};
#endif
unsigned int *dg_crc_table;

/*
 * functions for dgstation serial download...
 */
void gen_crc_table(void)
{
	int i, j;
	unsigned long acc, data;

	dg_crc_table = malloc( 256*sizeof(unsigned int) );

	for( i=0 ; i<256 ; i++ )
	{
		data = ( (unsigned long) i << 24 );
		acc = 0;
		for( j=0 ; j<8 ; j++ )
		{
			if( (data ^ acc) & 0x80000000L )
				acc = ( acc << 1 ) ^ CRC32;
			else
				acc = ( acc << 1 );
			data <<= 1;
		}
		dg_crc_table[i] = acc;
	}
	return;
}

void crc_update(unsigned int data, unsigned long *acc)
{
	int i;

	i = ((int)( *acc >> 24) ^ data ) & 0xff;
	*acc = ( *acc << 8 ) ^ dg_crc_table[i];
}

unsigned long getCRC32(unsigned char *buff, int size)
{
	unsigned long acc=0x0L;
	int i;

	for( i=0 ; i<size ; i++ )
	{
		crc_update(*(buff+i), &acc);
	}
	return acc;
}

void clean_crc_table( void )
{
	free( dg_crc_table );
}

int dg_getc( int to_ms )
{
	unsigned long long stime = get_ticks();

	do
	{
		if( serial_tstc() )
			return serial_getc();
	}while( get_ticks() < stime+(get_tbclk()/1000*to_ms) );

	return -1;
}

int receive_header( void )
{
	int a, b;
	int key;
	unsigned char buffer[64];
//	struct _serial_image_header header;
	struct DataHeader header;

	/*
	 * request header...
	 * and get header...
	 */
	serial_putc( 0x70 );
	for( a=0; a<64; a++ )
	{
		for( b=0; b<10; b++ )
		{
			key = dg_getc( TIMEOUT );
			if( key != -1 )
				break;
		}
		if( b == 10 )
		{
			dprintf( "header timeout.(got %d)\n", a );
			return -1;
		}

		buffer[a] = key;
	}

	memcpy( &header, buffer, sizeof(header) );
	header.size = ntohl( header.size );

	/*
	 * compare model name.
	 */

	if( (strcmp( header.model_name, CFG_SERIALDOWN_HEADER )) && (strcmp( header.model_name, CFG_SERIALDOWN_HEADER"_gz" )))
	{
		printf( "image is not mine.(got\"%s\",expected\"%s\" or \"%s\")\n",
				header.model_name, CFG_SERIALDOWN_HEADER,CFG_SERIALDOWN_HEADER"_gz" );
		return -1;
	}

	if (!strcmp( header.model_name, CFG_SERIALDOWN_HEADER"_gz" ))
	{
		puts("Gzipped image\n");
		gzipped=1;

	}	else   {
		puts("Normal image\n");
		gzipped =0;
	}

	return header.size;
}

int gunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp);

void ungzip_image(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	char gzip_buff[0x802000];
	unsigned long lenp;

	upg_buffer = (unsigned char*)gd->dgs_upg_buffer;

	puts("Ungzipping image: ");
	gunzip (gzip_buff,0x801040,(unsigned char*)gd->dgs_upg_buffer,&lenp);
	printf ("%d, bytes.. done!\n",lenp);
	memcpy((unsigned char*)gd->dgs_upg_buffer,gzip_buff,lenp);
}

int receive_body( int block_number, int body_size )
{
	int a, b;
	int key;
	int last_block = 0;
	unsigned long received_crc, my_crc;
	unsigned char block_buffer[2+1024+4];

	block_number ++;

	/*
	 * send request block code.
	 */
	serial_putc( 0x69 );
	serial_putc( block_number%10 );

	/*
	 * get real body.
	 */
	for( a=0; a<2+1024+4; a++ )
	{
		for( b=0; b<10; b++ )
		{
			key = dg_getc( TIMEOUT );
			if( key != -1 )
				break;
		}
		if( b == 10 )
		{
			dprintf( "body time out.(%d,%d)\n", block_number, a );
			return -1;
		}

		block_buffer[a] = key;
	}

	/*
	 * check block is not broken.
	 */
	if( block_buffer[0] != 0x69 )
	{
		dprintf( "block header is broken.(0x%02x)\n", block_buffer[0] );
		return -1;
	}
	if( (body_size+1023)/1024 == block_number )
	{
		if( block_buffer[1] != 0xee )
		{
			dprintf( "block sequence is broken.\n" );
			return -1;
		}
		last_block = 1;
	}
	else
	{
		if( block_buffer[1] != block_number%10 )
		{
			dprintf( "block sequence is broken.(%d)\n", block_buffer[1] );
			return -1;
		}
	}
	received_crc = block_buffer[2+1024+0]<<24;
	received_crc |= block_buffer[2+1024+1]<<16;
	received_crc |= block_buffer[2+1024+2]<<8;
	received_crc |= block_buffer[2+1024+3]<<0;
	my_crc = getCRC32( &block_buffer[2], 1024 );
	if( received_crc != my_crc )
	{
		dprintf( "body crc error.\n" );
		return -1;
	}

	/*
	 * move data to body buffer.
	 */
	memcpy( &upg_buffer[(block_number-1)*1024], &block_buffer[2], 1024 );

	return 0;
}

int do_serial_download( void )
{
	int data_size;
	int ret = 0;
	int a;

	dprintf( "start download... wait...\n" );
	/*
	 * get header.
	 */
	if( (data_size=receive_header())<0 )
	{
		dprintf( "header error.\n" );
		ret = -1;
		goto terminate;
	}

	/*
	 * get body...
	 */
	for( a=0; a<(data_size+1023)/1024; a++ )
	{
		if( receive_body( a, data_size ) )
		{
			dprintf( "body error.\n" );
			ret = -1;
			goto terminate;
		}

		front_persent( a, (data_size+1023)/1024 );
	}

	/*
	 * burn image to flash.
	 */
	if (gzipped==1)
		ungzip_image();

	ret = burn_flash();

terminate:
	serial_putc( 0x70 );
	serial_putc( 0x70 );
	serial_putc( 0x70 );

	return ret;
}

int serial_main( int argc, char *argv[] )
{
	DECLARE_GLOBAL_DATA_PTR;

//	int retry;
	int key;
	int baudrate;
	int a;
	int ret = 0;

	front_puts( "serial upgrade" );

	gen_crc_table();

	upg_buffer = (unsigned char*)gd->dgs_upg_buffer;

#if 1
	{
		int keys[2];

		keys[0] = key_release;
		keys[1] = key_front_release;
		keys[3] = key_null;

		front_waitkey( keys, 5 );
	}
#else
	{
		unsigned long long starttime;

		starttime = get_ticks();

		while( get_ticks() < (starttime+5*get_tbclk()) )
		{
			if( front_tstc() )
			{
				key = front_getkey();
				if( key == key_release || key == key_front_release )
					break;
			}
		}
	}
#endif

	dprintf( "serial download...\n" );
//	for( retry=0; retry<RETRY; retry++ )
	while( 1 )
	{
		/*
		 * check key is pressed.
		 */
		dprintf( "\n" );
		dprintf( "press Ok button to start download.\n" );
		dprintf( "Or press power to reboot.\n" );
#if 1
		{
			int keys[5];
			int key;

			keys[0] = key_ok;
			keys[1] = key_front_ok;
			keys[2] = key_power;
			keys[3] = key_front_power;
			keys[4] = key_null;

			key = front_waitkey( keys, 0 );
			if( key == key_power || key == key_front_power )
				goto terminate;
		}
#else
		while( 1 )
		{
			key = front_getkey();
			if( key == key_ok || key == key_front_ok )
			{
				while( 1 )
				{
					key = front_getkey();
					if( key == key_release || key == key_front_release )
						break;
				}
				break;
			}

			/*
			 * check pwr button.
			 */
			if( key == key_power || key == key_front_power )
			{
				while( 1 )
				{
					key = front_getkey();
					if( key == key_release || key == key_front_release )
						break;
				}
				goto terminate;
			}
		}
#endif

		gd->baudrate = 9600;
		serial_setbrg();

		while( serial_tstc() )
			serial_getc();
		serial_putc( _CMD_PC_BR );

		key = dg_getc( TIMEOUT*4 );	// large file in PC can couse timeout.
		if( key != 0x22 )
		{
			dprintf( "PC is not responding.. will retry.\n" );
			goto do_retry;
		}

		key = dg_getc( TIMEOUT );
		switch( key )
		{
			case 1:
				baudrate = 9600;
				break;
			case 2:
				baudrate = 19200;
				break;
			case 3:
				baudrate = 38400;
				break;
			case 4:
				baudrate = 57600;
				break;
			case 5:
				baudrate = 115200;
				break;
			case 6:
				baudrate = 230400;
				break;
			default:
				dprintf( "invalide baudrate.\n" );
				goto do_retry;
		}

		/*
		 * change real baud rate.
		 */
		if( baudrate != 9600 )
		{
			gd->baudrate = baudrate;
			serial_setbrg();
		}

		/*
		 * wait to PC change baud rate.
		 */
		for( a=0; a<10; a++ )
		{
			serial_putc( _CMD_PC_READY );
			key = dg_getc( TIMEOUT );
			if( key == _CMD_PC_READY )
				break;
		}
		if( a == 10 )
		{
			dprintf( "PC doesn`t respond PC rdy.\n" );
			goto do_retry;
		}

		/*
		 * receive function number...
		 */
		for( a=0; a<10; a++ )
		{
			serial_putc( _CMD_WHICH_FUNCTION );
			key = dg_getc( TIMEOUT );
			if( key != -1 )
				break;
		}
		if( a == 10 )
		{
			dprintf( "PC doesn`t send function number.\n" );
			goto do_retry;
		}
		serial_putc( key );

		/*
		 * Do PC wanted work.
		 */
		switch( key )
		{
			case 7:
				ret = do_serial_download();
				break;
			case 8:
				break;
			default:
				goto do_retry;
		}

		if( ret )
			goto do_retry;

		continue;
do_retry:
		front_puts( "error" );
	}

terminate:
//	if( retry == RETRY )
//		ret = -1;

	gd->baudrate = CONFIG_BAUDRATE;
	serial_setbrg();

	clean_crc_table();

	return ret;
}

int do_serial_upgrade( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] )
{
	return serial_main(argc,argv);
}

U_BOOT_CMD(
		serial_upgrade, 14, 0, do_serial_upgrade,
		"serial_upgrade- upgrade system with serial port.\n",
		"run serial download.\n"
		" if you want to debug, append debug after dg_serial.\n" );

#endif
