/*
 * cramfs.c
 *
 * Copyright (C) 1999 Linus Torvalds
 * Copyright (C) 2000-2002 Transmeta Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 *
 * Compressed ROM filesystem for Linux.
 */

/*
 * These are the VFS interfaces to the compressed ROM filesystem.
 * The actual compression is based on zlib, see the other files.
 */

#include <common.h>
#include <malloc.h>

#if (CONFIG_FS & CFG_FS_CRAMFS)

#include <cmd_fs.h>
#include "cramfs_fs.h"

/* These two macros may change in future, to provide better st_ino
   semantics. */
#define CRAMINO(x)	(CRAMFS_GET_OFFSET(x) ? CRAMFS_GET_OFFSET(x)<<2 : 1)
#define OFFSET(x)	((x)->i_ino)

struct cramfs_super super;

static int cramfs_read_super (struct part_info *info)
{
	unsigned long root_offset;

	/* Read the first block and get the superblock from it */
	memcpy (&super, (void *) info->offset, sizeof(super));

	/* Do sanity checks on the superblock */
	if (super.magic != CRAMFS_32 (CRAMFS_MAGIC)) {
		/* check at 512 byte offset */
		memcpy (&super, (void *) info->offset + 512, sizeof(super));
		if (super.magic != CRAMFS_32 (CRAMFS_MAGIC)) {
			printf ("cramfs: wrong magic\n");
			return -1;
		}
	}

	/* flags is reused several times, so swab it once */
	super.flags = CRAMFS_32 (super.flags);
	super.size = CRAMFS_32 (super.size);

	/* get feature flags first */
	if (super.flags & ~CRAMFS_SUPPORTED_FLAGS) {
		printf ("cramfs: unsupported filesystem features\n");
		return -1;
	}

	/* Check that the root inode is in a sane state */
	if (!S_ISDIR (CRAMFS_16 (super.root.mode))) {
		printf ("cramfs: root is not a directory\n");
		return -1;
	}
	root_offset = CRAMFS_GET_OFFSET (&(super.root)) << 2;
	if (root_offset == 0)
		printf ("cramfs: empty filesystem");
	else if (!(super.flags & CRAMFS_FLAG_SHIFTED_ROOT_OFFSET) &&
		 ((root_offset != sizeof (struct cramfs_super)) &&
		  (root_offset != 512 + sizeof (struct cramfs_super))))
	{
		printf ("cramfs: bad root offset %lu\n", root_offset);
		return -1;
	}

	return 0;
}

static unsigned long cramfs_load_readdir (unsigned long begin, unsigned long offset, unsigned long size, char *filename)
{
	unsigned long inodeoffset = 0, nextoffset;

	while (inodeoffset < size) {
		struct cramfs_inode *inode;
		char *name;
		int namelen;

		inode = (struct cramfs_inode *) (begin + offset + inodeoffset);

		/*
		 * Namelengths on disk are shifted by two
		 * and the name padded out to 4-byte boundaries
		 * with zeroes.
		 */
		namelen = CRAMFS_GET_NAMELEN(inode) << 2;
		name = (char *) inode + sizeof (struct cramfs_inode);

		nextoffset = inodeoffset + sizeof (struct cramfs_inode) + namelen;

		for (;;) {
			if (!namelen)
				return -1;
			if (name[namelen-1])
				break;
			namelen--;
		}

		if (!strncmp (filename, name, namelen))
		{
			if (S_ISDIR (CRAMFS_16 (inode->mode)))
			{
				return cramfs_load_readdir (begin, CRAMFS_GET_OFFSET(inode) << 2, CRAMFS_24(inode->size), strtok (NULL, "/"));
			}
			else if (S_ISREG (CRAMFS_16 (inode->mode)))
			{
				return offset + inodeoffset;
			}
			else
			{
				printf ("find unknown thing\n");
				return 0;
			}
		}

		inodeoffset = nextoffset;
	}

	printf ("can't find corresponding entry\n");
	return 0;
}

static int cramfs_load_uncompress (unsigned long begin, unsigned long offset, unsigned long loadoffset)
{
	struct cramfs_inode *inode = (struct cramfs_inode *) (begin + offset);
	unsigned long *block_ptrs = (unsigned long *) (begin + (CRAMFS_GET_OFFSET (inode) << 2));
	unsigned long curr_block = (CRAMFS_GET_OFFSET (inode) + (((CRAMFS_24 (inode->size)) + 4095) >> 12)) << 2;
	int i;
	int size, total_size = 0;

	cramfs_uncompress_init ();

	for (i = 0; i < ((CRAMFS_24(inode->size) + 4095) >> 12); i++)
       	{
		size = cramfs_uncompress_block ((void *) loadoffset, (void *) (begin + curr_block), (CRAMFS_32 (block_ptrs[i]) - curr_block));
		if (size < 0)
		       return size;
		loadoffset += size;
		total_size += size;
		curr_block = CRAMFS_32 (block_ptrs[i]);
	}

	return total_size;
}

int cramfs_load (char *loadoffset, struct part_info *info, char *filename)
{
	unsigned long offset;

	if (cramfs_read_super (info))
		return -1;

	offset = cramfs_load_readdir (info->offset, CRAMFS_GET_OFFSET(&(super.root)) << 2, CRAMFS_24(super.root.size), strtok (filename, "/"));

	if (offset <= 0)
		return offset;

	return cramfs_load_uncompress (info->offset, offset, (unsigned long) loadoffset);
}

int cramfs_ls (struct part_info *info, char *filename)
{
	printf ("not implemented\n");
	return -1;
}

int cramfs_info (struct part_info *info)
{
	if (cramfs_read_super (info))
		return 0;

	printf ("size: 0x%x (%u)\n", super.size, super.size);

	if (super.flags != 0)
	{
		printf ("flags:\n");
		if (super.flags & CRAMFS_FLAG_FSID_VERSION_2)
			printf ("\tFSID version 2\n");
		if (super.flags & CRAMFS_FLAG_SORTED_DIRS)
			printf ("\tsorted dirs\n");
		if (super.flags & CRAMFS_FLAG_HOLES)
			printf ("\tholes\n");
		if (super.flags & CRAMFS_FLAG_SHIFTED_ROOT_OFFSET)
			printf ("\tshifted root offset\n");
	}

	printf ("fsid:\n\tcrc: 0x%x\n\tedition: 0x%x\n", super.fsid.crc, super.fsid.edition);
	printf ("name: %16s\n", super.name);

	return 1;
}

#endif /* CFG_FS_CRAMFS */
