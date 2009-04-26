
#include <common.h>

#ifdef CONFIG_DGS_EEPROM_DB

#include <command.h>

#ifdef CFG_I2C_EEPROM_ADDR
#define CFG_EEPROM_ADD		CFG_I2C_EEPROM_ADDR
#else
#define CFG_EEPROM_ADD		0
#endif

#define DB_MAGIC		0x1234		/* magic */

typedef enum
{
	db_key_null = 0,
	db_key_end,
	db_key_ethaddr,
	db_key_prodnum,
	db_key_lch,
	db_key_vol,
	db_pcbver,
	db_key_videofmt = 160,
	db_key_mid	    = 161,
	db_key_cap	    = 162,
} db_key;
static struct
{
	db_key key;
	char *name;
} db_name_tbl[] =
{
	{ db_key_null,		"null" },
	{ db_key_end,		"end" },
	{ db_key_ethaddr,	"ethaddr" },
	{ db_key_prodnum,	"prodnum" },
	{ db_key_lch,		"lastch" },
	{ db_key_vol,		"volume" },
	{ db_pcbver,		"pcbver" },
	{ db_key_videofmt,	"videofmt" },
	{ db_key_mid,		"mID" },
	{ db_key_cap,		"capability" },
};
#define db_name_tbl_size	(sizeof(db_name_tbl)/sizeof(db_name_tbl[0]))

typedef struct
{
	unsigned short key;
	unsigned short len;
	unsigned char data[0];
} db_item;

#define DB_MAGIC_SIZE		sizeof(unsigned short)
#define DB_HEADE_SIZE		sizeof(db_item)
static int add_item( db_key key, const char *data );
static int get_keyname( db_key key, char **name )
{
	int a;

	for( a=0; a<db_name_tbl_size; a++ )
	{
		if( db_name_tbl[a].key == key )
		{
			*name = db_name_tbl[a].name;
			return 0;
		}
	}

	*name = "unknown";

	return 1;
}

static int get_keyvalue( const char *name, db_key *key )
{
	int a;

	for( a=0; a<db_name_tbl_size; a++ )
	{
		if( !strcmp(db_name_tbl[a].name,name) )
		{
			*key = db_name_tbl[a].key;
			return 0;
		}
	}

	return 1;
}

static int read_item( int offset, db_key *key, char *buf, int *buflen )
{
	int rcode;
	db_item item;

	if( offset >= CFG_EEPROM_SIZE - DB_HEADE_SIZE )
		return -1;

	rcode = eeprom_read( CFG_EEPROM_ADD, offset, (unsigned char*)&item, sizeof(item) );
	if( rcode )
		return -1;

	if( item.len > 0 && buf != NULL )
	{
		if( *buflen > item.len )
			*buflen = item.len;

		rcode = eeprom_read( CFG_EEPROM_ADD, offset + DB_HEADE_SIZE, buf, *buflen );
		if( rcode )
			return -1;

		buf[*buflen] = 0;
	}

	*key = item.key;

	return item.len;
}

static int search_item( int offset, db_key key, char *buf, int *buflen, int *offret, int *lenret )
{
	int rcode;
	int len;
	db_key keytmp;

	do
	{
		len = read_item( offset, &keytmp, NULL, NULL );
		if( len < 0 )
			return 1;

		if( keytmp == key )
		{
			/* found it */
			if( offret != NULL )
				*offret = offset;
			if( lenret != NULL )
				*lenret = len;

			if( len > 0 && buf != NULL )
			{
				if( *buflen > len )
					*buflen = len;

				rcode = eeprom_read(
						CFG_EEPROM_ADD,
						offset + DB_HEADE_SIZE,
						buf,
						*buflen );
				if( rcode )
					return 1;

				buf[*buflen] = 0;
			}

			return 0;
		}

		offset += DB_HEADE_SIZE + len;
	}
	while( keytmp != db_key_end );

	/* we coundn`t find the key.
	 * return the end key
	 */
	if( offret != NULL )
		*offret = offset - (DB_HEADE_SIZE + len);
	if( lenret != NULL )
		*lenret = len;

	return 2;
}

