#include <SDL.h>

#include "neopop-SDL.h"

#define CHIPBUFFERLENGTH	35280
#define DACBUFFERLENGTH		3200
#define DEFAULT_SAMPLERATE	44100
#define DEFAULT_CHANNELS	1
#define DEFAULT_SAMPLES		32000

#ifndef MAX
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#endif

void
system_sound_chipreset(void)
{

    return;
}

BOOL
system_sound_init(void)
{
    SDL_AudioSpec desired;

    memset(&desired, '\0', sizeof(desired));

    /* XXX: sound support */
    desired.freq = DEFAULT_SAMPLERATE;
    desired.channels = DEFAULT_CHANNELS;
    /* XXX: True (for both LE and BE)? */
    desired.format = AUDIO_S16LSB;
    /* following may need to be set higher power of two */
    desired.samples = DEFAULT_SAMPLES;
    desired.callback = system_sound_update;
    desired.userdata = NULL;

    if (SDL_OpenAudio(&desired, NULL) == -1) {
       fprintf(stderr, "Cannot initialize audio: %s\n", SDL_GetError());
       return FALSE;
    }

    sound_init(DEFAULT_SAMPLERATE);

    return TRUE;
}

void
system_sound_shutdown(void)
{

    SDL_CloseAudio();
    return;
}

void
system_sound_silence(void)
{

    SDL_PauseAudio(1);
    return;
}

void
system_sound_update(void *userdata, Uint8 *stream, int len)
{

    printf("system_sound_update called\n");
    sound_update(stream, len);
}
