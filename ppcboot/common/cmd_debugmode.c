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
#include <ppcboot.h>
#include <command.h>
#include <cmd_debugmode.h>

#if (CONFIG_COMMANDS & CFG_CMD_DEBUGMODE)

static unsigned long *product_ptr(void)
{
	switch (((char*)0x1001ffe0)[0]) {
	case 1:
		if (!strncmp((char*)0x100146c8, "1.0", 3)) {
			printf("nokia bmon 1.0\n");
			return (unsigned long*)0x10000944;
		}
		if (!strncmp((char*)0x10014a88, "1.2", 3)) {
			printf("nokia bmon 1.2\n");
			return (unsigned long*)0x1000095c;
		}
		break;

	case 2:
		if (!strncmp(((char*)0x100152a4), "1.0", 3)) {
			printf("philips bmon 1.0\n");
			return (unsigned long*)0x1000091c;
		}
		break;

	case 3:
#if 0
		if (!strncmp((char*)0x10015418, "1.0", 3)) { /* FIXME: wrong address */
			printf("sagem bmon 1.0\n");
			return (unsigned long*)0x10000a00;
		}
#endif
		if (!strncmp((char*)0x10015418, "1.1", 3)) {
			printf("sagem bmon 1.1\n");
			return (unsigned long*)0x10000904;
		}
		if (!strncmp((char*)0x10015418, "1.2", 3)) {
			printf("sagem bmon 1.2\n");
			return (unsigned long*)0x10000904;
		}
#if 0
		if (!strncmp((char*)0x10015418, "1.3", 3)) { /* FIXME: wrong address */
			printf("sagem bmon 1.0\n");
			return (unsigned long*)0x100008d8;
		}
#endif
		break;

	default:
		break;
	}
	
	printf("unknown bootloader, please report!\n");
	return NULL;
}

static void debugmode_status(unsigned long *product)
{
	switch(product[0]) {
	case 0x00000000:
		printf("debug mode is enabled\n");
		break;
	case 0xffffffff:
		printf("debug mode is disabled\n");
		break;
	default:
		printf("internal error or misconfigured bootloader\n");
		break;
	}
}

static void debugmode_enable(unsigned long *product)
{
	printf("+++ debugmode_enable\n");
	//product[0] = 0x00000000;
}

static void debugmode_disable(unsigned long *product)
{
	printf("+++ debugmode_disable\n");
	//product[0] = 0xffffffff;
}

static int on_off(const char *s)
{
	if (strcmp(s, "on") == 0) {
		return (1);
	} else if (strcmp(s, "off") == 0) {
		return (0);
	}
	return (-1);
}

void do_debugmode(cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	unsigned long *product = product_ptr();

	if (!product) {
		printf("could not identify product\n");
		return;
	}

	switch (argc) {
	case 2: /* on / off */
		switch (on_off(argv[1])) {
		case 0:	debugmode_disable(product);
			break;
		case 1:	debugmode_enable(product);
			break;
		default:
			break;
		}
		/* FALL TROUGH */

	case 1: /* get status */
		debugmode_status(product);
		break;

	default:
		printf("Usage:\n%s\n", cmdtp->usage);
		break;
	}
}

#endif	/* CFG_CMD_DEBUGMODE */
