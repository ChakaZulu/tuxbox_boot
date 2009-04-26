#include <common.h>

#ifdef CONFIG_CFB_CONSOLE

#include <video_fb.h>
#include <asm/processor.h>

#ifdef CONFIG_JPEG
#define SRC_FROM_MEM
#define EMBEDED
#include <../common/jpeg/jpeglib.h>
#endif

#define DENC0_CR1		0x131
#define DENC0_CR2		0x133
#define DENC0_VSR		0x13f

#define MEM_BASE		0xa0000000
#define GRA_OFFSET		0x700000
#define GRA_BASE		(MEM_BASE+GRA_OFFSET)
#define OSD_DATA_START_OFFSET	0x1000
#define CLUT			((unsigned short*)(GRA_BASE+sizeof(osdhdr_t)+sizeof(osdhdr_ext1_t)+sizeof(osdhdr_ext2_t)))
#define IMAGE_BASE		((unsigned char*)(GRA_BASE+OSD_DATA_START_OFFSET))

#define DENC_NTSC		1
#define DENC_PAL		2

#define VIDEO_WIDTH		720
#define VIDEO_HEIGHT_PAL	576
#define VIDEO_HEIGHT_NTSC	480
#define WELCOME_BPP		4

#define SCR_WIDTH		600
#define SCR_HEIGHT_PAL		440
#define SCR_HEIGHT_NTSC		360
#define SCR_H_OFFSET		((VIDEO_WIDTH-SCR_WIDTH)/2)

#define GZIPED_WEL		1
#define JPEG_WEL		2

#define DEBUG
#ifdef debug
#undef debug
#endif
#ifdef DEBUG
#define debug(a,b...) printf(a,##b)
#else
#define debug(a,b...) do{}while(0)
#endif

#define MEM_SEG0		0x175
#define MEM_SEG1		0x176
#define MEM_SEG2		0x177
#define MEM_SEG3		0x178
#define OSD_MODE		0x151
#define VIDEO_CNTL		0x140
#define DISP_MODE		0x154
#define DISP_DLY		0x155
#define VID0_GSLA		0x159
#define VID0_GPBASE		0x16e

#define MEM_SEG0_ADDR		MEM_BASE
#define MEM_SEG0_SIZE		2		// 4MByte
#define MEM_SEG1_ADDR		(MEM_BASE+0x0400000)
#define MEM_SEG1_SIZE		2		// 4MByte
#define MEM_SEG2_ADDR		(MEM_BASE+0x0800000)
#define MEM_SEG2_SIZE		2		// 4MByte
#define MEM_SEG3_ADDR		(MEM_BASE+0x0c00000)
#define MEM_SEG3_SIZE		2		// 4MByte
#define MEM_SEG4_ADDR		(MEM_BASE+0x1000000)
#define MEM_SEG4_SIZE		2		// 4MByte
#define MEM_SEG5_ADDR		(MEM_BASE+0x1400000)
#define MEM_SEG5_SIZE		2		// 4MByte
#define MEM_SEG6_ADDR		(MEM_BASE+0x1800000)
#define MEM_SEG6_SIZE		2		// 4MByte
#define MEM_SEG7_ADDR		(MEM_BASE+0x1c00000)
#define MEM_SEG7_SIZE		2		// 4MByte

#pragma pack(1)
typedef struct osdhdr
{
	unsigned color_table_update:1;
	unsigned region_hsize:8;
	unsigned shade_level:4;
	unsigned high_color:1;
	unsigned start_row:9;
	unsigned start_column:9;
	unsigned link_addr:16;
	unsigned color_resolution:1;
	unsigned region_vsize:9;
	unsigned pixel_resolution:1;
	unsigned blend_level:4;
	unsigned force_transparency:1;
} osdhdr_t;

typedef struct osdhdr_ext1
{
	unsigned int link_addr:19;
	unsigned int link_addr_lsb:4;
	unsigned int hsb_ext:2;
	unsigned int h_ext:1;
	unsigned int dcus:1;
	unsigned int shade_ext:2;
	unsigned int dcub:1;
	unsigned int blend_ext:2;
} osdhdr_ext1_t;

