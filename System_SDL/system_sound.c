#include <assert.h>
#include <SDL.h>

#include "neopop-SDL.h"

#define CHIPBUFFERLENGTH	35280
#define DACBUFFERLENGTH		3200
/* XXX: True (for both LE and BE)? */
#define DEFAULT_FORMAT		AUDIO_U16LSB
#define DEFAULT_SAMPLERATE	44100
#define DEFAULT_CHANNELS	1
/* following may need to be set higher (power of two?) */
#define DEFAULT_SAMPLES		8192

#ifndef MAX
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#endif

static SDL_AudioCVT acvt;
static _u16 *sound_buffer;
static int sound_buffer_offset;

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

    desired.freq = DEFAULT_SAMPLERATE;
    desired.channels = DEFAULT_CHANNELS;
    desired.format = DEFAULT_FORMAT;
    desired.samples = DEFAULT_SAMPLES;
    desired.callback = system_sound_callback;
    desired.userdata = NULL;

    if (SDL_OpenAudio(&desired, NULL) == -1) {
	fprintf(stderr, "Cannot initialize audio: %s\n", SDL_GetError());
	return FALSE;
    }

    /* build conversion structure for DAC sound data */
    if (SDL_BuildAudioCVT(&acvt, AUDIO_U8, 1, 8000, DEFAULT_FORMAT,
			  DEFAULT_CHANNELS, DEFAULT_SAMPLERATE) == -1) {
	fprintf(stderr, "Cannot build converter: %s\n", SDL_GetError());
	SDL_CloseAudio();
	return FALSE;
    }

    if ((sound_buffer=malloc(CHIPBUFFERLENGTH)) == NULL) {
	fprintf(stderr, "Cannot allocate sound buffer (%d bytes)\n",
		CHIPBUFFERLENGTH);
	SDL_CloseAudio();
	return FALSE;
    }
    /* start with empty sound buffer */
    sound_buffer_offset = CHIPBUFFERLENGTH;

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
system_sound_callback(void *userdata, Uint8 *stream, int len)
{
    assert(len != 0);
    assert(stream != NULL);

    if (len > CHIPBUFFERLENGTH - sound_buffer_offset) {
	fprintf(stderr, "warn: system_sound_callback: len from %d to %d\n",
		len, CHIPBUFFERLENGTH - sound_buffer_offset);
	len = CHIPBUFFERLENGTH - sound_buffer_offset;
    }

    memcpy(stream, sound_buffer+sound_buffer_offset, len);
    sound_buffer_offset += len;
}

void
system_sound_update(void)
{
    char dac_data[CHIPBUFFERLENGTH];

    /*
     * Differing buffer sizes because DAC data is 8-bit, 8kHz and will
     * be converted to 16-bit, 44.1kHz.
     */
    dac_update(dac_data, DACBUFFERLENGTH);
    /* convert to standard format */
    acvt.buf = dac_data;
    acvt.len = DACBUFFERLENGTH;
    if (SDL_ConvertAudio(&acvt) == -1) {
	fprintf(stderr, "DAC data conversion failed: %s\n", SDL_GetError());
	return;
    }

    SDL_LockAudio();
    /* get sound data */
    sound_update(sound_buffer, CHIPBUFFERLENGTH);

    /* mix both streams into one */
    SDL_MixAudio((_u8 *)sound_buffer, dac_data, CHIPBUFFERLENGTH,
		 SDL_MIX_MAXVOLUME);

    sound_buffer_offset = 0;

    SDL_UnlockAudio();
}
