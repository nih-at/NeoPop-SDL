/* $NiH: system_graphics.c,v 1.17 2004/07/14 00:38:54 dillo Exp $ */
/*
  system_graphics.c -- graphics support functions
  Copyright (C) 2002-2004 Thomas Klausner and Dieter Baron

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

#include <errno.h>
#include "NeoPop-SDL.h"

/* graphics size requested: 1 normal, 2 double size, 3 triple size */
int graphics_mag_req = 1;

/* smooth magnification mode: 1 on, 0 off */
int graphics_mag_smooth = 1;

/* use YUV overlay (hardware scaling) */
int use_yuv;
/* use YUV overlay even if not hardware accelerated */
int use_software_yuv;

/* display structure */
static SDL_Surface *disp = NULL;

/* YUV overlay */
static SDL_Overlay *over = NULL;
/* scale rectangle for fullscreen and windowed */
static SDL_Rect orect_fs, orect_win;
/* use YUV in current setting */
static int use_yuv_now;

/* displayed graphics is how big compared to original size? 1: normal size,
 * 2: double size, 3: triple size */
static int graphics_mag_actual = 1;

/* display in full screen mode? */
static int fs_mode = 0;


static BOOL system_graphics_screen_init(int mfactor);
#if 0
static BOOL system_graphics_mag_smooth(void);
#endif

/* lookup table for display surface pixels */
static Uint32 rgb_lookup[16*16*16];
/* lookup table for YUV overlay pixels */
static Uint32 yuv_lookup[16*16*16];

/* convert 4bit colour component to 8bit */
#define CONV4TO8(x)	(((x)<<4)|(x))

BOOL
system_graphics_init(void)
{
    int i;

    if (use_yuv) {
	SDL_Rect **mode;
	double mul;

	orect_fs.x = orect_fs.y = orect_win.x = orect_win.y = 0;
	
	if ((mode=SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE)) == NULL) {
	    fprintf(stderr, "cannot get list of modes");
	    orect_fs.w = SCREEN_WIDTH;
	    orect_fs.h = SCREEN_HEIGHT;
	}
	else {
	    mul = mode[0]->w/(double)SCREEN_WIDTH;
	    if (mode[0]->h/(double)SCREEN_HEIGHT < mul)
		mul = mode[0]->h/(double)SCREEN_HEIGHT;
	    orect_fs.w = SCREEN_WIDTH*mul;
	    orect_fs.h = SCREEN_HEIGHT*mul;
	}
    }

    if (system_graphics_screen_init(graphics_mag_req) == FALSE) {
	fprintf(stderr, "cannot create main window: %s\n", SDL_GetError());
	return FALSE;
    }
    graphics_mag_actual = graphics_mag_req;

    if (use_yuv) {
	if ((over=SDL_CreateYUVOverlay(SCREEN_WIDTH*2, SCREEN_HEIGHT,
				       SDL_YUY2_OVERLAY, disp)) == NULL) {
	    fprintf(stderr, "cannot create YUV overlay\n");
	    use_yuv = 0;
	}
	if (over->format != SDL_YUY2_OVERLAY || over->planes != 1) {
	    fprintf(stderr, "unsupported YUV overlay format\n");
	    use_yuv = 0;
	}
	if (!use_software_yuv && !over->hw_overlay) {
	    fprintf(stderr, "not using software YUV overlay\n");
	    SDL_FreeYUVOverlay(over);
	    use_yuv = 0;
	}
    }
	    
    if (use_yuv) {
	int y, u, v;
	int r, g, b;

	for (i=0; i<16*16*16; i++) {
	    r = CONV4TO8(i&0xf);
	    g = CONV4TO8((i>>4)&0xf);
	    b = CONV4TO8(i>>8);

	    /* see http://www.fourcc.org/fccyvrgb.php#mikes_answer */
	    
	    y = (((r * 16982) + (g * 32828) + (b * 6475)) >> 16) + 16;
	    u = ((r * 28656) - (g * 23995) - (b * 4660) + 8355840) >> 16;
	    v = (-(r * 9671) - (g * 18985) + (b * 28656) + 8355840) >> 16;
	    
#ifdef LSB_FIRST
	    yuv_lookup[i] = (u<<24)|(y<<16)|(v<<8)|y;
#else
	    yuv_nlookup[i] = (y<<24)|(v<<16)|(y<<8)|u;
#endif
	}
    }

    if (use_yuv < YUV_ALWAYS)
	for (i=0; i<16*16*16; i++)
	    rgb_lookup[i] = SDL_MapRGB(disp->format,
				       CONV4TO8(i&0xf),
				       CONV4TO8((i>>4)&0xf),
				       CONV4TO8(i>>8));

    /* set window caption */
    SDL_WM_SetCaption(PROGRAM_NAME, NULL);

    /* fill screen green */
    SDL_FillRect(disp, NULL, SDL_MapRGB(disp->format, 0, 0xff, 0));

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
    if (use_yuv >= YUV_FULLSCREEN)
	system_graphics_screen_init(graphics_mag_actual);
}

