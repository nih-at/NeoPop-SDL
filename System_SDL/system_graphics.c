/* $NiH: system_graphics.c,v 1.12 2003/10/15 12:16:14 wiz Exp $ */
/*
  system_graphics.c -- graphics support functions
  Copyright (C) 2002-2003 Thomas Klausner

  This file is part of NeoPop-SDL, a NeoGeo Pocket emulator
  The author can be contacted at <tk@giga.or.at>

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

#include <errno.h>
#include "NeoPop-SDL.h"

/* graphics size requested: 1 normal, 2 double size, 3 triple size */
int graphics_mag_req = 1;

/* smooth magnification mode: 1 on, 0 off */
int graphics_mag_smooth = 1;

/* display structure */
static SDL_Surface *disp = NULL;
/* SDL Surface containing the screen data */
static SDL_Surface *corescr = NULL;
/* displayed graphics is how big compared to original size? 1: normal size,
 * 2: double size, 3: triple size */
static int graphics_mag_actual = 1;
/* display in full screen mode? */
static int fs_mode = 0;


static BOOL system_graphics_screen_init(int mfactor);
static BOOL system_graphics_mag_smooth(void);

BOOL
system_graphics_init(void)
{
    Uint32 pixel;

    if (system_graphics_screen_init(1) == FALSE) {
	fprintf(stderr, "cannot create main window: %s\n", SDL_GetError());
	return FALSE;
    }

    /* set window caption */
    SDL_WM_SetCaption(PROGRAM_NAME, NULL);

    /* fill screen green */
    pixel = SDL_MapRGB(corescr->format, 0, 0xff, 0);
    SDL_FillRect(corescr, NULL, pixel);

    SDL_BlitSurface(corescr, NULL, disp, NULL);
    SDL_Flip(disp);

    return TRUE;
}

void
system_graphics_fullscreen_toggle(void)
{
    SDL_WM_ToggleFullScreen(disp);
    fs_mode = (fs_mode ? 0 : 1);
    /* hide mouse pointer in fullscreen mode */
    if (SDL_ShowCursor(-1) == fs_mode)
	    SDL_ShowCursor(1-fs_mode);
}

static BOOL
system_graphics_screen_init(int mfactor)
{
    SDL_Surface *disp_new;
    SDL_Surface *corescr_new;
    Uint32 rm, gm, bm;
    int flags;

    flags = SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE;
    if (fs_mode)
	flags |= SDL_FULLSCREEN;
    rm = 0x000f;
    gm = 0x00f0;
    bm = 0x0f00;

    if ((disp_new=SDL_SetVideoMode(mfactor*SCREEN_WIDTH,
				   mfactor*SCREEN_HEIGHT, 16,
				   flags)) == NULL) {
	fprintf(stderr, "cannot switch mode: %s\n", SDL_GetError());
	return FALSE;
    }

    if (mfactor == 1) {
	corescr_new = SDL_CreateRGBSurfaceFrom(cfb, SCREEN_WIDTH,
					       SCREEN_HEIGHT, 16,
					       sizeof(_u16)*SCREEN_WIDTH,
					       rm, gm, bm, 0);
    }
    else {
	corescr_new = SDL_CreateRGBSurface(SDL_SWSURFACE, mfactor*SCREEN_WIDTH,
					   mfactor*SCREEN_HEIGHT, 16, rm, gm,
					   bm, 0);
    }

    if (corescr_new == NULL) {
	fprintf(stderr, "CreateRGBSurface failed: %s", SDL_GetError());
	return FALSE;
    }

    disp = disp_new;
    SDL_FreeSurface(corescr);
    corescr = corescr_new;

    return TRUE;
}

