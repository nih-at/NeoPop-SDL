/* $NiH: system_main.c,v 1.31 2004/06/23 17:19:04 dillo Exp $ */
/*
  system_main.c -- main program
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

#include <sys/time.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include "config.h"
#include "NeoPop-SDL.h"

#define NGP_FPS 59.95

char *prg;
struct timeval throttle_last;
_u8 system_frameskip_key;
_u32 throttle_rate;
int do_exit = 0;

void readrc(void);

static void
printversion(void)
{
    printf(PROGRAM_NAME " (SDL) " NEOPOP_VERSION " (SDL-Version "
	   VERSION ")\n");
}

static void
usage(int exitcode)
{
    printversion();
    printf("NeoGeo Pocket emulator\n\n"
	   "Usage: %s [-cefghjMmSsv] [-C mode] [-P port] [-R remove] [game]\n"
	   "\t-C mode\tspecify comms mode (none, server, client; default: none)\n"
	   "\t-c\t\tstart in colour mode (default: automatic)\n"
	   "\t-e\t\temulate English language NeoGeo Pocket (default)\n"
	   "\t-f count\tframeskip: show one in `count' frames (default: 1)\n"
	   "\t-g\t\tstart in greyscale mode (default: automatic)\n"
	   "\t-h\t\tshow this short help\n"
	   "\t-j\t\temulate Japanese language NeoGeo Pocket\n"
	   "\t-l state\tload start state from file `state'\n"
	   "\t-M\t\tdo not use smoothed magnification modes\n"
	   "\t-m\t\tuse smoothed magnification modes (default)\n"
	   "\t-P port\tspecify port number to use for comms (default: 7846)\n"
	   "\t-R host\tspecify host to connect to as comms client\n"
	   "\t-S\t\tsilent mode\n"
	   "\t-s\t\twith sound (default)\n"
	   "\t-v\t\tshow version number\n", prg);

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
    static int frame_counter = 0;
    struct timeval current_time;
    long throttle_diff;
    struct timespec ts, tsrem;

    system_graphics_update();

    system_input_update();

    if (mute == FALSE)
	system_sound_update();

    /* throttling */
    throttle_last.tv_usec += throttle_rate;
    if (throttle_last.tv_usec > 1000000) {
	throttle_last.tv_sec += throttle_last.tv_usec/1000000;
	throttle_last.tv_usec %= 1000000;
    }
    gettimeofday(&current_time, NULL);
    throttle_diff = (throttle_last.tv_sec - current_time.tv_sec) * 1000000
	+ (throttle_last.tv_usec - current_time.tv_usec);
    
#if 0
    printf("cur: %ld.%06ld, next: %ld.%06ld, sleep: %ld\n",
	   current_time.tv_sec, current_time.tv_usec,
	   throttle_last.tv_sec, throttle_last.tv_usec,
	   throttle_diff);
#endif
    
    if (throttle_diff < 0)
	throttle_last = current_time;
    else {
	ts.tv_sec = throttle_diff / 1000000000;
	ts.tv_nsec = throttle_diff % 1000000000;
	while (nanosleep(&ts, &tsrem) < 0 && errno == EINTR)
	    ts = tsrem;
    }
    
    if (frame_counter++ > NGP_FPS) {
	char title[128];
	int fps;

	frame_counter = 0;
	fps = (int)(1000000/(throttle_rate-throttle_diff+.5);

	(void)snprintf(title, sizeof(title), PROGRAM_NAME " - %s - %dfps/FS%d",
		       rom.name, fps, system_frameskip_key);

	/* set window caption */
	SDL_WM_SetCaption(title, NULL);
    }

    return;
}

int
main(int argc, char *argv[])
{
    char *start_state;
    int ch;
    int i;

    prg = argv[0];
    start_state = NULL;

    /* some defaults, to be changed by getopt args */
    /* auto-select colour mode */
    system_colour = COLOURMODE_AUTO;
    /* default to English as language for now */
    language_english = TRUE;
    /* default to smooth graphics magnification */
    graphics_mag_smooth = 1;
    /* default to sound on */
    mute = FALSE;
    /* show every frame */
    system_frameskip_key = 1;
    /* interlink defaults (port taken from System_Win32) */
    comms_mode = COMMS_NONE;
    comms_port = 7846;
    comms_host = NULL;

    system_bindings_init();
    system_rc_read();

    while ((ch=getopt(argc, argv, "C:cef:ghjl:MmP:R:SsV")) != -1) {
	switch (ch) {
	case 'C':
	    i = system_rc_parse_comms_mode(optarg);
	    if (i == -1) {
		/* XXX: error message */
	    }
	    else
		comms_mode = i;
	    break;
	case 'c':
	    system_colour = COLOURMODE_COLOUR;
	    break;
	case 'e':
	    language_english = TRUE;
	    break;
	case 'f':
	    i = atoi(optarg);
	    if (i >=1 && i <= 7)
		system_frameskip_key = i;
	    break;
	case 'g':
	    system_colour = COLOURMODE_GREYSCALE;
	    break;
	case 'h':
	    usage(1);
	    break;
	case 'j':
	    language_english = FALSE;
	    break;
	case 'l':
	    start_state = optarg;
	    break;
	case 'M':
	    graphics_mag_smooth = 0;
	    break;
	case 'm':
	    graphics_mag_smooth = 1;
	    break;
	case 'P':
	    i = atoi(optarg);
	    if (i == 0) {
		/* XXX: error message */
	    }
	    else
		comms_port = i;
	    break;
	case 'R':
	    if (comms_host)
		free(comms_host);
	    comms_host = strdup(optarg);
	    break;
	case 'S':
	    mute = TRUE;
	    break;
	case 's':
	    mute = FALSE;
	    break;
	case 'V':
	    printversion();
	    exit(0);
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
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) < 0) {
       fprintf(stderr, "cannot initialize SDL: %s\n", SDL_GetError());
       exit(1);
    }
    atexit(SDL_Quit);

    if (SDL_NumJoysticks() > 0) {
	SDL_JoystickOpen(0);
	SDL_JoystickEventState(SDL_ENABLE);
    }

    if (system_graphics_init() == FALSE) {
	fprintf(stderr, "cannot create window: %s\n", SDL_GetError());
	exit(1);
    }

    if (mute == FALSE && system_sound_init() == FALSE) {
	fprintf(stderr, "cannot turn on sound: %s\n", SDL_GetError());
	mute = TRUE;
    }

    if (comms_mode != COMMS_NONE)
	system_comms_connect();

    /*
     * Throttle rate is number_of_ticks_per_second divided by number
     * of complete frames that should be shown per second.
     * In this case, both are constants;
     */
    throttle_rate = 1000000/NGP_FPS;

    if (argc > 0) {
	if (system_rom_load(argv[0]) == FALSE)
	    exit(1);
    }
    else {
	fprintf(stderr, "no ROM loaded\n");
    }

    reset();
    SDL_PauseAudio(0);
    if (start_state != NULL)
	state_restore(start_state);

    gettimeofday(&throttle_last, NULL);
    i = 200;
    do {
	emulate();
	if (i-- == 0 && !mute) {
	    /* for the sound thread */
	    SDL_Delay(1);
	    i = 200;
	}
    } while (do_exit == 0);

    system_rom_unload();
    system_sound_shutdown();

    return 0;
}
