/*
 * cramfs.h
 *
 * Copyright (C) 2002 Bastian Blank <waldi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: cramfs.h,v 1.1 2002/12/22 17:38:55 bastian Exp $
 */

#ifndef CRAMFS_H
#define CRAMFS_H

#include <cmd_fs.h>

int cramfs_load (char *loadoffset, struct part_info *info, char *filename);
int cramfs_ls (struct part_info *info, char *filename);
int cramfs_info (struct part_info *info);

#endif
