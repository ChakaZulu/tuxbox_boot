/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001 Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Andreas Heppel <aheppel@sysgo.de>

 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* #define DEBUG */

#include <common.h>

#if defined(CFG_ENV_IS_IN_FLASH) /* Environment is in Flash */

#include <command.h>
#include <environment.h>
#include <cmd_nvedit.h>
#include <linux/stddef.h>

#if ((CONFIG_COMMANDS&(CFG_CMD_ENV|CFG_CMD_FLASH)) == (CFG_CMD_ENV|CFG_CMD_FLASH))
#define CMD_SAVEENV
#elif defined(CFG_ENV_ADDR_REDUND)
#error Cannot use CFG_ENV_ADDR_REDUND without CFG_CMD_ENV & CFG_CMD_FLASH
#endif

#if defined(CFG_ENV_SECT_SIZE) && (CFG_ENV_SECT_SIZE > CFG_ENV_SIZE) && \
    defined(CFG_ENV_ADDR_REDUND)
#error CFG_ENV_ADDR_REDUND should not be used when CFG_ENV_SECT_SIZE > CFG_ENV_SIZE
#endif

#if defined(CFG_ENV_SIZE_REDUND) && (CFG_ENV_SIZE_REDUND < CFG_ENV_SIZE)
#error CFG_ENV_SIZE_REDUND should not be less then CFG_ENV_SIZE
#endif

#ifdef CONFIG_INFERNO
# ifdef CFG_ENV_ADDR_REDUND
#error CFG_ENV_ADDR_REDUND is not implemented for CONFIG_INFERNO
# endif
#endif

char * env_name_spec = "Flash";

#ifdef ENV_IS_EMBEDDED

extern uchar environment[];
env_t *env_ptr = (env_t *)(&environment[0]);

#ifdef CMD_SAVEENV
/* static env_t *flash_addr = (env_t *)(&environment[0]);-broken on ARM-wd-*/
static env_t *flash_addr = (env_t *)CFG_ENV_ADDR;
#endif

#else /* ! ENV_IS_EMBEDDED */

env_t *env_ptr = (env_t *)CFG_ENV_ADDR;
#ifdef CMD_SAVEENV
static env_t *flash_addr = (env_t *)CFG_ENV_ADDR;
#endif

#endif /* ENV_IS_EMBEDDED */

#ifdef CFG_ENV_ADDR_REDUND
static env_t *flash_addr_new = (env_t *)CFG_ENV_ADDR_REDUND;

static ulong end_addr = CFG_ENV_ADDR + CFG_ENV_SIZE - 1;
static ulong end_addr_new = CFG_ENV_ADDR_REDUND + CFG_ENV_SIZE_REDUND - 1;

static uchar active_flag = 1;
static uchar obsolete_flag = 0;
#endif

extern uchar default_environment[];
extern int default_environment_size;


uchar env_get_char_spec (int index)
{
	DECLARE_GLOBAL_DATA_PTR;

	return ( *((uchar *)(gd->env_addr + index)) );
}

#ifdef CFG_ENV_ADDR_REDUND

int  env_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	int crc1_ok =
		(crc32(0, flash_addr->data, ENV_SIZE) == flash_addr->crc);
	int crc2_ok =
		(crc32(0, flash_addr_new->data, ENV_SIZE) == flash_addr_new->crc);

	uchar flag1 = flash_addr->flags;
	uchar flag2 = flash_addr_new->flags;

	ulong addr_default = (ulong)&default_environment[0];
	ulong addr1 = (ulong)&(flash_addr->data);
	ulong addr2 = (ulong)&(flash_addr_new->data);

	if (crc1_ok && ! crc2_ok)
	{
		gd->env_addr  = addr1;
		gd->env_valid = 1;
	}
	else if (! crc1_ok && crc2_ok)
	{
		gd->env_addr  = addr2;
		gd->env_valid = 1;
	}
	else if (! crc1_ok && ! crc2_ok)
	{
		gd->env_addr  = addr_default;
		gd->env_valid = 0;
	}
	else if (flag1 == active_flag && flag2 == obsolete_flag)
	{
		gd->env_addr  = addr1;
		gd->env_valid = 1;
	}
	else if (flag1 == obsolete_flag && flag2 == active_flag)
	{
		gd->env_addr  = addr2;
		gd->env_valid = 1;
	}
	else if (flag1 == flag2)
	{
		gd->env_addr  = addr1;
		gd->env_valid = 2;
	}
	else if (flag1 == 0xFF)
	{
		gd->env_addr  = addr1;
		gd->env_valid = 2;
	}
	else if (flag2 == 0xFF)
	{
		gd->env_addr  = addr2;
		gd->env_valid = 2;
	}

	return (0);
}

