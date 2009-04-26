#include <common.h>
#include <malloc.h>

#ifdef CONFIG_DGS_UPGRADE

#include "common/upgrade.h"

#ifdef CONFIG_DGS_FRONT
#include "common/front.h"
#endif

unsigned char *upg_buffer;
int upg_buffer_len;
enum _upg_buffer_status upg_buffer_status;
char *upg_buffer_message;
int upg_buffer_errorno;
int upg_verbose_message;
static int boot_try_fail=0;

/*
 * start writing the image to flash.
 * return:
 * 	0 : nomal terminate. all is completed.
 * 	2 : writing failed. this will not happen...
 * 	3 : image error. unknown image or crc error.
 */
int burn_flash( void )
{
	struct _image_header header;
	int ret = 0;
	unsigned long crc;
	unsigned long data_start;
	unsigned long data_size;
	unsigned long erase_size;
	int err,i;


	unsigned long nb_erase_size;
	unsigned long nb_data_size;
	unsigned long uboot_len = CFG_MONITOR_LEN;
	char badboot=0;

	puts( "checking received image...\n");

	/*
	 * check header and body is correct.
	 */
	crc = crc32( 0xffffffff, upg_buffer+4, HEADER_SIZE-4 );
	if( *(unsigned long*)upg_buffer != crc )
	{
		printf( "header crc is not correct.(got 0x%08x,expected 0x%08x)\n",
				(int)*(unsigned long*)upg_buffer, (int)crc );
		ret = 3;
#ifdef CONFIG_DGS_FRONT
		front_puts( "Er 1" );
#endif
		upg_buffer_errorno = 1;
		goto terminate_witherr;
	}
	memcpy( &header, upg_buffer, sizeof(struct _image_header) );
#ifdef CONFIG_DGS_UPGRADE_DEBUG
	if( upg_verbose_message )
	{
		printf( "magic          : 0x%08x\n", (int)header.magic );
		printf( "structure_size : 0x%08x\n", (int)header.structure_size );
		printf( "vendor_id      : 0x%08x\n", (int)header.vendor_id );
		printf( "product_id     : 0x%08x\n", (int)header.product_id );
		printf( "hw_model       : 0x%08x\n", (int)header.hw_model );
		printf( "hw_version     : 0x%08x\n", (int)header.hw_version );
		printf( "start_addr     : 0x%08x\n", (int)header.start_addr );
		printf( "erase_size     : 0x%08x\n", (int)header.erase_size );
		printf( "data_offset    : 0x%08x\n", (int)header.data_offset );
		printf( "data_size      : 0x%08x\n", (int)header.data_size );
		printf( "data_crc       : 0x%08x\n", (int)header.data_crc );
		printf( "name           : \"%s\"\n", header.name );
	}
#endif
	crc = crc32( 0xffffffff, &upg_buffer[header.data_offset], header.data_size );
	if( header.data_crc != crc )
	{
		printf( "data crc is not correct.(got 0x%08x,expected 0x%08x)\n",
				(int)header.data_crc, (int)crc );
		ret = 3;
#ifdef CONFIG_DGS_FRONT
		front_puts( "Er 2" );
#endif
		upg_buffer_errorno = 2;
		goto terminate_witherr;
	}

	/*
	 * determin image is mine.
	 */
	if( header.vendor_id != MY_VENDOR_ID )
	{
		printf( "vendor id is not mine.(got 0x%08x,expected 0x%08x)\n",
				(int)header.vendor_id, MY_VENDOR_ID );
		ret = 3;
#ifdef CONFIG_DGS_FRONT
		front_puts( "Er 3" );
#endif
		upg_buffer_errorno = 3;
		goto terminate_witherr;
	}
	if( header.product_id != MY_PRODUCT_ID )
	{
		printf( "product id is not mine.(got 0x%08x,expected 0x%08x)\n",
				(int)header.product_id, MY_PRODUCT_ID );
		ret = 3;
#ifdef CONFIG_DGS_FRONT
		front_puts( "Er 4" );
#endif
		upg_buffer_errorno = 4;
		goto terminate_witherr;
	}
	if( header.hw_model != MY_HW_MODEL )
	{
		printf( "hardware model is not mine.(got 0x%08x,expected 0x%08x)\n",
				(int)header.hw_model, MY_HW_MODEL );
		ret = 3;
#ifdef CONFIG_DGS_FRONT
		front_puts( "Er 5" );
#endif
		upg_buffer_errorno = 5;
		goto terminate_witherr;
	}

	if( header.hw_version != MY_HW_VERSION )
	{
		printf( "hardware version is not mine.(got 0x%08x,expected 0x%08x)\n",
				(int)header.hw_version, MY_HW_VERSION );
		ret = 3;
#ifdef CONFIG_DGS_FRONT
		front_puts( "Er 5" );
#endif
		upg_buffer_errorno = 5;
		goto terminate_witherr;
	}

	header.name[H_NAME_SIZE-1] = 0;
	printf( "got %s image.\n", header.name );
	data_start = header.start_addr;
	data_size  = header.data_size;
	nb_data_size = header.data_size;
	erase_size = header.erase_size;
	nb_erase_size = header.erase_size;

	if( data_start+erase_size-1 >= (unsigned long)CFG_MONITOR_BASE )
	{
		puts( "updating boot loader.\n" );
		puts( "DO NOT TURN OFF THE POWER while updating!!!\n" );
		nb_erase_size = (unsigned long)CFG_MONITOR_BASE-data_start;
		nb_data_size = data_size -uboot_len;

		badboot=1;
		for (i=0;i<40;i++)
		{
			if(!memcmp(&upg_buffer[header.data_offset]+ (data_size -(0x2000*i))+4,"U-Boot",6))
			{
				printf("Uboot found at @ %x\n",(data_size -(0x2000*i)));
				badboot=0;
			}
		}

		if (badboot)
		{
			puts("WARNING! IT DOES NOT SEEM THAT YOU ARE FLASHING U-BOOT, THIS MAY BREAK YOUR BOX\n");
			ret = 3;
			boot_try_fail++;
			if (boot_try_fail <2)
			{
#ifdef CONFIG_DGS_FRONT
			front_puts( "Er10" );
#endif
			upg_buffer_errorno = 10;
			goto terminate_witherr;
			}
			puts("WARNING! I SEE YOU ARE INSISTING!, I WILL FLASH, BUT AT IT'S AT YOUR OWN RISK!!\n");
		} else {
			boot_try_fail=0;
			printf("New bootloader version %s\n" ,&upg_buffer[header.data_offset]+ (data_size -uboot_len)+4 );
		}

		if (erase_size != 0 )
		{
			/*
			 * erase bootloader flash first...
			 */
			if (crc32( 0xffffffff, CFG_MONITOR_BASE+64, uboot_len-64)== crc32( 0xffffffff, &upg_buffer[header.data_offset]+ (data_size -uboot_len)+64, uboot_len-64))
			{
				puts("Same bootloader, skipping!\n");
				goto do_flash;
			}
			upg_buffer_status = UPGBUF_ERASE;
	#ifdef CONFIG_DGS_FRONT
	#ifdef CONFIG_DGS_FRONT_7SEG
			front_puts( "ErAS" );
	#else
			front_puts( "ERASING" );
	#endif
	#endif
			printf( "Erasing bootloader at 0x%08x - 0x%08x\n", (int)CFG_MONITOR_BASE, (int)CFG_MONITOR_BASE+uboot_len-1 );
			if( flash_sect_erase(CFG_MONITOR_BASE,CFG_MONITOR_BASE+uboot_len-1))
			{
				puts( "erase error.\n" );
				ret = 2;
	#ifdef CONFIG_DGS_FRONT
				front_puts( "Er 8" );
	#endif
				upg_buffer_errorno = 8;
				goto terminate_witherr;
			}
		}
		if (data_size != 0 )
		{

			/*
			 * and... write the boot to flash...
			 */
			upg_buffer_status = UPGBUF_WRITE;
	#ifdef CONFIG_DGS_FRONT
			front_puts( "FLASHING" );
	#endif
			puts( "Flasing Bootloader...\n" );
			err = flash_write(
					&upg_buffer[header.data_offset]+ (data_size -uboot_len),
					CFG_MONITOR_BASE,
					CFG_MONITOR_BASE+uboot_len-1
					);
			if( err )
			{
				flash_perror( err );
				ret = 2;
	#ifdef CONFIG_DGS_FRONT
				front_puts( "Er 9" );
	#endif
				upg_buffer_errorno = 9;
				goto terminate_witherr;
			}
		}
	}
	do_flash:
	if( nb_erase_size != 0 )
	{

		/*
		 * erase flash first...
		 */
		upg_buffer_status = UPGBUF_ERASE;
#ifdef CONFIG_DGS_FRONT
#ifdef CONFIG_DGS_FRONT_7SEG
		front_puts( "ErAS" );
#else
		front_puts( "ERASING" );
#endif
#endif
		printf( "erase 0x%08x - 0x%08x\n", (int)data_start, (int)data_start+nb_erase_size-1 );
		if( flash_sect_erase(data_start,data_start+nb_erase_size-1) )
		{
			puts( "erase error.\n" );
			ret = 2;
#ifdef CONFIG_DGS_FRONT
			front_puts( "Er 8" );
#endif
			upg_buffer_errorno = 8;
			goto terminate_witherr;
		}
	}
	/*
	 * and... write the body to flash...
	 */
	if( nb_data_size != 0 )
	{
		upg_buffer_status = UPGBUF_WRITE;
#ifdef CONFIG_DGS_FRONT
		front_puts( "FLASHING" );
#endif
		puts( "write to flash...\n" );
		err = flash_write(
				&upg_buffer[header.data_offset],
				data_start,
				nb_data_size
				);
		if( err )
		{
			flash_perror( err );
			ret = 2;
#ifdef CONFIG_DGS_FRONT
			front_puts( "Er 9" );
#endif
			upg_buffer_errorno = 9;
			goto terminate_witherr;
		}
	}

	puts( "completed...\n" );

#ifdef CONFIG_DGS_FRONT
	front_puts( "done" );
#endif

	goto terminate;

terminate_witherr:
	if( ret == 2 )
	{
		puts( "WARNING!!! The STB may not boot in next power up.\n" );
		puts( "The error is unrecoverable.\n" );
		puts( "Capture this screen and contact to vendor.\n" );
	}
	else if( ret == 3 )
	{
		puts( "Maybe you transferred wrong file.\n" );
	}
	printf( "My hardware model  : %08x\n", MY_HW_MODEL );
	printf( "My hardware version: %08x\n", MY_HW_VERSION );

terminate:
	upg_buffer_status = UPGBUF_UNUSED;

	puts( "Select another file.\n" );
	puts( "Or press power to reboot.\n" );
	puts( "\n" );

	return ret;
}

