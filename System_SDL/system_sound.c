/* $NiH: system_sound.c,v 1.16 2004/06/20 23:43:31 dillo Exp $ */
/*
  system_sound.c -- sound support functions
  Copyright (C) 2002-2003 Thomas Klausner

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
#include <assert.h>
#include "NeoPop-SDL.h"

#define CHIPBUFFERLENGTH	35280
#define DACBUFFERLENGTH		3200
#define DEFAULT_FORMAT		AUDIO_U16SYS
#define DEFAULT_SAMPLERATE	44100
#define DEFAULT_CHANNELS	1
/* following may need to be set higher power of two */
#define DEFAULT_SAMPLES		8192

static SDL_AudioCVT acvt;
static _u16 *sound_buffer;
static int sound_buffer_offset;
static Uint8 silence_value;

void
system_sound_chipreset(void)
{

    sound_init(DEFAULT_SAMPLERATE);
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

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
	fprintf(stderr, "Cannot initialize audio: %s\n", SDL_GetError());
	return FALSE;
    }
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
    silence_value = desired.silence;

    return TRUE;
}

void
system_sound_shutdown(void)
{

    SDL_CloseAudio();
    free(sound_buffer);
    sound_buffer = NULL;

    return;
}

void
system_sound_silence(void)
{

    if (mute == TRUE)
	return;

    SDL_LockAudio();
    memset(sound_buffer, silence_value, CHIPBUFFERLENGTH);
    sound_buffer_offset = 0;
    SDL_UnlockAudio();
    return;
}

void
system_sound_callback(void *userdata, Uint8 *stream, int len)
{
    assert(len != 0);
    assert(stream != NULL);

    if (sound_buffer == NULL)
	return;

    if (len > CHIPBUFFERLENGTH - sound_buffer_offset) {
	fprintf(stderr, "warn: system_sound_callback: len from %d to %d\n",
		len, CHIPBUFFERLENGTH - sound_buffer_offset);
	len = CHIPBUFFERLENGTH - sound_buffer_offset;
    }

#if 0
    printf("%p stream, %p sb, %d sbo, %d len\n", stream, sound_buffer,
	   sound_buffer_offset, len);
#endif
    memcpy(stream, sound_buffer+sound_buffer_offset, len);
    sound_buffer_offset += len;
}

void
system_sound_update(void)
{
    char dac_data[CHIPBUFFERLENGTH];

    if (mute == TRUE)
	return;

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