#ifdef CMD_SAVEENV
int saveenv(void)
{
	int rc = 1;

	debug ("Protect off %08lX ... %08lX\n",
		(ulong)flash_addr, end_addr);

	if (flash_sect_protect (0, (ulong)flash_addr, end_addr)) {
		goto Done;
	}

	debug ("Protect off %08lX ... %08lX\n",
		(ulong)flash_addr_new, end_addr_new);

	if (flash_sect_protect (0, (ulong)flash_addr_new, end_addr_new)) {
		goto Done;
	}

	puts ("Erasing Flash...");
	debug (" %08lX ... %08lX ...",
		(ulong)flash_addr_new, end_addr_new);

	if (flash_sect_erase ((ulong)flash_addr_new, end_addr_new)) {
		goto Done;
	}

	puts ("Writing to Flash... ");
	debug (" %08lX ... %08lX ...",
		(ulong)&(flash_addr_new->data),
		sizeof(env_ptr->data)+(ulong)&(flash_addr_new->data));
	if (flash_write(env_ptr->data,
	                (ulong)&(flash_addr_new->data),
			sizeof(env_ptr->data)) ||

	    flash_write((char *)&(env_ptr->crc),
	                (ulong)&(flash_addr_new->crc),
			sizeof(env_ptr->crc)) ||

	    flash_write((char *)&obsolete_flag,
	                (ulong)&(flash_addr->flags),
			sizeof(flash_addr->flags)) ||

	    flash_write((char *)&active_flag,
	                (ulong)&(flash_addr_new->flags),
			sizeof(flash_addr_new->flags)))
	{
		flash_perror (rc);
		goto Done;
	}
	puts ("done\n");

	{
		env_t * etmp = flash_addr;
		ulong ltmp = end_addr;

		flash_addr = flash_addr_new;
		flash_addr_new = etmp;

		end_addr = end_addr_new;
		end_addr_new = ltmp;
	}

	rc = 0;
Done:

	/* try to re-protect */
	(void) flash_sect_protect (1, (ulong)flash_addr, end_addr);
	(void) flash_sect_protect (1, (ulong)flash_addr_new, end_addr_new);

	return rc;
}
#endif /* CMD_SAVEENV */

#else /* ! CFG_ENV_ADDR_REDUND */

int  env_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	if (crc32(0, env_ptr->data, ENV_SIZE) == env_ptr->crc) {
		gd->env_addr  = (ulong)&(env_ptr->data);
		gd->env_valid = 1;
	} else {
		gd->env_addr  = (ulong)&default_environment[0];
		gd->env_valid = 0;
	}

	return (0);
}

#ifdef CMD_SAVEENV

