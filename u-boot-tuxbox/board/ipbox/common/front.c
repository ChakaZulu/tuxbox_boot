#include <common.h>

#ifdef CONFIG_DGS_FRONT

#include <command.h>

#include "common/front.h"

//#define DEBUG

#ifdef DEBUG
#define fdebug(fmt,arg...) printf(fmt,##arg)
#else
#define fdebug(fmt,arg...) do{}while(0)
#endif

int front_waitkey( int *keys, int timeout )
{
	unsigned long long end;
	int key;
	int a;

	end = get_ticks() + timeout*get_tbclk();

	while( 1 )
	{
		if( timeout && get_ticks() > end )
			break;

		key = front_getkey();
		if( key == key_null )
			continue;

		for( a=0; keys[a]!=key_null; a++ )
		{
			if( key == keys[a] )
			{
				while( 1 )
				{
					if( key == key_release || key == key_front_release )
						return keys[a];
					key = front_getkey();
				}
			}
		}
	}

	return -1;
}

int front_singlekey( int timeout )
{
	unsigned long long end;
	int key, oldkey = key_null;

	end = get_ticks() + timeout*get_tbclk();

	while( 1 )
	{
		if( timeout && get_ticks() > end )
			return key_null;

		key = front_getkey();
		if( key == key_null )
			continue;

		if( oldkey != key_null && ( key == key_release || key == key_front_release ) )
			return oldkey;

		oldkey = key;
	}
}

int do_front_puts( cmd_tbl_t *cmd, int flag, int argc, char *argv[] )
{
	if( argc > 1 )
		front_puts( argv[1] );
	else
		front_puts( "" );

	return 0;
}

U_BOOT_CMD(
		front_puts, 10, 0, do_front_puts,
		"front_puts- put string to front display.\n",
		"put string to front VFD.\n" );


#endif
