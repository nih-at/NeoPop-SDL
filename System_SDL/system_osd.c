/* $NiH$

  system_osd.c -- on-screen display
  Copyright (C) 2004 Thomas Klausner and Dieter Baron

  This file is part of NeoPop-SDL, a NeoGeo Pocket emulator
  The author can be contacted at <wiz@danbala.tuwien.ac.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/



#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include "NeoPop-SDL.h"

#define OSD_DISPLAY_SECONDS	2	/* seconds to display OSD message */

#include "font.xpm"

static _u16 font[8*8*64];

static int font_usable;

static char osd[SCREEN_WIDTH/8];
static int osd_timer;



int
system_osd_init(void)
{
    _u16 pal[128];
    int w, h;
    int i, n, pixel;
    int x, y, c;
    char *p;

    font_usable = FALSE;

    w = strtol(font_xpm[0], &p, 10);
    h = strtol(p, &p, 10);
    n = strtol(p, &p, 10);

    if (w%8 || h%8 || w*h != 64*8*8)
	return FALSE;

    if (strtol(p, &p, 10) != 1)
	return FALSE;

    for (i=0; i<n; i++) {
	if ((p=strrchr(font_xpm[i+1], '#')) == NULL)
	    return FALSE;
	    
	pixel = strtol(p+1, NULL, 16);
	pal[(int)font_xpm[i+1][0]] = ((pixel&0xff0000)>>12)
	    | ((pixel&0xff00)>>8)
	    | ((pixel&0xff)>>4);
    }

    for (y=0; y<h; y++) {
	for (x=0; x<w; x++) {
	    c = (y/8) * (w/8) + (x/8);
	    font[c*8*8 + (y%8)*8 + x%8] = pal[(int)font_xpm[y+n+1][x]];
	}
    }

    font_usable = TRUE;

    return TRUE;
}



void
system_osd(const char *fmt, ...)
{
    int i;
    va_list va;

    if (!font_usable)
	return;

    va_start(va, fmt);
    vsnprintf(osd, sizeof(osd), fmt, va);
    va_end(va);
    osd_timer = OSD_DISPLAY_SECONDS*NGP_FPS;

    /* eliminate illegal chars */
    for (i=0; osd[i]; i++) {
	if (osd[i] >= 'a' && osd[i] <= 'z')
	    osd[i] = toupper(osd[i]);
	else if (osd[i] < ' ' || osd[i] > '_')
	    osd[i] = ' ';
    }
}



void
system_osd_display(void)
{
    int i, x, y;
    _u16 *base;

    if (!font_usable || osd_timer == 0)
	return;

    --osd_timer;

    base = cfb + SCREEN_WIDTH*(SCREEN_HEIGHT-8);
    
    for (i=0; osd[i]; i++) {
	for (y=0; y<8; y++)
	    for (x=0; x<8; x++)
		base[y*SCREEN_WIDTH+x] = font[(osd[i]-32)*64+y*8+x];
	base += 8;
    }
}
