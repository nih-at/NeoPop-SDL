#include <SDL_mixer.h>

#include "neopop.h"

#define CHIPBUFFERLENGTH	35280
#define DACBUFFERLENGTH		3200
#define DEFAULT_SAMPLERATE	44100
#define DEFAULT_CHANNELS	2
#define DEFAULT_CHUNKSIZE	4096

#ifndef MAX
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#endif

static unsigned char *sound_data;

void
system_sound_chipreset(void)
{

    (void)Mix_HaltMusic();
    return;
}

BOOL
system_sound_init(void)
{

    /* 44.1 kHz, S16*SB, 2-channel, 16-bit */
    if (Mix_OpenAudio(DEFAULT_SAMPLERATE, MIX_DEFAULT_FORMAT,
		      DEFAULT_CHANNELS, DEFAULT_CHUNKSIZE) < 0)
	return FALSE;
    else {
	int rate, channels;
	Uint16 format;

	Mix_QuerySpec(&rate, &format, &channels);
	printf("Opened audio at %dHz %d-bit %s\n", rate,
	       format&0xFF, channels > 1 ? "stereo" : "mono");
    }

    if (Mix_AllocateChannels(2) < 0)
	return FALSE;

    if ((sound_data=(unsigned char *)malloc(MAX(CHIPBUFFERLENGTH,
						DACBUFFERLENGTH))) == NULL) {
	Mix_CloseAudio();
	return FALSE;
    }

    sound_init(DEFAULT_SAMPLERATE);

    return TRUE;
}

void
system_sound_shutdown(void)
{

    Mix_CloseAudio();
    free(sound_data);
    sound_data = NULL;
    return;
}

void
system_sound_silence(void)
{

    (void)Mix_HaltMusic();
    return;
}

void
system_sound_update(void)
{
    Mix_Chunk *sdchunk;

    sound_update((_u16*)sound_data, DEFAULT_CHUNKSIZE);
    sdchunk = Mix_QuickLoad_RAW(sound_data, DEFAULT_CHUNKSIZE);
    Mix_PlayChannel(0, sdchunk, 0);
#if 0
    Mix_FreeChunk(sdchunk);
#endif

    dac_update(sound_data, DEFAULT_CHUNKSIZE);
    sdchunk = Mix_QuickLoad_RAW(sound_data, DEFAULT_CHUNKSIZE);
    Mix_PlayChannel(1, sdchunk, 0);
    Mix_FreeChunk(sdchunk);
}
