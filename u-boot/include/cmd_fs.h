/*
 * (C) Copyright 2000
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
 * fs support
 */
#ifndef	_CMD_FS_H
#define	_CMD_FS_H

#if (CONFIG_COMMANDS & CFG_CMD_FS)

#include <command.h>

#define CMD_TBL_FS_FSLOAD	MK_CMD_TBL_ENTRY(			\
	"fsload",	5,	3,	0,	do_fs_fsload,		\
	"fsload  - load binary file from a filesystem image\n",		\
	"[ off ] [ filename ]\n"					\
	"    - load binary file from flash bank\n"			\
	"      with offset 'off'\n"					\
	),

#define CMD_TBL_FS_FSINFO	MK_CMD_TBL_ENTRY(			\
	"fsinfo",	5,	1,	1,	do_fs_fsinfo,		\
	"fsinfo  - print information about filesystems\n",		\
	"    - print information about filesystems\n"			\
	),

#define CMD_TBL_FS_LS		MK_CMD_TBL_ENTRY(			\
	"ls",		2,	2,	1,	do_fs_ls,		\
	"ls      - list files in a directory (default /)\n",		\
	"[ directory ]\n"						\
	"    - list files in a directory.\n"				\
	),

#define MK_FS_PART_TBL_ENTRY(type,offset,size)				\
	{ type, offset, size }

typedef struct part_info
{
	int type;
	unsigned long offset;
	unsigned long size;
	void *jffs2_priv;
} part_info_t;

int do_fs_fsload (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[]);
int do_fs_fsinfo (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[]);
int do_fs_ls (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[]);

int fs_fsload (unsigned long offset, char *filename);

extern part_info_t part_info[];
#else
#define CMD_TBL_FS_FSLOAD
#define CMD_TBL_FS_FSINFO
#define CMD_TBL_FS_LS
#endif	/* CFG_CMD_FS */

#endif	/* _CMD_FS_H */
