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
 * $Id: fb.c,v 1.6 2003/03/14 16:30:57 waldi Exp $
 */

#include <common.h>

#ifdef CONFIG_DBOX2_FB

#ifdef CONFIG_DBOX2_FB_LOGO_FS
#include <cmd_fs.h>
#endif /* CONFIG_DBOX2_FB_LOGO_FS */

#ifdef CONFIG_DBOX2_FB_LOGO_TFTP
#include <net.h>
#endif /* CONFIG_DBOX2_FB_LOGO_TFTP */

extern unsigned char mid;

extern void saa7126_init (int);
extern void avs_init (int);
extern void avs_blank (int);
extern void avia_init_pre (int);
extern void avia_init_load (unsigned char*);
extern void avia_init_post (int);

int fb_init (void)
{
	puts ("FB:    ");

#ifdef CONFIG_DBOX2_FB_LOGO
	avs_blank (mid);
#else /* CONFIG_DBOX2_FB_LOGO */
	avs_init (mid);
#endif /* CONFIG_DBOX2_FB_LOGO */
	saa7126_init (mid);
	avia_init_pre (mid);

	puts ("ready\n");

	return 0;
}

#ifdef CONFIG_DBOX2_FB_LOGO
int fb_load (void)
{
	unsigned char *fb_logo = (unsigned char *) 0x100000;
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
# ifdef CONFIG_DBOX2_FB_LOGO_TFTP
	NetLoop (BOOTP);
	copy_filename (BootFile, CONFIG_DBOX2_FB_LOGO_TFTP, sizeof (BootFile));
	size = NetLoop (TFTP);

	if (size <= 0)
	{
		puts ("can't find logo\n");
		return 1;
	}
	else
		goto load_logo;
# endif /* CONFIG_DBOX2_FB_LOGO_TFTP */
load_logo:
	avia_init_load (fb_logo);
	avia_init_post (mid);
	avs_init (mid);

	return 0;
}
#endif /* CONFIG_DBOX2_FB_LOGO */

#endif
