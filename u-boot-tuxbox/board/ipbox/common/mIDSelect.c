#include <common.h>
#include <command.h>
#include "common/front.h"
#include "common/upgrade.h"
#include "common/flash_img_info.h"


#define numMids 16

static struct
{
	char *mID;
	char *descr;
	char *vfdescr;
	char *segdescr;

}  midList[numMids] =
{
	{ "01", "Dbox2      Nokia    ","DBOX2 Nokia  ","M 01"},
	{ "02", "DBox2      Philips  ","DBOX2 PHILIPS","M 02"},
	{ "03", "DBox2      Sagem    ","DBOX2 SAGEM  ","M 03"},
	{ "04", "Free                ","FREE NOT USED","FREE"},
	{ "05", "Dreambox   DM7000   ","DMM DB7000   ","M 05"},
	{ "06", "Dreambox   DM5620   ","DMM DB5620   ","M 06"},
	{ "07", "Dreambox   DM500    ","DMM DB500    ","M 07"},
	{ "08", "Triax      DVB272S  ","TRIAX DVB272S","M 08"},
	{ "09", "Dreambox   DM7020   ","DMM DB7020   ","M 09"},
	{ "10", "ITgate     TGS100   ","ITGATE TGS100","M 10"},
	{ "20", "DGS CubeCaFe 200    ","DGS CUBE 200 ","M 20"},
	{ "25", "DGS CubeCaFe 250    ","DGS CUBE 250 ","M 25"},
	{ "26", "DGS CubeCaFe Prime  ","DGS CUBEPRIME","M 26"},
	{ "40", "DGS CubeCaFe 400    ","DGS CUBE 400 ","M 40"},
	{ "50", "DGS Relook R510S    ","DGS RELOOK510","M 50"},
	{ "\0", "\0","\0","\0"},
};

extern int eeprom_add ( int key, const char *data );
#define KEY_MID	161


static inline int do_midselect(void)
{
	int a;
	int key;
	int mIDnum;
	int mIDmax;

	char buf[16];
	while( 1 )
	{
		puts( "\nModel ID selection menu\n" );
		puts( "Use up, down, left, right to select, OK to save, or power to reboot\n" );
		puts( "-------------------------------------------------------------------\n" );

		for( a=0; a < numMids; a++ )
		{
			printf( "%2.2d) %s - %s \r\n", a+1, midList[a].mID,midList[a].descr);
		}

		mIDmax = a-1;

		mIDnum = 0;
		puts( "\nSelected Model :\n");
		puts("                          " );
		while( 1 )
		{
			if( mIDnum < 0 )
				mIDnum = 0;
			if( mIDnum > mIDmax )
				mIDnum = mIDmax;
#if defined(CONFIG_RELOOK400S)
			sprintf( buf, "%s", midList[mIDnum].vfdescr);  //400
#else
			sprintf( buf, "%s", midList[mIDnum].segdescr);   //200
#endif
			front_puts( buf );

			for (a=0;a<25;a++)
				puts( "\b");

			printf( "[%s] %s",midList[mIDnum].mID,midList[mIDnum].descr);
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
				mIDnum --;
				continue;
			}
			if(
					key == key_up ||
					key == key_front_up ||
					key == key_right ||
					key == key_front_right
			  )
			{
				mIDnum ++;
				continue;
			}
			if( key_0 <= key && key <= key_9 )
			{
				mIDnum = key-key_0-1;
				continue;
			}
			if( key == key_power || key == key_front_power )
				return 0;

		}

	    eeprom_add ( KEY_MID, midList[mIDnum].mID );
		front_puts( "Saved" );
		break;

	}
	return 0;
}


int do_midselect_cmd( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] )
{
	return do_midselect( );
}

U_BOOT_CMD(
		mid_select, 14, 0, do_midselect_cmd,
		"mid_select - Menuscreen to select the Model ID\n",
		"mID Selector" );
