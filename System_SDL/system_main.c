#include <SDL.h>
#include "neopop.h"

/* display structure */
SDL_Surface *disp;
/* SDL Surface containing the data returned from neopop Core */
SDL_Surface *corescr;

/* dummy functions for now */
void
system_sound_clear(void)
{
    return;
}

/* copied from Win32/system_language.c */
typedef struct {
    char label[9];
    char string[256];
} STRING_TAG;

static STRING_TAG string_tags[]={
    "SDEFAULT",     "Are you sure you want to revert to the default control setup?",
    "ROMFILT",      "Rom Files (*.ngp,*.ngc,*.npc,*.zip)\0*.ngp;*.ngc;*.npc;*.zip\0\0", 
    "STAFILT",      "State Files (*.ngs)\0*.ngs\0\0",
    "FLAFILT",      "Flash Memory Files (*.ngf)\0*.ngf\0\0",
    "BADFLASH",     "The flash data for this rom is from a different version of NeoPop, it will be destroyed soon.",
    "POWER",        "The system has been signalled to power down. You must reset or load a new rom.",
    "BADSTATE",     "State is from an unsupported version of NeoPop.",
    "ERROR1",       "An error has occured creating the application window",
    "ERROR2",       "An error has occured initialising DirectDraw",
    "ERROR3",       "An error has occured initialising DirectInput",
    "TIMER",        "This system does not have a high resolution timer.",
    "WRONGROM",     "This state is from a different rom, Ignoring.",
    "EROMFIND",     "Cannot find ROM file",
    "EROMOPEN",     "Cannot open ROM file",
    "EZIPNONE",     "No roms found",
    "EZIPBAD",      "Corrupted ZIP file",
    "EZIPFIND",     "Cannot find ZIP file",
};

char *
system_get_string(STRINGS string_id)
{
    if (string_id >= STRINGS_MAX)
	return "Unknown String";

    return string_tags[string_id].string;
}
 
void
system_message(char *vaMessage, ...)
{
    va_list vl;

    va_start(vl, vaMessage);
    vprintf(vaMessage, vl);
    va_end(vl);
}

void
system_graphics_update(void)
{

    SDL_BlitSurface(corescr, NULL, disp, NULL);
    SDL_Flip(disp);
}
         
void
system_VBL(void)
{
    /* update screen */
    return;
}

int
main(int argc, char *argv[])
{
    SDL_Surface *scr;
    SDL_Surface *corescr;
    Uint32 pixel;
    Uint8 r, g, b;
    Uint32 rm, gm, bm, am;

    /* initialize SDL and create standard-sized surface */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
       fprintf(stderr, "cannot initialize SDL: %s\n", SDL_GetError());
       exit(1);
    }
    if ((disp=SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 16,
			      SDL_HWSURFACE|SDL_DOUBLEBUF)) == NULL) {
	fprintf(stderr, "cannot create main window: %s\n", SDL_GetError());
    }

    /* set window caption */
    SDL_WM_SetCaption(PROGRAM_NAME, NULL);

    /* make red screen */
    pixel = SDL_MapRGB(disp->format, 0xff, 0, 0);
    SDL_LockSurface(disp);
    SDL_FillRect(disp, NULL, pixel);
    SDL_UnlockSurface(disp);
    SDL_Flip(disp);

    sleep(4);

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
        exit(1);
    }
    free(corescr->pixels);
    corescr->pixels = cfb;

#if 0
    pixel = SDL_MapRGB(corescr->format, 0, 0xff, 0);
    SDL_FillRect(corescr, NULL, pixel);
#endif

    SDL_BlitSurface(corescr, NULL, disp, NULL);
    SDL_Flip(disp);

    sleep(4);
    atexit(SDL_Quit);
}
