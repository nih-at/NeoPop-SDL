#include <SDL_audio.h>

#include "neopop.h"

void
system_sound_callback(void *userdata, Uint8 *stream, int len)
{
    /* XXX: DAC update? */
    sound_update((_u16 *)stream, len);

    return;
}

/* dummy function for now */
BOOL
system_sound_init(void)
{
#ifdef I_WANT_UNTESTED_AUDIO
    SDL_AudioSpec desired;
    SDL_AudioSpec obtained;

    memset(&desired, '\0', sizeof(desired));

    /* XXX: DAC support */

    desired.freq = 44100;
    desired.channels = 2;
    /* XXX: True (for both LE and BE)? */
    desired.format = AUDIO_S16LSB;
    /* following may need to be set higher power of two */
    desired.samples = 512;
    desired.callback = system_sound_callback;
    desired.userdata = NULL;

    if (SDL_OpenAudio(&desired, &obtained) == -1) {
	fprintf(stderr, "Cannot initialize audio: %s\n", SDL_GetError());
	return FALSE;
    }

    return TRUE;
#else
    return FALSE;
#endif
}

/* dummy function for now */
void
system_sound_chipreset(void)
{
    return;
}

/* dummy function for now */
void
system_sound_silence(void)
{
    return;
}
