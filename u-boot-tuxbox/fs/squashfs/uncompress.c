	/*
 * uncompress.c
 *
 * Copyright (C) 1999 Linus Torvalds
 * Copyright (C) 2000-2002 Transmeta Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 *
 * shamelessly stolen from fs/cramfs
 */

#include <common.h>
#include <malloc.h>
#include <watchdog.h>
#include <zlib.h>

#if (CONFIG_FS & CFG_FS_SQUASHFS)

static z_stream stream;

#define ZALLOC_ALIGNMENT	16

static void *zalloc (void *x, unsigned items, unsigned size)
{
	void *p;

	size *= items;
	size = (size + ZALLOC_ALIGNMENT - 1) & ~(ZALLOC_ALIGNMENT - 1);

	p = malloc (size);

	return (p);
}

static void zfree (void *x, void *addr, unsigned nb)
{
	free (addr);
}

/* The correct size is already known in advance for squashfs from the meta data */
int squashfs_uncompress_block (void *dst, int dstlen, void *src, int srclen)
{
	int err;
	inflateReset (&stream);

	stream.next_in = src;
	stream.avail_in = srclen;

	stream.next_out = dst;
	stream.avail_out = dstlen;

	err = inflate (&stream, Z_FINISH);
	if ((err==Z_OK)||(err=Z_STREAM_END))
		return dstlen-stream.avail_out;
	else
		return 0;
}

int squashfs_uncompress_init (void)
{
	int err;

	stream.zalloc = zalloc;
	stream.zfree = zfree;
	stream.next_in = 0;
	stream.avail_in = 0;

#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
	stream.outcb = (cb_func) WATCHDOG_RESET;
#else
	stream.outcb = Z_NULL;
#endif /* CONFIG_HW_WATCHDOG */

	err = inflateInit (&stream);
	if (err != Z_OK) {
		printf ("Error: inflateInit() returned %d\n", err);
		return -1;
	}

	return 0;
}

int squashfs_uncompress_exit (void)
{
	inflateEnd (&stream);
	return 0;
}

#endif /* CFG_FS_SQUASHFS */