static int save_item( int offset, db_key key, const char *buf, int len )
{
	int rcode;
	db_item item;

	/* write item data */
	if( buf != NULL && len != 0 )
	{
		rcode = eeprom_write(
				CFG_EEPROM_ADD,
				offset+DB_HEADE_SIZE,
				(unsigned char*)buf,
				len );
		if( rcode )
			return rcode;
	}

	/* write item header */
	item.key = key;
	item.len = len;
	rcode = eeprom_write( CFG_EEPROM_ADD, offset, (unsigned char*)&item, sizeof(item) );

	return rcode;
}

static int clear_eeprom(void)
{

	int rcode;
	int offset = DB_MAGIC_SIZE;
	db_key key;
	unsigned char etherdddr [20];
	unsigned char prodNum [20];
	unsigned char buff[0x80]={0x12,0x34,0x00,0x00,0x00,0x01,0x39,0x00,0x01,0x00,0x00};
	memset(etherdddr,0,20);
	memset(prodNum,0,20);
	do
	{
		char buf[256];
		int buflen;

		buf[0] = 0;
		buflen = 255;
		rcode = read_item( offset, &key, buf, &buflen );
		if( rcode < 0 )
		{
			rcode = 1;
			return 1;
		}
		offset += DB_HEADE_SIZE + rcode;

		if (key==db_key_ethaddr)
		{
			memcpy(etherdddr,buf,buflen);
		}

		if (key==db_key_prodnum)
		{
			memcpy(prodNum,buf,buflen);
		}
	}
	while( key != db_key_end );

	eeprom_write( CFG_EEPROM_ADD, 0, buff, 0x80 );
	add_item( db_key_ethaddr, etherdddr );
	add_item( db_key_prodnum, prodNum );

	return 0;
}

static int del_item( db_key key )
{
	int offset;
	db_key keytmp;
	int len;

	int last_offset;
	db_key last_keytmp;
	int last_len;

	int next_offset;
	db_key next_keytmp;
	int next_len;

	/* find the item */
	offset = DB_MAGIC_SIZE;
	last_offset = 0;
	last_keytmp = db_key_end;
	last_len = 0;
	do
	{
		len = read_item( offset, &keytmp, NULL, NULL );
		if( len < 0 )
			return -1;

		if( keytmp == key )
		{
			/* merge with next one if it is null item */
			next_offset = offset + DB_HEADE_SIZE + len;
			next_len = read_item( next_offset, &next_keytmp, NULL, NULL );
			if( next_len >= 0 )
			{
				if( next_keytmp == db_key_null )
					len += DB_HEADE_SIZE + next_len;
			}

			/* merge with privious one if it was null item */
			if( last_keytmp == db_key_null )
			{
				offset = last_offset;
				len += last_len + DB_HEADE_SIZE;
			}

			return save_item( offset, db_key_null, NULL, len );
		}

		last_offset = offset;
		last_keytmp = keytmp;
		last_len = len;

		offset += DB_HEADE_SIZE + len;
	}
	while( key != db_key_end );

	return 0;
}

static int add_item( db_key key, const char *data )
{
	int data_len;
	int rcode;

	int offset;
	int len;

	if( data )
		data_len = strlen( data );
	else
		data_len = 0;

	/* delete same key */
	del_item( key );

	/* search enough space to store the item */
	offset = DB_MAGIC_SIZE;
	do
	{
		rcode = search_item( offset, db_key_null, NULL, NULL, &offset, &len );
		if( rcode == 1 )	/* device error */
			return 1;
		if( rcode == 2 )	/* it`s the end */
			break;

		if( len == data_len || len >= data_len + DB_HEADE_SIZE )
		{
			rcode = save_item( offset, key, data, data_len );
			if( rcode )
				return rcode;

			if( len > data_len )
				rcode = save_item(
						offset + DB_HEADE_SIZE + data_len,
						db_key_null,
						NULL,
						len - data_len - DB_HEADE_SIZE
						);

			return rcode;
		}

		offset += DB_HEADE_SIZE + len;
	}
	while( 1 );

	/* store at the end */
	rcode = save_item( offset, key, data, data_len );
	if( rcode )
	{
		save_item( offset, db_key_end, NULL, 0 );
		return 1;
	}
	else
	{
		offset += DB_HEADE_SIZE + data_len;

		rcode = save_item( offset, db_key_end, NULL, 0 );
	}

	return rcode;
}