static BOOL
system_graphics_screen_init(int mfactor)
{
    SDL_Surface *disp_new;
    int flags, w, h;

    flags = SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE | SDL_ANYFORMAT;
    if (fs_mode)
	flags |= SDL_FULLSCREEN;

    if (use_yuv >= YUV_FULLSCREEN && fs_mode) {
	w = orect_fs.w;
	h = orect_fs.h;
    }
    else {
	w = mfactor*SCREEN_WIDTH;
	h = mfactor*SCREEN_HEIGHT;
    }

    if ((disp_new=SDL_SetVideoMode(w, h, 16, flags)) == NULL) {
	fprintf(stderr, "cannot switch mode: %s\n", SDL_GetError());
	return FALSE;
    }

    disp = disp_new;

    if (fs_mode && use_yuv >= YUV_FULLSCREEN)
	use_yuv_now = 1;
    else if ((mfactor > 1 && use_yuv >= YUV_MAGNIFIED)
	     || use_yuv >= YUV_ALWAYS) {
	use_yuv_now = 1;
	orect_win.w = w;
	orect_win.h = h;
    }
    else
	use_yuv_now = 0;

    return TRUE;
}

#if 0
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
#endif

/* set pixel, magnification 1 */
#define SET1	(line[x] = lookup[*(fbp++)])
/* set pixel, magnification 2 */
#define SET2	(line[pitch+x+1] = line[pitch+x] = line[x+1] = SET1)
/* set pixel, magnification 3 */
#define SET3	(line[pitch*2+x+2] = line[pitch*2+x+1] = line[pitch*2+x] = \
		 line[pitch+x+2] = line[x+2] = SET2)

/*
  convert framebuffer.
    T: type of pixels
    S: size of pixels (in bytes)
    F: magnification factor
    P: pointer to pixels b
*/
#define LOOP(T, S, F, P)	{		\
    T *line;					\
						\
    line = (T *)P;				\
    fbp = cfb;					\
    for (y=0; y<SCREEN_HEIGHT; y++) {		\
	for (x=0; x<SCREEN_WIDTH*F; x+=F)	\
	    SET##F;				\
	line += pitch*F;			\
    }						\
}

/*
  convert framebuffer for given display depth
    T: type of pixels
    S: size of pixels (in bytes)
*/   
#define SWITCH(T, S)				\
        pitch = disp->pitch/S;			\
	switch(graphics_mag_actual) {		\
	case 1:					\
	    LOOP(T, S, 1, disp->pixels);	\
	    break;				\
	case 2:					\
	    LOOP(T, S, 2, disp->pixels);	\
	    break;				\
	case 3:					\
	    LOOP(T, S, 3, disp->pixels);	\
	    break;				\
	}

void
system_graphics_update(void)
{
    Uint32 *lookup;
    int x, y, pitch;
    _u16 *fbp;

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

    if (use_yuv_now) {
	SDL_LockYUVOverlay(over);
	pitch = over->pitches[0]/4;
	lookup = yuv_lookup;
	LOOP(Uint32, 4, 1, over->pixels[0]);
	SDL_UnlockYUVOverlay(over);

	SDL_DisplayYUVOverlay(over, (fs_mode ? &orect_fs : &orect_win));
    }
    else {
	lookup = rgb_lookup;
	switch (disp->format->BytesPerPixel) {
	case 1:
	    SWITCH(Uint8, 1);
	    break;
	    
	case 2:
	    SWITCH(Uint16, 2);
	    break;
	    
	case 4:
	    SWITCH(Uint32, 4);
	    break;
	}
	SDL_Flip(disp);
    }

}
