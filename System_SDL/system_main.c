#include <unistd.h>
#include <SDL.h>
#include "neopop.h"

#define VERSION "0.01"

char *prg;

void
usage(int exitcode)
{
    printf(PROGRAM_NAME " (SDL) " NEOPOP_VERSION " (" VERSION ")\n");
    printf("Usage: %s [-hv]\n"
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
}
         
void
system_VBL(void)
{
    system_graphics_update();

#if 0
    system_input_update();
    if (mute == FALSE)
	system_sound_update();

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

    if (system_graphics_init() == FALSE) {
	fprintf(stderr, "cannot create window: %s\n", SDL_GetError());
	exit(1);
    }

#if NOT_YET
    if (system_input_init() == FALSE) {
	fprintf(stderr, "cannot get input devices: %s\n", SDL_GetError());
	exit(1);
    }
#endif

    if (system_sound_init() == FALSE) {
	fprintf(stderr, "cannot turn on sound: %s\n", SDL_GetError());
	mute = TRUE;
    }

    if (argc > 0) {
	system_load_rom(argv[0]);
    }    

    reset();

    i = 0;
    do {
	i++;
	emulate();
    } while (i<1000000);

    sleep(4);

    system_unload_rom();
#if 0
    system_sound_shutdown();
    system_input_shutdown();
    system_graphics_shutdown();
#endif
}
