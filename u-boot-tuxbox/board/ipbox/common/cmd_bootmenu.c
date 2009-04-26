#include <common.h>

#ifdef CONFIG_DGS_BOOTMENU
#ifndef CONFIG_DGS_FRONT
#error "CONFIG_DGS_FRONT is needed with CONFIG_DGS_BOOTMENU."
#endif

#include <command.h>
#include <asm/processor.h>
#ifdef CFG_HUSH_PARSER
#include <hush.h>
#endif

#include "common/front.h"
#include "common/cmd_bootmenu.h"

//#define DEBUG

#ifdef DEBUG
#define fdebug(fmt,arg...) printf(fmt,##arg)
#else
#define fdebug(fmt,arg...) do{}while(0)
#endif

#define MAX_KEY	60

static int dgs_bootmenu( int timeout )	/* timeout in ms */
{
	int i, j;
	int presskey[MAX_KEY];
	int key;
	char *cmd;
	char *s;
	long long start_tick;

	memset( presskey, 0, sizeof(presskey) );

	for( i=0; dgs_bootmenu_cmds[i].cmd!=NULL; i++ )
		if( dgs_bootmenu_cmds[i].repeat > MAX_KEY )
			dgs_bootmenu_cmds[i].repeat = MAX_KEY;

	start_tick = get_ticks();

	while( start_tick + get_tbclk()/1000*timeout > get_ticks() )
	{
		if( tstc() )
		{
			if(getc()==0x1b) /* esc to enter boot debug mode */
			{
				cmd = "menu_serialbreak";
				goto exec_cmd;
			}
		}

		if( front_tstc() )
		{
			key = front_getkey();

			if( key != key_null )
			{
				fdebug( "get front key %d\n", key );
				for( i=MAX_KEY-1; i>0; i-- )
					presskey[i] = presskey[i-1];
				presskey[0] = key;

				for( i=0; dgs_bootmenu_cmds[i].cmd!=NULL; i++ )
				{
					if( dgs_bootmenu_cmds[i].repeat )
					{
						for( j=0; j<dgs_bootmenu_cmds[i].repeat; j++ )
							if( presskey[j] != dgs_bootmenu_cmds[i].pattern[0] )
								break;

						if( j == dgs_bootmenu_cmds[i].repeat )
						{
							fdebug( "got cmd %d repeat %d\n",
									i, dgs_bootmenu_cmds[i].repeat );
							cmd = dgs_bootmenu_cmds[i].cmd;
							goto exec_cmd;
						}
					}
					else
					{
						for( j=0; dgs_bootmenu_cmds[i].pattern[j] != key_null; j++ )
							if( presskey[j] != dgs_bootmenu_cmds[i].pattern[j] )
								break;

						if( dgs_bootmenu_cmds[i].pattern[j] == key_null )
						{
							fdebug( "got cmd %d no repeat\n", i );
							cmd = dgs_bootmenu_cmds[i].cmd;
							goto exec_cmd;
						}
					}
				}

				start_tick = get_ticks();
			}
		}
	}

	cmd = "menu_timeout";

exec_cmd:
	s = getenv( cmd );
	if( s )
	{
		fdebug( "excute \"%s\"\n", s );

# ifndef CFG_HUSH_PARSER
		run_command (s, 0);
# else
		parse_string_outer(s, FLAG_PARSE_SEMICOLON |
				    FLAG_EXIT_FROM_LOOP);
# endif
	}
	else
		printf( "no command.(%s)\n", cmd );

	return 0;
}

static int do_bootmenu( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] )
{
	int timeout = 500;

	if( argc > 1 )
		timeout = simple_strtoul( argv[1], NULL, 10 );

	return dgs_bootmenu(timeout);
}

U_BOOT_CMD(
		bootmenu, 12, 0, do_bootmenu,
		"bootmenu- check boot key.\n",
		"bootmenu timeout(ms)\n"
		" check user input and excute the menu.\n"
		);

#endif