typedef struct osdhdr_ext2
{
	unsigned int horizontal_fir_scaling_control:4;
	unsigned int tiling_control:2;
	unsigned int anti_flicker_correction:2;
	unsigned int reserved1:1;
	unsigned int color_specific_blending:1;
	unsigned int header_extecsion3:1;
	unsigned int reserved2:1;
	unsigned int chroma_bitmap_link_address_enable:1;
	unsigned int chroma_bitmap_link_address:19;
} osdhdr_ext2_t;
#pragma pack()

struct _welcome_header
{
	unsigned long crc;
	unsigned long data_len;
	unsigned long compress_type;
	unsigned long bg_color;
};

GraphicDevice gGD;

/* NOTICE
 * NTSC and PAL has different height of tv out. */
static int video_mode = DENC_PAL;
static int video_height = VIDEO_HEIGHT_PAL;
static int scr_height = SCR_HEIGHT_PAL;
static int scr_v_offset = (VIDEO_HEIGHT_PAL-SCR_HEIGHT_PAL)/2;

static int denc_init(int mode)
{
	switch(mode) {
		case DENC_NTSC:
			mtdcr(DENC0_CR1,0x90100040);
			break;
		case DENC_PAL:
			mtdcr(DENC0_CR1,0x91100040);
			break;
	}

	mtdcr( DENC0_VSR, 0x78000000 );
	mtdcr( DENC0_CR2, 0x000080c0 );

	return 0;
}

#if 0
static void rgb_to_ycbcr(
		unsigned char  r, unsigned char   g, unsigned char   b,
		unsigned char *y, unsigned char *cb, unsigned char *cr)
{
	// Y  =  0.257*R + 0.504*G + 0.098*B + 16
	// CB = -0.148*R - 0.291*G + 0.439*B + 128
	// CR =  0.439*R - 0.368*G - 0.071*B + 128
	*y  = (unsigned char)((8432*(unsigned long)r + 16425*(unsigned long)g + 3176*(unsigned long)b + 16*32768)>>15);
	*cb = (unsigned char)((128*32768 + 14345*(unsigned long)b - 4818*(unsigned long)r -9527*(unsigned long)g)>>15);
	*cr = (unsigned char)((128*32768 + 14345*(unsigned long)r - 12045*(unsigned long)g-2300*(unsigned long)b)>>15);

	return;
}

void video_set_lut (unsigned int index,
                    unsigned char r, unsigned char g, unsigned char b)
{
	unsigned char y, cb, cr;

	rgb_to_ycbcr( r, g, b, &y, &cb, &cr );
	CLUT[index] = ((y>>2) << 10) | ((cb>>4) << 6) | ((cr>>4) << 2);
}
#else
static void rgb_to_ycbcr(unsigned long r,
                         unsigned long g, unsigned long b, int *r_y,
                         int *r_cb, int *r_cr)
{

	long y;
	long cb;
	long cr;
	long r1;

	/*-------------------------------------------------------------------------+
	| Convert using formula on page 42 of Video Demystified.
	+-------------------------------------------------------------------------*/
	r1 = (r * 77) + (g * 150) + (b * 29);

	if ((r1 % 256) > 128)
	{
		y = 1;
	}
	else
	{
		y = 0;
	}

	y += r1 / 256;
	r1 = ((-((long) r * 44)) - ((long) g * 87) + ((long) b * 131));

	if ((r1 % 256) > 128)
	{
		cb = 1;
	}
	else if ((r1 % 256) < (-128))
	{
		cb = -1;
	}
	else
	{
		cb = 0;
	}

	cb += (r1 / 256) + 128;
	r1 = (((long) r * 131) - ((long) g * 110) - ((long) b * 21));

	if ((r1 % 256) > 128)
	{
		cr = 1;
	}
	else if ((r1 % 256) < (-128))
	{
		cr = -1;
	}
	else
	{
		cr = 0;
	}

	cr += (r1 / 256) + 128;
	/*-------------------------------------------------------------------------+
	| Bound check.
	+-------------------------------------------------------------------------*/

	if (y < 0)
	{
		y = 0;
	}

	if (y > 255)
	{
		y = 255;
	}

	if (cb < 0)
	{
		cb = 0;
	}

	if (cb > 255)
	{
		cb = 255;
	}

	if (cr < 0)
	{
		cr = 0;
	}

	if (cr > 255)
	{
		cr = 255;
	}

	/*-------------------------------------------------------------------------+
	  | Scale to accepted values.
	  +-------------------------------------------------------------------------*/
	if ((y % 4) > 2)
	{
		*r_y = (y / 4) + 1;
	}
	else
	{
		*r_y = y / 4;
	}

	if ((*r_y) > 0x3F)
	{
		*r_y = 0x3F;
	}

	if ((cb % 16) > 8)
	{
		*r_cb = (cb / 16) + 1;
	}
	else
	{
		*r_cb = cb / 16;
	}

	if ((*r_cb) > 0xF)
	{
		*r_cb = 0x0F;
	}

	if ((cr % 16) > 8)
	{
		*r_cr = (cr / 16) + 1;
	}
	else
	{
		*r_cr = cr / 16;
	}

	if ((*r_cr) > 0xF)
	{
		*r_cr = 0x0F;
	}

	return;
}