static BOOL
system_graphics_mag_smooth(void)
{
    int x, y, linelen;
    /*
     * offset of currently handled pixel in original (small)
     * graphics data
     */
    int cfb_offset;
    /* pointers to neighbours in the original (small) graphics data */
    _u16 *cfb_up, *cfb_left, *cfb_down, *cfb_right;
    /* pointer to currently handled pixel in magnified graphics data */
    _u16 *pixelptr;
    /* results of some tests for smooth magnification */
    BOOL conddl, conddr, condul, condur;

    pixelptr = corescr->pixels;
    cfb_offset = 0;
    linelen = graphics_mag_actual*SCREEN_WIDTH;
    /* first line */
    for (x=0;x<SCREEN_WIDTH; x++) {
	switch (graphics_mag_actual) {
	case 3:
	    *pixelptr++ = cfb[cfb_offset];
	    /* FALLTHROUGH */
	case 2:
	    *pixelptr++ = cfb[cfb_offset];
	    *pixelptr++ = cfb[cfb_offset];
	    break;
	default:
	    break;
	}
	cfb_offset++;
    }

    switch (graphics_mag_actual) {
    case 3:
	memcpy(pixelptr, pixelptr-linelen, linelen*sizeof(_u16));
	pixelptr += linelen;
	/* FALLTHROUGH */
    case 2:
	memcpy(pixelptr, pixelptr-linelen, linelen*sizeof(_u16));
	pixelptr += linelen;
	break;
    default:
	break;
    }

    for (y=1; y<SCREEN_HEIGHT-1; y++) {
	/* first pixels in each line */
	switch (graphics_mag_actual) {
	case 3:
	    *(pixelptr+6*SCREEN_WIDTH) = cfb[cfb_offset];
	    *(pixelptr+3*SCREEN_WIDTH) = cfb[cfb_offset];
	    *pixelptr++ = cfb[cfb_offset];
	    *(pixelptr+6*SCREEN_WIDTH) = cfb[cfb_offset];
	    *(pixelptr+3*SCREEN_WIDTH) = cfb[cfb_offset];
	    *pixelptr++ = cfb[cfb_offset];
	    *(pixelptr+6*SCREEN_WIDTH) = cfb[cfb_offset];
	    *(pixelptr+3*SCREEN_WIDTH) = cfb[cfb_offset];
	    *pixelptr++ = cfb[cfb_offset];
	    break;
	case 2:
	    *(pixelptr+2*SCREEN_WIDTH) = cfb[cfb_offset];
	    *pixelptr++ = cfb[cfb_offset];
	    *(pixelptr+2*SCREEN_WIDTH) = cfb[cfb_offset];
	    *pixelptr++ = cfb[cfb_offset];
	    break;
	default:
	    break;
	}
	cfb_offset++;

	cfb_up = cfb+cfb_offset-SCREEN_WIDTH;
	cfb_down = cfb+cfb_offset+SCREEN_WIDTH;
	cfb_left = cfb+cfb_offset-1;
	cfb_right = cfb+cfb_offset+1;
	for (x=1; x<SCREEN_WIDTH-1; x++) {
	    condul =  *cfb_up == *cfb_left
		&& *cfb_left != *cfb_down && *cfb_up != *cfb_right;
	    condur =  *cfb_up == *cfb_right
		&& *cfb_right != *cfb_down && *cfb_up != *cfb_left;
	    conddl =  *cfb_down == *cfb_left
		&& *cfb_down != *cfb_right && *cfb_left != *cfb_up;
	    conddr =  *cfb_down == *cfb_right
		&& *cfb_down != *cfb_left && *cfb_right != *cfb_up;
	    switch (graphics_mag_actual) {
	    case 3:
		/* left pixel column */
		*(pixelptr+6*SCREEN_WIDTH) = conddl ? *cfb_left : cfb_left[1];
		*(pixelptr+3*SCREEN_WIDTH) = cfb_left[1];
		*pixelptr++ = condul ? *cfb_left : cfb_left[1];
		/* center pixel column */
		*(pixelptr+6*SCREEN_WIDTH) = cfb_left[1];
		*(pixelptr+3*SCREEN_WIDTH) = cfb_left[1];
		*pixelptr++ = cfb_left[1];
		/* right pixel column */
		*(pixelptr+6*SCREEN_WIDTH) = conddr ? *cfb_right : cfb_left[1];
		*(pixelptr+3*SCREEN_WIDTH) = cfb_left[1];
		*pixelptr++ = condur ? *cfb_right : cfb_left[1];
		break;
	    case 2:
		*(pixelptr+2*SCREEN_WIDTH) = conddl ? *cfb_down : cfb_left[1];
		*pixelptr++ = condul ? *cfb_up : cfb_left[1];
		*(pixelptr+2*SCREEN_WIDTH) = conddr ? *cfb_down : cfb_left[1];
		*pixelptr++ = condur ? *cfb_up : cfb_left[1];
		break;
	    default:
		break;
	    }
	    cfb_left++;
	    cfb_right++;
	    cfb_up++;
	    cfb_down++;
	    cfb_offset++;
	}

	/* last pixels in each line */
	switch (graphics_mag_actual) {
	case 3:
	    *(pixelptr+6*SCREEN_WIDTH) = cfb[cfb_offset];
	    *(pixelptr+3*SCREEN_WIDTH) = cfb[cfb_offset];
	    *pixelptr++ = cfb[cfb_offset];
	    *(pixelptr+6*SCREEN_WIDTH) = cfb[cfb_offset];
	    *(pixelptr+3*SCREEN_WIDTH) = cfb[cfb_offset];
	    *pixelptr++ = cfb[cfb_offset];
	    *(pixelptr+6*SCREEN_WIDTH) = cfb[cfb_offset];
	    *(pixelptr+3*SCREEN_WIDTH) = cfb[cfb_offset];
	    *pixelptr++ = cfb[cfb_offset];
	    break;
	case 2:
	    *(pixelptr+2*SCREEN_WIDTH) = cfb[cfb_offset];
	    *pixelptr++ = cfb[cfb_offset];
	    *(pixelptr+2*SCREEN_WIDTH) = cfb[cfb_offset];
	    *pixelptr++ = cfb[cfb_offset];
	    break;
	default:
	    break;
	}
	cfb_offset++;
	/*
	 * since we already filled graphics_mag_actual lines, skip
	 * (n-1) lines of length n.
	 */ 
	switch (graphics_mag_actual) {
	case 3:
	    pixelptr += 6*SCREEN_WIDTH;
	    break;
	case 2:
	    pixelptr += 2*SCREEN_WIDTH;
	    break;
	default:
	    break;
	}
    }

    /* last line */
    for (x=0;x<SCREEN_WIDTH; x++) {
	switch (graphics_mag_actual) {
	case 3:
	    *pixelptr++ = cfb[cfb_offset];
	    /* FALLTHROUGH */
	case 2:
	    *pixelptr++ = cfb[cfb_offset];
	    *pixelptr++ = cfb[cfb_offset];
	    break;
	default:
	    break;
	}
	cfb_offset++;
    }

    switch (graphics_mag_actual) {
    case 3:
	memcpy(pixelptr, pixelptr-linelen, linelen*sizeof(_u16));
	pixelptr += linelen;
	/* FALLTHROUGH */
    case 2:
	memcpy(pixelptr, pixelptr-linelen, linelen*sizeof(_u16));
	pixelptr += linelen;
	break;
    default:
	break;
    }

    return TRUE;
}

