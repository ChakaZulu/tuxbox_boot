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

struct product {
	char mid;
	char ver[3];
	int *addr;
} __attribute__ ((packed));

static struct product models[7] = {
	{ 1, "1.0", (int *) 0x10000944 },
	{ 1, "1.2", (int *) 0x1000095c },
	{ 2, "1.0", (int *) 0x1000091c },
	{ 3, "1.0", (int *) 0x10000a00 },
	{ 3, "1.1", (int *) 0x10000904 },
	{ 3, "1.2", (int *) 0x10000904 },
	{ 3, "1.3", (int *) 0x100008d8 },
};

static char *manufacturers[3] = {
	"nokia",
	"philips",
	"sagem"
};

static int bmon_version(unsigned char *ver) {

	char *bmon = (char *) 0x10000000;
	char *p;

	for (p = bmon; p < bmon + 131072; p++)
		if ((p - bmon + 11 <= 131072) && (!memcmp(p, "dbox2:", 6))) {
			memcpy(ver, &p[8], 3);
			return 0;
		}

	return -1;
}


static int *product_ptr(void)
{
	char mid = ((char *) 0x1001ffe0)[0];
	char ver[3];
	int i;

	if (bmon_version(ver) < 0) {
		printf("unable to detect bmon version\n");
		return NULL;
	}

	for (i = 0; i < 7; i++) {
		if ((models[i].mid == mid) && (!memcmp(models[i].ver, ver, 3))) {
			printf("%s bmon %c%c%c\n", manufacturers[mid - 1], ver[0], ver[1], ver[2]);
			return models[i].addr;
		}
	}

	printf("unknown %s bmon version %c%c%c. please report!\n", manufacturers[mid - 1], ver[0], ver[1], ver[2]);
	return NULL;
}

static void debugmode_status(int *product)
{
	switch (product[0]) {
	case 0:
		printf("debug mode is enabled\n");
		break;
	case -1:
		printf("debug mode is disabled\n");
		break;
	default:
		printf("internal error or misconfigured bootloader\n");
		break;
	}
}

static void debugmode_enable(int *product)
{
	printf("+++ debugmode_enable\n");
	//product[0] = 0;
}

static void debugmode_disable(int *product)
{
	printf("+++ debugmode_disable\n");
	//product[0] = -1;
}

static int on_off(const char *s)
{
	if (!strcmp(s, "on"))
		return 1;
	else if (!strcmp(s, "off"))
		return 0;
	return -1;
}

void do_debugmode(cmd_tbl_t *cmdtp, bd_t *bd, int flag, int argc, char *argv[])
{
	int *product = product_ptr();

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