void video_set_lut( unsigned int index,
		unsigned char r, unsigned char g, unsigned char b )
{
	int y, cb, cr;

	rgb_to_ycbcr( r, g, b, &y, &cb, &cr );
	CLUT[index] = ((y & 0x3F) << 10) | ((cb & 0xF) << 6) | ((cr & 0xF) << 2);
}

#endif

void video_clear_screen( void )
{
	int a, b;

	for( a=0; a<video_height; a++ )
	{
		for( b=0; b<VIDEO_WIDTH; b++ )
		{
			IMAGE_BASE[a*VIDEO_WIDTH+b] =
				0x00;
		}
	}

	video_set_lut( 0x00, 0, 0, 0 );
}

#ifdef CFG_VIDEO_STARTUP_IMAGE
#ifdef CONFIG_JPEG
void putpixel( int x, int y, int r, int g, int b )
{
	unsigned char y;
	unsigned char cb;
	unsigned char cf;

	rgb_to_ycbcr( r, g, b, &y, &cb, &cr );
}

int decompress_jpeg( unsigned char *buf, unsigned int len, struct _welcome_header *wel )
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPARRAY buffer;		/* Output row buffer */
	int x, y, dx;
	int sleft, stop;

	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_decompress( &cinfo );
	jpeg_mem_src( &cinfo, buf, len );
	jpeg_read_header( &cinfo, TRUE );
	jpeg_start_decompress( &cinfo );
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo,
		 JPOOL_IMAGE,
		 cinfo.output_width * cinfo.output_components,
		 1);

	debug( "width %d height %d components %d\n",
			cinfo.output_width,
			cinfo.output_height,
			cinfo.output_components );
	if( cinfo.output_components != 3 )
		puts( "components 3 is not supported.\n" );
	sleft = (VIDEO_WIDTH-cinfo.output_width)/2;
	stop = (video_height-cinfo.output_height)/2;
	if( (sleft<0) || (stop<0) )
	{
		puts( "too big image... will not draw...\n");
		return -1;
	}

	switch( WELCOME_BPP )
	{
		case 1:
			puts( "!!\n" );
			break;
		case 2:
			puts( "!!\n" );
			break;
		case 4:
			break;
		default:
			puts( "!!\n" );
			break;
	}
	for( y=0; y<stop; y++ )
	{
		for( x=0; x<VIDEO_WIDTH; x++ )
			IMAGE_BASE[y*VIDEO_WIDTH+x] = wel->bg_color;
	}
	for( ; y<cinfo.output_height+stop; y++ )
	{
		jpeg_read_scanlines( &cinfo, buffer, 1 );

		for( x=0; x<sleft; x++ )
			IMAGE_BASE[y*VIDEO_WIDTH+x] = wel->bg_color;

		switch( WELCOME_BPP )
		{
			case 4:
				for( dx=0; dx<cinfo.output_width; x++,dx++ )
				{
					putpixel( x, y,
							buffer[0][dx*3+0],
							buffer[0][dx*3+1],
							buffer[0][dx*3+2] );
				}
				break;
			default:
				break;
		}

		for( ; x<VIDEO_WIDTH; x++ )
			IMAGE_BASE[y*VIDEO_WIDTH+x] = wel->bg_color;
	}
	for( ; y<video_height; y++ )
	{
		for( x=0; x<VIDEO_WIDTH; x++ )
			IMAGE_BASE[y*VIDEO_WIDTH+x] = wel->bg_color;
	}

	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );

	return 0;
}
#endif

