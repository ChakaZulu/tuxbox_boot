/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * dbox2 debug mode support: switch on or off, get status
 */
#ifndef	_CMD_DEBUGMODE_H
#define	_CMD_DEBUGMODE_H

#if (CONFIG_COMMANDS & CFG_CMD_DEBUGMODE)
#define CMD_TBL_DEBUGMODE MK_CMD_TBL_ENTRY(					\
	"debugmode",	9,	CFG_MAXARGS,	1,	do_debugmode,		\
	"debugmode - enable or disable debug mode\n",				\
	"[on, off]\n"								\
	"    - enable or disable debug mode\n"					\
),

void do_debugmode (cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[]);
#else
#define CMD_TBL_DEBUGMODE
#warn NO DEBUG MODE OPTION SELECTED
#endif	/* CFG_CMD_DEBUGMODE */

#endif	/* _CMD_DEBUGMODE_H */