int saveenv(void)
{
	int	len, rc;
	ulong	end_addr;
	ulong	flash_sect_addr;
#if defined(CFG_ENV_SECT_SIZE) && (CFG_ENV_SECT_SIZE > CFG_ENV_SIZE)
	ulong	flash_offset;
	uchar	env_buffer[CFG_ENV_SECT_SIZE];
#else
	uchar *env_buffer = (char *)env_ptr;
#endif	/* CFG_ENV_SECT_SIZE */
	int rcode = 0;

#if defined(CFG_ENV_SECT_SIZE) && (CFG_ENV_SECT_SIZE > CFG_ENV_SIZE)

	flash_offset    = ((ulong)flash_addr) & (CFG_ENV_SECT_SIZE-1);
	flash_sect_addr = ((ulong)flash_addr) & ~(CFG_ENV_SECT_SIZE-1);

	debug ( "copy old content: "
		"sect_addr: %08lX  env_addr: %08lX  offset: %08lX\n",
		flash_sect_addr, (ulong)flash_addr, flash_offset);

	/* copy old contents to temporary buffer */
	memcpy (env_buffer, (void *)flash_sect_addr, CFG_ENV_SECT_SIZE);

	/* copy current environment to temporary buffer */
	memcpy ((uchar *)((unsigned long)env_buffer + flash_offset),
		env_ptr,
		CFG_ENV_SIZE);

	len	 = CFG_ENV_SECT_SIZE;
#else
	flash_sect_addr = (ulong)flash_addr;
	len	 = CFG_ENV_SIZE;
#endif	/* CFG_ENV_SECT_SIZE */

#ifndef CONFIG_INFERNO
	end_addr = flash_sect_addr + len - 1;
#else
	/* this is the last sector, and the size is hardcoded here */
	/* otherwise we will get stack problems on loading 128 KB environment */
	end_addr = flash_sect_addr + 0x20000 - 1;
#endif

	debug ("Protect off %08lX ... %08lX\n",
		(ulong)flash_sect_addr, end_addr);

	if (flash_sect_protect (0, flash_sect_addr, end_addr))
		return 1;

	puts ("Erasing Flash...");
	if (flash_sect_erase (flash_sect_addr, end_addr))
		return 1;

	puts ("Writing to Flash... ");
	rc = flash_write(env_buffer, flash_sect_addr, len);
	if (rc != 0) {
		flash_perror (rc);
		rcode = 1;
	} else {
		puts ("done\n");
	}

	/* try to re-protect */
	(void) flash_sect_protect (1, flash_sect_addr, end_addr);
	return rcode;
}

#endif /* CMD_SAVEENV */

#endif /* CFG_ENV_ADDR_REDUND */

void env_relocate_spec (void)
{
#if !defined(ENV_IS_EMBEDDED) || defined(CFG_ENV_ADDR_REDUND)
#ifdef CFG_ENV_ADDR_REDUND
	DECLARE_GLOBAL_DATA_PTR;

	if (gd->env_addr != (ulong)&(flash_addr->data))
	{
		env_t * etmp = flash_addr;
		ulong ltmp = end_addr;

		flash_addr = flash_addr_new;
		flash_addr_new = etmp;

		end_addr = end_addr_new;
		end_addr_new = ltmp;
	}

	if (flash_addr_new->flags != obsolete_flag &&
	    crc32(0, flash_addr_new->data, ENV_SIZE) ==
	    flash_addr_new->crc)
	{
		gd->env_valid = 2;
		flash_sect_protect (0, (ulong)flash_addr_new, end_addr_new);
		flash_write((char *)&obsolete_flag,
                    	    (ulong)&(flash_addr_new->flags),
	        	    sizeof(flash_addr_new->flags));
		flash_sect_protect (1, (ulong)flash_addr_new, end_addr_new);
	}

	if (flash_addr->flags != active_flag &&
	    (flash_addr->flags & active_flag) == active_flag)
	{
		gd->env_valid = 2;
		flash_sect_protect (0, (ulong)flash_addr, end_addr);
		flash_write((char *)&active_flag,
                    	    (ulong)&(flash_addr->flags),
	        	    sizeof(flash_addr->flags));
		flash_sect_protect (1, (ulong)flash_addr, end_addr);
	}

	if (gd->env_valid == 2)
		puts ("*** Warning - some problems detected "
		      "reading environment; recovered successfully\n\n");
#endif /* CFG_ENV_ADDR_REDUND */
	memcpy (env_ptr, (void*)flash_addr, CFG_ENV_SIZE);
#endif /* ! ENV_IS_EMBEDDED || CFG_ENV_ADDR_REDUND */
}

#endif /* CFG_ENV_IS_IN_FLASH) */