int video_draw_startup_image( void )
{
	extern int gunzip(void*,int,unsigned char*,int*);

	unsigned char *img;
	int a;
	int ret;
	unsigned long crc;
	struct _welcome_header wel_header;

	img = (unsigned char*)CFG_VIDEO_STARTUP_IMAGE_ADDR;
	memcpy( &wel_header, img, sizeof(wel_header) );

	/*
	 * check image.
	 */
	puts( "checking welcome image... " );
	if( wel_header.data_len > CFG_VIDEO_STARTUP_IMAGE_SIZE-sizeof(wel_header) )
	{
		puts( "length error.\n" );
		goto error;
	}
	if( wel_header.crc != (crc=crc32(0xffffffff,img+4,wel_header.data_len+sizeof(wel_header)-4)) )
	{
		printf( "crc error.(got 0x%08x,expected 0x%08x)\n", wel_header.crc, crc );
		goto error;
	}
	puts( "ok.\n" );

	switch( wel_header.compress_type )
	{
		case GZIPED_WEL:
			debug( "gzip compressed welcome image.\n" );

			/*
			 * set pallete.
			 */
			img += sizeof(wel_header);
			for( a=0; a<256; a++ )
				video_set_lut( a, img[a*3+0], img[a*3+1], img[a*3+2] );

			/*
			 * unzip image...
			 */
			img += 256*3;
			wel_header.data_len -= 256*3;
			ret = gunzip( IMAGE_BASE, 0x100000, img, (int*)&wel_header.data_len );

			return ret;
#ifdef CONFIG_JPEG
		case JPEG_WEL:
			debug( "jpeg compressed welcome image.\n" );
			return decompress_jpeg( img+sizeof(struct _welcome_header), wel_header.data_len, &wel_header );
#endif
		default:
			printf( "unknown compression type.(%d)\n", wel_header.compress_type );
			break;
	}

error:
	puts( "clear screen.\n" );
	video_clear_screen();

	return -1;
}
#endif

void video_change_size( int mode )
{
	osdhdr_t osd;
	osdhdr_ext1_t osd_e1;
	osdhdr_ext2_t osd_e2;
	unsigned int antiflicker = 0;
	char *s;

	s = getenv( "antiflicker" );
	if( s != NULL )
	{
		antiflicker = simple_strtoul( s, NULL, 0 );
		if( antiflicker > 3 )
		{
			puts( "limite anti-flicker to 3.\n" );
			antiflicker = 3;
		}
	}

	/* disable the osd */
	mtdcr( OSD_MODE, mfdcr( OSD_MODE ) & 0x00000048 );

	/*
	 * mode 0 : full screen, used when draw welcome image.
	 * mode 1 : small screen, used when used as console.
	 */
	if( mode == 0 )
	{
		osd.start_row = 0;
		osd.start_column = 0;
		osd.region_hsize = VIDEO_WIDTH/4;
		osd.region_vsize = video_height/2;
	}
	else
	{
		osd.start_row = scr_v_offset/2;
		osd.start_column = SCR_H_OFFSET/2;
		osd.region_hsize = SCR_WIDTH/4;
		osd.region_vsize = scr_height/2;
	}

	osd.color_table_update = 1;
	osd.shade_level = 0;
	osd.high_color = 1;
	osd.link_addr = 0;
	osd.color_resolution = 0;
	osd.pixel_resolution = 0;
	osd.blend_level = 0;
	osd.force_transparency = 0;

	osd_e1.link_addr = OSD_DATA_START_OFFSET/4;
	osd_e1.link_addr_lsb = 0;
	osd_e1.hsb_ext = 0;
	osd_e1.h_ext = 1;
	osd_e1.dcus = 0;
	osd_e1.shade_ext = 0;
	osd_e1.dcub = 0;
	osd_e1.blend_ext = 0;

	osd_e2.horizontal_fir_scaling_control = 0;
	osd_e2.tiling_control = 0;
	osd_e2.anti_flicker_correction = antiflicker;
	osd_e2.reserved1 = 0;
	osd_e2.color_specific_blending = 0;
	osd_e2.header_extecsion3 = 0;
	osd_e2.reserved2 = 0;
	osd_e2.chroma_bitmap_link_address_enable = 0;
	osd_e2.chroma_bitmap_link_address = 0;

	memcpy( (void*)GRA_BASE, &osd, sizeof(osd) );
	memcpy( (void*)(GRA_BASE+sizeof(osd)), &osd_e1, sizeof(osd_e1) );
	memcpy( (void*)(GRA_BASE+sizeof(osd)+sizeof(osd_e1)), &osd_e2, sizeof(osd_e2) );

	/* enable the osd */
	mtdcr( OSD_MODE, mfdcr( OSD_MODE ) | 0x00201068 );

	return;
}

