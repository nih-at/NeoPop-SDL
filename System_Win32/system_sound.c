//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

/*
//---------------------------------------------------------------------------
//=========================================================================

	system_sound.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

26 JUL 2002 - neopop_uk
=======================================
- Offset the initial write pointer to prevent corruption.
	Seems to have improved things quite a bit.

31 JUL 2002 - neopop_uk
=======================================
- Converted sound buffer to use unsigned data.

01 AUG 2002 - neopop_uk
=======================================
- Changed the sound buffer length to be an integral number of frames.
- Added a #define for the mix-ahead value (write pointer offset)

03 AUG 2002 - neopop_uk
=======================================
- Changed the DAC code so that it uses a seperate buffer, this way
the frequency can be controlled precisely. 8 khz sounds right.
- DAC buffer is 8 bit. Chip buffer is still 16 bit.
- Converted DAC to mono, i've never seen it used in stereo.

08 AUG 2002 - neopop_uk
=======================================
- Re-written buffering using dynamic block length
- Replaced sound start and stop with 'system_sound_clear'.
- system_sound_init() now calls system_sound_clear()

08 AUG 2002 - neopop_uk
=======================================
- Fixed a crash bug by making system_sound_shutdown set ptr's to NULL
- Fixed the DAC buffer size

09 AUG 2002 - neopop_uk
=======================================
- Reduced latency.
- Optimised the UNDEFINED state to ensure DAC stays in sync.
- The chip sound buffer is always stereo format.

16 AUG 2002 - neopop_uk
=======================================
- The audio stream generation is always mono, but I've had
better results with a stereo buffer, so the data is doubled up.

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include <windows.h>
#include <mmsystem.h>
#define DIRECTSOUND_VERSION 0x0500
#include <dsound.h>

#include "system_main.h"
#include "system_sound.h"
#include "system_config.h"

//=============================================================================

static LPDIRECTSOUND ds = NULL;					// DirectSound Object
static LPDIRECTSOUNDBUFFER primaryBuffer = NULL;	// Primary Buffer

#define CHIP_FREQUENCY		44100

#define CHIPBUFFERLENGTH	35280

#define DACBUFFERLENGTH		3200

#define UNDEFINED		0xFFFFFF

_u8* block;		// Gets filled with sound data.

// ====== Chip sound =========
static LPDIRECTSOUNDBUFFER chipBuffer = NULL;	// Chip Buffer
static int lastChipWrite = 0, chipWrite = UNDEFINED;	//Write Cursor

// ====== DAC sound =========
static LPDIRECTSOUNDBUFFER dacBuffer = NULL;	// DAC Buffer
static int lastDacWrite = 0, dacWrite = UNDEFINED;		//Write Cursor

//=============================================================================

void system_sound_update(void)
{
	int		pdwAudioBytes1, pdwAudioBytes2, count;
	int		Write, LengthBytes;
	_u16	*chipPtr1, *chipPtr2, *src16, *dest16;
	_u8		*dacPtr1, *dacPtr2, *src8, *dest8;

	IDirectSoundBuffer_GetCurrentPosition(chipBuffer, NULL, &Write);

	// UNDEFINED write cursors
	if (chipWrite == UNDEFINED || dacWrite == UNDEFINED)
	{
		lastChipWrite = chipWrite = Write;

		// Get DAC position too.
		IDirectSoundBuffer_GetCurrentPosition(dacBuffer, NULL, &Write);
		lastDacWrite = dacWrite = Write;

		return; //Wait a frame to accumulate length.
	}

	//Chip -> Direct Sound
	//====================

	if (Write < lastChipWrite)	//Wrap?
		lastChipWrite -= CHIPBUFFERLENGTH;

	LengthBytes = Write - lastChipWrite;
	lastChipWrite = Write;

	sound_update((_u16*)block, LengthBytes>>1);	//Get sound data

	if SUCCEEDED(IDirectSoundBuffer_Lock(chipBuffer, 
		chipWrite, LengthBytes, &chipPtr1, &pdwAudioBytes1, 
		&chipPtr2, &pdwAudioBytes2, 0))
	{
		src16 = (_u16*)block;	//Copy from this buffer

		dest16 = chipPtr1;
		count = pdwAudioBytes1 >> 2;
		while(count)
		{ 
			*dest16++ = *src16;
			*dest16++ = *src16++; count--; 
		}

		//Buffer Wrap?
		if (chipPtr2)
		{
			dest16 = chipPtr2;
			count = pdwAudioBytes2 >> 2;
			while(count)
			{ 
				*dest16++ = *src16;
				*dest16++ = *src16++; count--; 
			}
		}
		
		IDirectSoundBuffer_Unlock(chipBuffer, 
			chipPtr1, pdwAudioBytes1, chipPtr2, pdwAudioBytes2);

		chipWrite += LengthBytes;
		if (chipWrite > CHIPBUFFERLENGTH)
			chipWrite -= CHIPBUFFERLENGTH;
	}
	else
	{
		DWORD status;
		chipWrite = UNDEFINED;
		IDirectSoundBuffer_GetStatus(chipBuffer, &status);
		if (status & DSBSTATUS_BUFFERLOST)
		{
			if (IDirectSoundBuffer_Restore(chipBuffer) != DS_OK) return;
			if (!mute) IDirectSoundBuffer_Play(chipBuffer, 0, 0, DSBPLAY_LOOPING);
		}
	}

	//DAC -> Direct Sound
	//===================
	IDirectSoundBuffer_GetCurrentPosition(dacBuffer, NULL, &Write);

	if (Write < lastDacWrite)	//Wrap?
		lastDacWrite -= DACBUFFERLENGTH;

	LengthBytes = (Write - lastDacWrite); 
	lastDacWrite = Write;

	dac_update(block, LengthBytes);	//Get DAC data

	if SUCCEEDED(IDirectSoundBuffer_Lock(dacBuffer, 
		dacWrite, LengthBytes, &dacPtr1, &pdwAudioBytes1, 
		&dacPtr2, &pdwAudioBytes2, 0))
	{
		src8 = block;	//Copy from this buffer

		dest8 = dacPtr1;
		count = pdwAudioBytes1;
		while(count)
		{ 
		   *dest8++ = *src8++; 
		   count--;
		}

		//Buffer Wrap?
		if (dacPtr2)
		{
		   dest8 = dacPtr2;
		   count = pdwAudioBytes2;
		   while(count)
		   { 
			   *dest8++ = *src8++; 
			   count--;
		   }
		}
		
		IDirectSoundBuffer_Unlock(dacBuffer, 
			dacPtr1, pdwAudioBytes1, dacPtr2, pdwAudioBytes2);

		dacWrite += LengthBytes;
		if (dacWrite >= DACBUFFERLENGTH)
			dacWrite -= DACBUFFERLENGTH;
	}
	else
	{
		DWORD status;
		dacWrite = UNDEFINED;
		IDirectSoundBuffer_GetStatus(dacBuffer, &status);
		if (status & DSBSTATUS_BUFFERLOST)
		{
			if (IDirectSoundBuffer_Restore(dacBuffer) != DS_OK) return;
			if (!mute) IDirectSoundBuffer_Play(dacBuffer, 0, 0, DSBPLAY_LOOPING);
		}
	}
}

//=============================================================================

BOOL system_sound_init(void)
{
	WAVEFORMATEX wfx;
	DSBUFFERDESC dsbdesc;

    if FAILED(DirectSoundCreate(NULL, &ds, NULL))
		return FALSE;
 
    // Set co-op level
	if FAILED(IDirectSound_SetCooperativeLevel(ds, g_hWnd, DSSCL_PRIORITY ))
		if FAILED(IDirectSound_SetCooperativeLevel(ds, g_hWnd, DSSCL_NORMAL ))
				return FALSE;

	//Create a primary buffer
	//=======================

	// Set up DSBUFFERDESC structure.
	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	dsbdesc.dwBufferBytes = 0;
	dsbdesc.lpwfxFormat = NULL;

	if FAILED(IDirectSound_CreateSoundBuffer(ds, &dsbdesc, &primaryBuffer, NULL))
		return FALSE;
 
    // Set primary buffer format
    memset(&wfx, 0, sizeof(WAVEFORMATEX)); 
    wfx.wFormatTag = WAVE_FORMAT_PCM; 
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = CHIP_FREQUENCY;	
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.wBitsPerSample / 8) * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

 	if FAILED(IDirectSoundBuffer_SetFormat(primaryBuffer, &wfx))
		return FALSE;

	//Create a chip buffer
	//=========================
	
	// Set up DSBUFFERDESC structure.
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2;
	dsbdesc.lpwfxFormat = &wfx; //Use the primary buffer format
	dsbdesc.dwBufferBytes = CHIPBUFFERLENGTH;

	// Try to create a secondary buffer.
	if FAILED(IDirectSound_CreateSoundBuffer(ds, &dsbdesc, &chipBuffer, NULL))
		return FALSE;

	//Create a DAC buffer
	//=========================
	
	// Set DAC buffer format
    memset(&wfx, 0, sizeof(WAVEFORMATEX)); 
    wfx.wFormatTag = WAVE_FORMAT_PCM; 
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = DAC_FREQUENCY;	
    wfx.wBitsPerSample = 8;
    wfx.nBlockAlign = (wfx.wBitsPerSample / 8) * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	// Set up DSBUFFERDESC structure.
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2;
	dsbdesc.lpwfxFormat = &wfx;
	dsbdesc.dwBufferBytes = DACBUFFERLENGTH;

	// Try to create a secondary buffer.
	if FAILED(IDirectSound_CreateSoundBuffer(ds, &dsbdesc, &dacBuffer, NULL))
		return FALSE;

	//=========================

	//Allocate a buffer to fill with sound samples.
	block = (_u8*)malloc(max(CHIPBUFFERLENGTH, DACBUFFERLENGTH));
	
	system_sound_chipreset();	//Resets chips
	system_sound_silence();	// Clear buffers and starts playing them.

	//Loop primary buffer for increased sound quality.
	if FAILED(IDirectSoundBuffer_Play(primaryBuffer, 0,0, DSBPLAY_LOOPING ))
		return FALSE;

	return TRUE;
}

//=============================================================================

void system_sound_chipreset(void)
{
	//Initialises sound chips, matching frequncies
	sound_init(CHIP_FREQUENCY);
}
	
void system_sound_silence(void)
{
	BYTE	*ppvAudioPtr1, *ppvAudioPtr2;
	DWORD	pdwAudioBytes1, pdwAudioBytes2;

	chipWrite = UNDEFINED;
	dacWrite = UNDEFINED; 

	if (chipBuffer)
	{
		IDirectSoundBuffer_Stop(chipBuffer);

		// Fill the sound buffer
		if SUCCEEDED(IDirectSoundBuffer_Lock(chipBuffer, 0, 0, 
			&ppvAudioPtr1, &pdwAudioBytes1, 
			&ppvAudioPtr2, &pdwAudioBytes2, DSBLOCK_ENTIREBUFFER))
		{
			if (ppvAudioPtr1 && pdwAudioBytes1)
				memset(ppvAudioPtr1, 0, pdwAudioBytes1);

			if (ppvAudioPtr2 && pdwAudioBytes2)
				memset(ppvAudioPtr2, 0, pdwAudioBytes2);
			
			IDirectSoundBuffer_Unlock(chipBuffer, 
				ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2);
		}

		//Start playing
		if (mute == FALSE)
			IDirectSoundBuffer_Play(chipBuffer, 0,0, DSBPLAY_LOOPING );
	}

	if (dacBuffer)
	{
		IDirectSoundBuffer_Stop(dacBuffer);

		// Fill the sound buffer
		if SUCCEEDED(IDirectSoundBuffer_Lock(dacBuffer, 0, 0, 
			&ppvAudioPtr1, &pdwAudioBytes1, 
			&ppvAudioPtr2, &pdwAudioBytes2, DSBLOCK_ENTIREBUFFER))
		{
			if (ppvAudioPtr1 && pdwAudioBytes1)
				memset(ppvAudioPtr1, 0x80, pdwAudioBytes1);

			if (ppvAudioPtr2 && pdwAudioBytes2)
				memset(ppvAudioPtr2, 0x80, pdwAudioBytes2);
			
			IDirectSoundBuffer_Unlock(dacBuffer, 
				ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2);
		}

		//Start playing
		if (mute == FALSE)
			IDirectSoundBuffer_Play(dacBuffer, 0,0, DSBPLAY_LOOPING );
	}
}

//=============================================================================

void system_sound_shutdown(void)
{
	if (chipBuffer)	IDirectSoundBuffer_Stop(chipBuffer);
	if (chipBuffer)	IDirectSoundBuffer_Release(chipBuffer);
	chipBuffer = NULL;

	if (dacBuffer)	IDirectSoundBuffer_Stop(dacBuffer);
	if (dacBuffer)	IDirectSoundBuffer_Release(dacBuffer);
	dacBuffer = NULL;

	if (primaryBuffer)		IDirectSoundBuffer_Stop(primaryBuffer);
	if (primaryBuffer)		IDirectSoundBuffer_Release(primaryBuffer);

	if (ds)	IDirectSound_Release(ds);

	free(block);
	block = NULL;
}

//=============================================================================
