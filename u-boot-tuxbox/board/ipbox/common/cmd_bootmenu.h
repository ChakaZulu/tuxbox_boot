#ifndef _CMD_BOOTMENU_H
#define _CMD_BOOTMENU_H

#include "common/front.h"

typedef struct
{
	enum front_key pattern[3];
	int repeat;
	char *cmd;
} dgs_bootmenu_t;

extern dgs_bootmenu_t dgs_bootmenu_cmds[];

#endif
