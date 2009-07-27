/*
 * fb.c
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
 * $Id: fb.c,v 1.3 2009/07/27 14:39:01 rhabarber1848 Exp $
 */

#include <common.h>

#ifdef CONFIG_DBOX2_FB

#ifdef CONFIG_DBOX2_FB_LOGO_FS
#include <cmd_fs.h>
#endif /* CONFIG_DBOX2_FB_LOGO_FS */

#if defined(CONFIG_DBOX2_FB_LOGO_TFTP) || defined(CONFIG_DBOX2_FB_LOGO_NFS)
#include <net.h>
#endif /* CONFIG_DBOX2_FB_LOGO_TFTP */

extern void saa7126_init (int);
extern void avs_init (int);
extern void avs_blank (int);
extern void avia_init (int, unsigned char*);

static unsigned char *hwi = (unsigned char *) (CONFIG_SYS_FLASH_BASE + CONFIG_SYS_HWINFO_OFFSET);

int fb_init (void)
{
	puts ("FB:    ");

#ifdef CONFIG_DBOX2_FB_LOGO
	avs_blank (hwi[0]);
#else /* CONFIG_DBOX2_FB_LOGO */
	avs_init (hwi[0]);
#endif /* CONFIG_DBOX2_FB_LOGO */
	saa7126_init (hwi[0]);

	puts ("ready\n");

	return 0;
}

#ifdef CONFIG_DBOX2_FB_LOGO
int fb_load (void)
{
	unsigned char *fb_logo = (unsigned char *) CONFIG_SYS_LOAD_ADDR;
	int size;

# ifdef CONFIG_DBOX2_FB_LOGO_FS
	size = fs_fsload ((unsigned long) fb_logo, CONFIG_DBOX2_FB_LOGO_FS);

	if (size <= 0)
	{
#  ifdef CONFIG_DBOX2_FB_LOGO_TFTP
		puts ("can't find logo in flash - try network\n");
#  else /* CONFIG_DBOX2_FB_LOGO_TFTP */
		puts ("can't find logo in flash\n");
		return 1;
#  endif /* CONFIG_DBOX2_FB_LOGO_TFTP */
	}
	else
		goto load_logo;
# endif /* CONFIG_DBOX2_FB_LOGO_FS */

# if defined(CONFIG_DBOX2_FB_LOGO_TFTP) || defined(CONFIG_DBOX2_FB_LOGO_NFS)
	NetLoop (BOOTP);
	netboot_update_env();

#ifdef CONFIG_DBOX2_FB_LOGO_TFTP
	copy_filename (BootFile, CONFIG_DBOX2_FB_LOGO_TFTP, sizeof (BootFile));
	size = NetLoop (TFTP);
#elif defined(CONFIG_DBOX2_FB_LOGO_NFS)
	ip_to_string (NetServerIP, BootFile);
	strncat(BootFile, ":", sizeof (BootFile));
	strncat(BootFile, NetOurRootPath, sizeof (BootFile));
	strncat(BootFile, CONFIG_DBOX2_FB_LOGO_NFS, sizeof (BootFile));
	size = NetLoop (NFS);
#else
	size = -1;
#endif
	if (size <= 0)
	{
		puts ("can't find FB logo\n");
		return 1;
	}
	else
		goto load_logo;

# endif /* CONFIG_DBOX2_FB_LOGO_TFTP */
load_logo:
	avia_init (hwi[0], fb_logo);
	avs_init (hwi[0]);

	return 0;
}
#endif /* CONFIG_DBOX2_FB_LOGO */

#endif