#if (CONFIG_COMMANDS)

#include <command.h>

int set_upgrade_buffer(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	DECLARE_GLOBAL_DATA_PTR;
	char buf[32];
	sprintf(buf, "0x%08x", (unsigned char*)gd->dgs_upg_buffer);
	setenv("upgrade_buffer", buf);
	return 0;
}


U_BOOT_CMD(
	setupdbuf,	1,	0,	set_upgrade_buffer,
	"setupdbuf - sets upgrade buffer variable\n",
	NULL
);

int gunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp);

int burn_flash_img( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] )
{
	DECLARE_GLOBAL_DATA_PTR;
	char gzip_buff[0x802000];
	unsigned long lenp;

	upg_buffer = (unsigned char*)gd->dgs_upg_buffer;

	if( (strcmp( upg_buffer+1, CFG_SERIALDOWN_HEADER )) && (strcmp( upg_buffer+1, CFG_SERIALDOWN_HEADER"_gz" )))
	{
		printf( "image is not mine.(got\"%s\",expected\"%s\" or \"%s\")\n",
				upg_buffer+1, CFG_SERIALDOWN_HEADER,CFG_SERIALDOWN_HEADER"_gz" );
		return -1;
	}
	if (!strcmp( upg_buffer+1, CFG_SERIALDOWN_HEADER"_gz" ))
	{
		puts("Ungzipping image: ");
		gunzip (gzip_buff,0x801040,(unsigned char*)gd->dgs_upg_buffer+0x40,&lenp);
		printf ("%d, bytes.. done!\n",lenp);
		memcpy((unsigned char*)gd->dgs_upg_buffer+0x40,gzip_buff,lenp);
	}
	upg_buffer = (unsigned char*)gd->dgs_upg_buffer+0x40;

	return burn_flash();
}


U_BOOT_CMD(
	dgsupdate,	1,	0,	burn_flash_img,
	"dgsupdate - burn image to flash\n",
	NULL
);


#endif

#endif
