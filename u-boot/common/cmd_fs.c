/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2002 Bastian Blank <waldi@tuxbox.org>
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <cmd_fs.h>
#include <cmd_boot.h>
#include <cmd_autoscript.h>
#include <s_record.h>
#include <net.h>

#if (CONFIG_COMMANDS & CFG_CMD_FS)

#if (CONFIG_FS & CFG_FS_CRAMFS)
#include <cramfs.h>
#endif
#if (CONFIG_FS & CFG_FS_JFFS2)
#include <jffs2/jffs2.h>
#endif

static part_info_t part_info[] =
{
#ifdef CFG_FS_PART0_TYPE
	MK_FS_PART_TBL_ENTRY(CFG_FS_PART0_TYPE,CFG_FS_PART0_OFFSET,CFG_FS_PART0_SIZE),
#else
	{ 0, 0, 0 },
#endif
#ifdef CFG_FS_PART1_TYPE
	MK_FS_PART_TBL_ENTRY(CFG_FS_PART1_TYPE,CFG_FS_PART1_OFFSET,CFG_FS_PART1_SIZE),
#else
	{ 0, 0, 0 },
#endif
#ifdef CFG_FS_PART2_TYPE
	MK_FS_PART_TBL_ENTRY(CFG_FS_PART2_TYPE,CFG_FS_PART2_OFFSET,CFG_FS_PART2_SIZE),
#else
	{ 0, 0, 0 },
#endif
#ifdef CFG_FS_PART3_TYPE
	MK_FS_PART_TBL_ENTRY(CFG_FS_PART3_TYPE,CFG_FS_PART3_OFFSET,CFG_FS_PART3_SIZE),
#else
	{ 0, 0, 0 },
#endif
};

int do_fs_fsload (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	part_info_t *part = part_info;
	char *filename = "vmlinuz";
	ulong offset = CFG_LOAD_ADDR;
	int size, i;

	if (argc >= 2)
		offset = simple_strtoul(argv[1], NULL, 16);
	if (argc == 3)
	{
		if (argv[2][0] && argv[2][1] == ':')
		{
			i = argv[2][0] - '0';
			if (i > 4 || part_info[i].type == 0)
			{
				printf ("### FS partition %d invalid\n", i);
				return 1;
			}
			part = &part_info[i];
			filename = &argv[2][2];
		}
		else
			filename = argv[2];
	}

	switch (part -> type)
	{
#if (CONFIG_FS & CFG_FS_JFFS2)
		case CFG_FS_JFFS2:
			printf ("### FS (jffs2) loading '%s' to 0x%lx\n", filename, offset);
			size = jffs2_1pass_load ((char *) offset, part, filename);
			break;
#endif
#if (CONFIG_FS & CFG_FS_CRAMFS)
		case CFG_FS_CRAMFS:
			printf ("### FS (cramfs) loading '%s' to 0x%lx\n", filename, offset);
			size = cramfs_load ((char *) offset, part, filename);
			break;
#endif
		default:
			printf ("### FS unsupported (%d)\n", part -> type );
			return 0;
	}

	if (size > 0)
		printf ("### FS load compleate: %d bytes loaded to 0x%lx\n", size, offset);
	else
		printf ("### FS LOAD ERROR<%d> for %s!\n", size, filename);

	return !(size > 0);
}

int do_fs_ls (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	part_info_t *part = part_info;
	char *filename = "/";
	int ret;

	if (argc == 2)
	{
		if (argv[1][0] && argv[1][1] == ':')
		{
			ret = argv[1][0] - '0';
			if (ret > 4 || part_info[ret].type == 0)
			{
				printf ("### FS partition %d invalid\n", ret);
				return 1;
			}
			part = &part_info[ret];
			filename = &argv[1][2];
		}
		else
			filename = argv[1];
	}

	switch (part -> type)
	{
#if (CONFIG_FS & CFG_FS_JFFS2)
		case CFG_FS_JFFS2:
			ret = jffs2_1pass_ls (part, filename);
			break;
#endif
#if (CONFIG_FS & CFG_FS_CRAMFS)
		case CFG_FS_CRAMFS:
			ret = cramfs_ls (part, filename);
			break;
#endif
		default:
			printf ("### FS unsupported (%d)\n", part -> type );
			return 0;
	}

	return (ret == 1);
}

static void fsinfo (int number)
{
	printf ("Partition # %d: ", number);
	switch (part_info[number].type)
	{
		case CFG_FS_JFFS2:
			puts ("jffs2");
#if !(CONFIG_FS & CFG_FS_JFFS2)
			puts (" (unsupported)");
#endif
			break;
		case CFG_FS_CRAMFS:
			puts ("cramfs");
#if !(CONFIG_FS & CFG_FS_CRAMFS)
			puts (" (unsupported)");
#endif
			break;
		default:
			puts ("unsupported");
	}

	printf (", offset: 0x%x, size 0x%x\n", part_info[number].offset, part_info[number].size);
}

int do_fs_fsinfo(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	struct part_info *part = NULL;
	int ret = 1;

	if (argc == 2)
		if (argv[1][0])
		{
			ret = argv[1][0] - '0';
			if (ret > 4 || part_info[ret].type == 0)
			{
				printf ("### FS partition %d invalid\n", ret);
				return 1;
			}
			part = &part_info[ret];
		}

	if (part)
		switch (part -> type)
		{
#if (CONFIG_FS & CFG_FS_JFFS2)
			case CFG_FS_JFFS2:
				ret = jffs2_1pass_info (part);
				break;
#endif
#if (CONFIG_FS & CFG_FS_CRAMFS)
			case CFG_FS_CRAMFS:
				ret = cramfs_info (part);
				break;
#endif
			default:
				printf ("### FS unsupported (%d)\n", part -> type );
				return 0;
		}
	else
	{
		fsinfo (0);
		fsinfo (1);
		fsinfo (2);
		fsinfo (3);
	}

	return (ret == 1);
}

int fs_fsload (unsigned long offset, char *filename)
{
	part_info_t *part = part_info;
	int i, size;
	char *temp;

	if (filename[0] && filename[1] == ':')
	{
		i = filename[0] - '0';
		if (i > 4 || part_info[i].type == 0)
			return -2;
		part = &part_info[i];
		temp = &filename[2];
	}
	else
		temp = filename;

	switch (part -> type)
	{
#if (CONFIG_FS & CFG_FS_JFFS2)
		case CFG_FS_JFFS2:
			size = jffs2_1pass_load ((char *) offset, part, temp);
			break;
#endif
#if (CONFIG_FS & CFG_FS_CRAMFS)
		case CFG_FS_CRAMFS:
			size = cramfs_load ((char *) offset, part, temp);
			break;
#endif
		default:
			return -3;
	}

	return size;
}

#endif /* CFG_CMD_FS */