void
system_graphics_update(void)
{

    /* handle screen size changes */
    if (graphics_mag_req != graphics_mag_actual) {
	if (system_graphics_screen_init(graphics_mag_req) == FALSE) {
	    fprintf(stderr, "can't switch magnification factor to %d, "
		    "staying at %d\n", graphics_mag_req,
		    graphics_mag_actual);
	    graphics_mag_req = graphics_mag_actual;
	}
	else
	    graphics_mag_actual = graphics_mag_req;
    }

    if (graphics_mag_actual > 1) {
	if (!graphics_mag_smooth || system_graphics_mag_smooth() == FALSE) {
	    /* simple magnification */
	    _u16 *pixelptr;
	    int x, y, linelen;
	    int cfb_offset;

	    pixelptr = corescr->pixels;
	    cfb_offset = 0;
	    linelen = graphics_mag_actual*SCREEN_WIDTH;
	    for (y=0; y<SCREEN_HEIGHT; y++) {
		for (x=0; x<SCREEN_WIDTH; x++) {
		    /* magnify in x-direction */
		    switch(graphics_mag_actual) {
		    case 3:
			*pixelptr++ = cfb[cfb_offset];
			/* FALLTHROUGH */
		    case 2:
			*pixelptr++ = cfb[cfb_offset];
			*pixelptr++ = cfb[cfb_offset];
			break;
		    default:
			break;
		    }
		    cfb_offset++;
		}

		/* magnify in y-direction */
		switch(graphics_mag_actual) {
		case 3:
		    memcpy(pixelptr, pixelptr-linelen, linelen*sizeof(_u16));
		    pixelptr += linelen;
		    /* FALLTHROUGH */
		case 2:
		    memcpy(pixelptr, pixelptr-linelen, linelen*sizeof(_u16));
		    pixelptr += linelen;
		    break;
		default:
		    break;
		}
	    }
	}
    }

    SDL_BlitSurface(corescr, NULL, disp, NULL);
    SDL_Flip(disp);
}