static int do_edb( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] )
{
	int rcode;
	unsigned short magic;

	/* read magic */
	rcode = eeprom_read( CFG_EEPROM_ADD, 0, (unsigned char*)&magic, sizeof(magic) );
	if( rcode )
		goto failed;
	magic = be16_to_cpu( magic );

	if( magic != DB_MAGIC )
	{
		/* if it is not initialized */
		puts( "initialize empty eeprom db.\n" );

		/* set the final key */
		rcode = save_item( DB_MAGIC_SIZE, db_key_end, NULL, 0 );
		if( rcode )
			goto failed;

		/* initialize magic */
		magic = cpu_to_be16( DB_MAGIC );
		rcode = eeprom_write( CFG_EEPROM_ADD, 0, (unsigned char*)&magic, sizeof(magic) );
		if( rcode )
			goto failed;
	}

	if( argc > 1 )
	{
		db_key key;

		if( !strcmp(argv[1],"exec") )
		{
		}
		else if( !strcmp(argv[1],"clear") )
		{
			clear_eeprom();
			return 0;
		}
		else if( !strcmp(argv[1],"toenv") && argc > 2 )
		{
			rcode = get_keyvalue( argv[2], &key );
			if( rcode )
				puts( "unknown key.\n" );
			else
			{
				char buf[256];
				int buflen;

				buflen = 255;
				rcode = search_item( DB_MAGIC_SIZE, key, buf, &buflen, NULL, NULL );
				if( rcode == 1 )
					goto failed;
				if( rcode == 2 )
				{
					printf( "no such a key.(%s)\n", argv[2] );
					return 1;
				}

				if( buflen != 0 && key != db_key_null )
				{
					setenv( argv[2], buf );
					return 0;
				}
				else
				{
					printf( "no data on the key.(%s)\n", argv[2] );
					return 1;
				}
			}
		}
		else if( !strcmp(argv[1],"add") && argc > 2 )
		{
			rcode = get_keyvalue( argv[2], &key );
			if( rcode )
				puts( "unknown key.\n" );
			else
			{
				if( argc > 3 )
					rcode = add_item( key, argv[3] );
				else
					rcode = add_item( key, argv[3] );

				if( rcode )
					goto failed;

				return rcode;
			}
		}
		else if( !strcmp(argv[1],"del") && argc > 2 )
		{
			rcode = get_keyvalue( argv[2], &key );
			if( rcode )
				puts( "unknown key.\n" );
			else
			{
				rcode = del_item( key );

				if( rcode )
					goto failed;

				return rcode;
			}
		}

		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	else
	{
		db_key key;

		/* dump all of database */
		int offset = DB_MAGIC_SIZE;

		puts( "offset\tkey\tlen\tdata\n" );
		do
		{
			char *name;
			char buf[256];
			int buflen;

			buf[0] = 0;
			buflen = 255;
			rcode = read_item( offset, &key, buf, &buflen );
			if( rcode < 0 )
			{
				rcode = 1;
				goto failed;
			}
			get_keyname( key, &name );

			printf( "%d\t%s\t%d\t%s\n", offset, name, rcode, (key==db_key_null)?"":buf );

			offset += DB_HEADE_SIZE + rcode;
		}
		while( key != db_key_end );
	}

	return 0;

failed:
	puts( "failed.\n" );
	return rcode;
}

int eeprom_add ( int key, const char *data )
{
	return add_item( key, data );
}

U_BOOT_CMD(
		edb, 4, 0, do_edb,
		"edb     - manage database on eeprom\n",
		"add key data\n"
		"edb del key\n"
		"              - add/del database key.\n"
		"edb toenv key - set the key to environment.\n"
		"edb           - display all database key.\n"
		"edb clear     - Removes all keys except serial and etheraddr.\n"
	  );

#endif
