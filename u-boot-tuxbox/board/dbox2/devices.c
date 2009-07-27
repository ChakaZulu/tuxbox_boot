/*
 * Copyright (C) 2001 Florian Schirmer <jolt@tuxbox.org>
 *               2002 Bastian Blank <waldi@tuxbox.org>
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

#include <common.h>
#include <stdio_dev.h>

#if defined(CONFIG_DBOX2_ENV_READ_NFS) || defined(CONFIG_DBOX2_ENV_READ_TFTP)
void load_env_net (void);
#endif

#ifdef CONFIG_DBOX2_FB
static struct stdio_dev fbdev;

int fb_init (void);

# ifdef CONFIG_DBOX2_FB_LOGO
int fb_load (void);
# endif /* CONFIG_DBOX2_FB_LOGO */
#endif /* CONFIG_DBOX2_FB */

#ifdef CONFIG_LCD_BOARD
#include <lcd.h>

static struct stdio_dev lcddev;

# ifdef CONFIG_DBOX2_LCD_LOGO
int lcd_load (void);
# endif /* CONFIG_DBOX2_LCD_LOGO */
#endif /* CONFIG_LCD_BOARD */

int drv_dbox2_init (void)
{
#ifdef CONFIG_DBOX2_FB
	fb_init ();

	strcpy (fbdev.name, "fb");
	stdio_register (&fbdev);
#endif /* CONFIG_DBOX2_FB */

	return 1;
}

#ifdef CONFIG_LAST_STAGE_INIT
int last_stage_init (void)
{
#if defined(CONFIG_DBOX2_ENV_READ_NFS) || defined(CONFIG_DBOX2_ENV_READ_TFTP)
	load_env_net ();
#endif
	
#ifdef CONFIG_LCD_BOARD
	lcd_init ();

	strcpy (lcddev.name, "lcd");
	lcddev.putc  = lcd_putc;
	lcddev.puts  = lcd_puts;
	stdio_register (&lcddev);
#endif /* CONFIG_LCD_BOARD */

#ifdef CONFIG_DBOX2_LCD_LOGO
	lcd_load ();
#endif /* CONFIG_DBOX2_LCD_LOGO */

#ifdef CONFIG_DBOX2_FB_LOGO
	fb_load ();
#endif /* CONFIG_DBOX2_FB_LOGO */

	return 1;
}
#endif /* CONFIG_LAST_STAGE_INIT */