void *video_hw_init (void)
{
	char *s;

	s = getenv( "videofmt" );
	if( s != NULL )
	{
		if( !strcmp(s,"ntsc") )
		{
			video_height = VIDEO_HEIGHT_NTSC;
			scr_height = SCR_HEIGHT_NTSC;
			scr_v_offset = (video_height-scr_height)/2;
			video_mode = DENC_NTSC;
		}
		else
		{
			video_height = VIDEO_HEIGHT_PAL;
			scr_height = SCR_HEIGHT_PAL;
			scr_v_offset = (video_height-scr_height)/2;
			video_mode = DENC_PAL;
		}
	}

	denc_init( video_mode );

	mtdcr( VIDEO_CNTL, 0x00000000 );
	mtdcr( DISP_DLY, 0x00004714 );
	mtdcr( VID0_GPBASE, GRA_OFFSET/128 );
	mtdcr( VID0_GSLA, 0x00000000 );
	mtdcr( MEM_SEG0,
			(MEM_SEG4_SIZE<<28)|(MEM_SEG4_ADDR>>4)|
			(MEM_SEG0_SIZE<<12)|(MEM_SEG0_ADDR>>20) );
	mtdcr( MEM_SEG1,
			(MEM_SEG5_SIZE<<28)|(MEM_SEG5_ADDR>>4)|
			(MEM_SEG1_SIZE<<12)|(MEM_SEG1_ADDR>>20) );
	mtdcr( MEM_SEG2,
			(MEM_SEG6_SIZE<<28)|(MEM_SEG6_ADDR>>4)|
			(MEM_SEG2_SIZE<<12)|(MEM_SEG2_ADDR>>20) );
	mtdcr( MEM_SEG3,
			(MEM_SEG7_SIZE<<28)|(MEM_SEG7_ADDR>>4)|
			(MEM_SEG3_SIZE<<12)|(MEM_SEG3_ADDR>>20) );
	if( video_mode == DENC_NTSC )
		mtdcr( DISP_MODE, 0x0088e200 );
	else
		mtdcr( DISP_MODE, 0x0088e600 );

#ifdef	CFG_VIDEO_STARTUP_IMAGE
#	ifndef	CFG_VIDEO_START_FUNCTION
#	error "CFG_VIDEO_START_FUNCTION have to define with CFG_VIDEO_STARTUP_IMAGE."
#	endif
	video_draw_startup_image();
#else
#endif

	video_change_size( 0 );

	memset( &gGD, 0, sizeof(gGD) );
	gGD.winSizeX = SCR_WIDTH;
	gGD.winSizeY = scr_height;
	gGD.gdfBytesPP = 1;
	gGD.gdfIndex = GDF__8BIT_INDEX;
	gGD.frameAdrs = (unsigned int)IMAGE_BASE;

	return &gGD;
}

#endif
