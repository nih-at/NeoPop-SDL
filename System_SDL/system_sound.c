/* $NiH: system_sound.c,v 1.17 2004/06/23 21:35:44 dillo Exp $ */
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

#define DEFAULT_FORMAT		AUDIO_U16SYS
#define DEFAULT_CHANNELS	1
/* following may need to be set higher power of two */

int samplerate;		/* desired output sample rate */
int spf;		/* samples per frame */
int bpf;		/* bytes per frame */
int dac_bpf;		/* bytes of DAC data per frame */

static SDL_AudioCVT acvt;
static Uint8 silence_value;

static char *sound_buffer;
static char *dac_data;
static int sound_read;

static SDL_sem *rsem, *wsem;

void
system_sound_chipreset(void)
{

    sound_init(samplerate);
    return;
}

BOOL
system_sound_init(void)
{
    SDL_AudioSpec desired;

    spf = samplerate/NGP_FPS;
    bpf = spf*2;

    rsem = SDL_CreateSemaphore(0);
    wsem = SDL_CreateSemaphore(0);

    memset(&desired, '\0', sizeof(desired));

    desired.freq = samplerate;
    desired.channels = DEFAULT_CHANNELS;
    desired.format = DEFAULT_FORMAT;
    desired.samples = spf;
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
			  DEFAULT_CHANNELS, samplerate) == -1) {
	fprintf(stderr, "Cannot build converter: %s\n", SDL_GetError());
	SDL_CloseAudio();
	return FALSE;
    }

    dac_bpf = (bpf+acvt.len_mult-1)/acvt.len_mult;

    if ((sound_buffer=malloc(bpf)) == NULL) {
	fprintf(stderr, "Cannot allocate sound buffer (%d bytes)\n",
		bpf);
	SDL_CloseAudio();
	return FALSE;
    }
    if ((dac_data=malloc(dac_bpf*acvt.len_mult)) == NULL) {
	fprintf(stderr, "Cannot allocate sound buffer (%d bytes)\n",
		bpf+1);
	SDL_CloseAudio();
	free(sound_buffer);
	return FALSE;
    }

    sound_init(samplerate);
    silence_value = desired.silence;
    sound_read = 1;

    return TRUE;
}

void
system_sound_shutdown(void)
{
    SDL_SemPost(rsem);
    SDL_CloseAudio();
    SDL_DestroySemaphore(rsem);
    SDL_DestroySemaphore(wsem);
    free(sound_buffer);
    sound_buffer = NULL;

    return;
}

void
system_sound_silence(void)
{

    if (mute == TRUE)
	return;

    memset(sound_buffer, silence_value, bpf);
    return;
}

void
system_sound_callback(void *userdata, Uint8 *stream, int len)
{
#if 0
    /* plays sound when host too slow, but bad sound even if fast enough */

    static _u16 ls;
    _u16 *p;
#endif

    if (sound_buffer == NULL)
	return;

#if 0
    /* plays sound when host too slow, but bad sound even if fast enough */

    if (SDL_SemTryWait(rsem) == 0) {
	memcpy(stream, sound_buffer, len);
	ls = *(_u16 *)(sound_buffer+len-2);
	sound_read = 1;
    }
    else {
	p = (_u16 *)stream;
	while (len-- > 0)
	    *(p++) = ls;
    }
#else
    SDL_SemWait(rsem);
    sound_read = 1;
    memcpy(stream, sound_buffer, len);
#endif
    SDL_SemPost(wsem);
}

void
system_sound_update(void)
{
    if (mute == TRUE)
	return;

    if (sound_read) {
#if 1
	dac_update(dac_data, dac_bpf);
	/* convert to standard format */
	acvt.buf = dac_data;
	acvt.len = dac_bpf;
	if (SDL_ConvertAudio(&acvt) == -1) {
	    fprintf(stderr, "DAC data conversion failed: %s\n", SDL_GetError());
	    return;
	}
#endif
	
	/* get sound data */
	sound_update((_u16 *)sound_buffer, bpf);
	
#if 1
	/* mix both streams into one */
	SDL_MixAudio(sound_buffer, dac_data, bpf, SDL_MIX_MAXVOLUME);
#endif
	
	sound_read = 0;
	
	SDL_SemPost(rsem);
    }
    SDL_SemWait(wsem);
}
