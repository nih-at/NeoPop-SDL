#include <SDL.h>
#include "neopop.h"

/* display structure */
SDL_Surface *disp;
/* SDL Surface containing the data returned from neopop Core */
SDL_Surface *corescr;
 
BOOL
system_graphics_init(void)
{
    SDL_Surface *scr;
    SDL_Surface *corescr;
    Uint32 pixel;
    Uint8 r, g, b;
    Uint32 rm, gm, bm, am;

    if ((disp=SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 16,
			       SDL_HWSURFACE|SDL_DOUBLEBUF)) == NULL) {
	fprintf(stderr, "cannot create main window: %s\n", SDL_GetError());
	return FALSE;
    }

    /* set window caption */
    SDL_WM_SetCaption(PROGRAM_NAME, NULL);

#if 0
    /* make red screen */
    pixel = SDL_MapRGB(disp->format, 0xff, 0, 0);
    SDL_LockSurface(disp);
    SDL_FillRect(disp, NULL, pixel);
    SDL_UnlockSurface(disp);
    SDL_Flip(disp);

    sleep(4);
#endif

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rm = 0x0f00;
    gm = 0x00f0;
    bm = 0x000f;
#else
    rm = 0x00f0;
    gm = 0x0f00;
    bm = 0xf000;
#endif
    corescr = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT,
				   16, rm, gm, bm, 0);
    if(corescr == NULL) {
        fprintf(stderr, "CreateRGBSurface failed: %s", SDL_GetError());
	return FALSE;
    }
    free(corescr->pixels);
    corescr->pixels = cfb;

    pixel = SDL_MapRGB(corescr->format, 0, 0xff, 0);
    SDL_FillRect(corescr, NULL, pixel);

    SDL_BlitSurface(corescr, NULL, disp, NULL);
    SDL_Flip(disp);

    return TRUE;
}

void
system_graphics_update(void)
{

    /* update screen */
    SDL_BlitSurface(corescr, NULL, disp, NULL);
    SDL_Flip(disp);
}
