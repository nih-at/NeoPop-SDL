#include <SDL.h>
#include "neopop-SDL.h"

/* display structure */
static SDL_Surface *disp;
/* SDL Surface containing the data returned from neopop Core */
static SDL_Surface *corescr;
 
BOOL
system_graphics_init(void)
{
    Uint32 pixel;
    Uint32 rm, gm, bm;

    if ((disp=SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 16,
			       SDL_HWSURFACE|SDL_DOUBLEBUF)) == NULL) {
	fprintf(stderr, "cannot create main window: %s\n", SDL_GetError());
	return FALSE;
    }

    /* set window caption */
    SDL_WM_SetCaption(PROGRAM_NAME, NULL);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rm = 0x0f00;
    gm = 0x00f0;
    bm = 0x000f;
#else
    rm = 0x000f;
    gm = 0x00f0;
    bm = 0x0f00;
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
