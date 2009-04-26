#include <common.h>
#include <command.h>
#include "common/front.h"
#include "common/upgrade.h"
#include "common/flash_img_info.h"


#define numVmodes 2

static struct
{
	char *video_mode;
	char *descr;
	char *vfdescr;
}  vmodeList[numVmodes] =
{
	{ "pal", "Selected PAL format ","PAL  SELECTED"},
	{ "ntsc","Selected NTSC Format","NTSC SELECTED"},
};

extern int eeprom_add ( int key, const char *data );
#define KEY_VMODE	160


static inline int do_vmode_select(void)
{

	int a;
	int key;
	int vmodeNum;
	int vmodeMax;

	char buf[16];
	while( 1 )
	{
		puts( "\nVideo Mode selection menu\n" );
		puts( "Use up, down, left, right to select, OK to save, or power to reboot\n" );
		puts( "-------------------------------------------------------------------\n" );

		for( a=0; a < numVmodes; a++ )
		{
			printf( "%2.2d) %s - %s \r\n", a+1, vmodeList[a].video_mode,vmodeList[a].descr);
		}

		vmodeMax = a-1;

		vmodeNum = 0;
		puts( "\nSelected Mode  :\n");
		puts("                          " );
		while( 1 )
		{
			if( vmodeNum < 0 )
				vmodeNum = 0;
			if( vmodeNum > vmodeMax )
				vmodeNum = vmodeMax;

			sprintf( buf, "%s", vmodeList[vmodeNum].vfdescr);
			front_puts( buf );

			for (a=0;a<20;a++)
				puts( "\b");

			printf( "%s",vmodeList[vmodeNum].descr);
			key = front_singlekey( 0 );
			if( key == key_ok || key == key_front_ok )
				break;
			if(
					(key == key_down) ||
					(key == key_front_down) ||
					(key == key_left) ||
					key == key_front_left
			  )
			{
				vmodeNum --;
				continue;
			}
			if(
					key == key_up ||
					key == key_front_up ||
					key == key_right ||
					key == key_front_right
			  )
			{
				vmodeNum ++;
				continue;
			}
			if( key_0 <= key && key <= key_9 )
			{
				vmodeNum = key-key_0-1;
				continue;
			}
			if( key == key_power || key == key_front_power )
				return 0;

		}

	    eeprom_add ( KEY_VMODE, vmodeList[vmodeNum].video_mode );
		front_puts( "Saved" );
		break;

	}
	return 0;

}


int vmode_select_cmd( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] )
{
	return do_vmode_select( );
}

U_BOOT_CMD(
		vmode_select, 14, 0, vmode_select_cmd,
		"vmode_select - Menuscreen to select the Video mode\n",
		"video_mode Selector" );
