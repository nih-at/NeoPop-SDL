#include <unistd.h>
#include <SDL.h>
#include "neopop-SDL.h"

#define VERSION "0.01"

char *prg;
_u8 system_frameskip_key = 1;
int do_exit = 0;


void
usage(int exitcode)
{
    printf(PROGRAM_NAME " (SDL) " NEOPOP_VERSION " (" VERSION ")\n");
    printf("Usage: %s [-hv] [game]\n"
	   "\t-h\tshow short help\n"
	   "\t-v\tshow version number\n", prg);

    exit(exitcode);
}

void
system_message(char *vaMessage, ...)
{
    va_list vl;

    va_start(vl, vaMessage);
    vprintf(vaMessage, vl);
    va_end(vl);
    printf("\n");
}

void
system_VBL(void)
{
    system_graphics_update();

    system_input_update();

    if (mute == FALSE)
	system_sound_update();
    
#if 0
    /* XXX: do throttling */
    /* XXX: show frame rate */
#endif

    return;
}

int
main(int argc, char *argv[])
{
    int ch;
    int i;

    prg = argv[0];

    while ((ch=getopt(argc, argv, "hv")) != -1) {
	switch (ch) {
	case 'h':
	    usage(1);
	    break;
	case 'v':
	    usage(0);
	    break;
	default:
	    usage(1);
	}
    }

    argc -= optind;
    argv += optind;

    /* Fill BIOS buffer */
    if (bios_install() == FALSE) {
	fprintf(stderr, "cannot install BIOS\n");
	exit(1);
    }

    /* initialize SDL and create standard-sized surface */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
       fprintf(stderr, "cannot initialize SDL: %s\n", SDL_GetError());
       exit(1);
    }
    atexit(SDL_Quit);

    /* auto-select colour mode */
    system_colour = COLOURMODE_AUTO;
    /* default to English as language for now */
    language_english = TRUE;
    /* default to sound on */
    mute = FALSE;

    if (system_graphics_init() == FALSE) {
	fprintf(stderr, "cannot create window: %s\n", SDL_GetError());
	exit(1);
    }

    if (system_sound_init() == FALSE) {
	fprintf(stderr, "cannot turn on sound: %s\n", SDL_GetError());
	mute = TRUE;
    }

    if (argc > 0) {
	if (system_rom_load(argv[0]) == FALSE)
	    exit(1);
    }

    reset();
    SDL_PauseAudio(0);

    i = 0;
    do {
	i++;
	emulate();
	if (mute == FALSE && i == 200) {
	    SDL_Delay(1);
	    i = 0;
	}
    } while (do_exit == 0);

    system_rom_unload();
    system_sound_shutdown();

    return 0;
}
