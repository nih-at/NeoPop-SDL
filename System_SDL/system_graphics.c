#include <errno.h>
#include <SDL.h>
#include "neopop-SDL.h"

/* graphics size requested: 1 normal, 2 double size, 3 triple size */
int graphics_mag_req = 1;

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

    SDL_BlitSurface(corescr, NULL, disp, NULL);
    SDL_Flip(disp);
}
