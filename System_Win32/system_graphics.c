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

	system_graphics.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

21 JUL 2002 - neopop_uk
=======================================
- Promoted 'system_graphics_clear' with a comment heading to make it
more prominent as an emulator interface function.

06 AUG 2002 - neopop_uk
=======================================
- Optimised the graphics update, adding 6% more speed to my machine.
- Removed "system_graphics_clear" - not needed.

11 AUG 2002 - neopop_uk
=======================================
- Fixed extra blank top line in 16-bit colour mode.

22 AUG 2002 - neopop_uk
=======================================
- Added a border around the display.

27 AUG 2002 - neopop_uk
=======================================
- Added fullscreen mode.
- Optimised both modes very slightly by pre-calculating a CFB pointer, and
	performing increments instead of multiplications.

31 AUG 2002 - neopop_uk
=======================================
- Fixed the full-screen screen position, it wasn't quite central.

10 SEP 2002 - neopop_uk
=======================================
- Added left and right borders to the full-screen display so that the NVidia
filtering isn't cropped.
- Added full-screen width stretching.

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include <windows.h>
#include <ddraw.h>

#include "system_main.h"
#include "system_graphics.h"
#include "system_config.h"

//=============================================================================

static DWORD dwRMask32 = 0, dwRShiftL32 = 8, dwRShiftR32 = 0;
static DWORD dwGMask32 = 0, dwGShiftL32 = 8, dwGShiftR32 = 0;
static DWORD dwBMask32 = 0, dwBShiftL32 = 8, dwBShiftR32 = 0;
static DWORD dwAMask32 = 0, dwAShiftL32 = 8, dwAShiftR32 = 0;

static WORD dwRMask16 = 0, dwRShiftL16 = 8, dwRShiftR16 = 0;
static WORD dwGMask16 = 0, dwGShiftL16 = 8, dwGShiftR16 = 0;
static WORD dwBMask16 = 0, dwBShiftL16 = 8, dwBShiftR16 = 0;
static WORD dwAMask16 = 0, dwAShiftL16 = 8, dwAShiftR16 = 0;

static LPDIRECTDRAW            lpDD = NULL;           
static LPDIRECTDRAWSURFACE     lpDDSPrimary = NULL;   
static LPDIRECTDRAWSURFACE     lpDDSBack = NULL;   
static LPDIRECTDRAWSURFACE     lpDDSOne = NULL;

static DDSURFACEDESC ddsd;

static RECT	fsRect;

//=============================================================================

//-----------------------------------------------------------------------------
// system_graphics_ready()
//-----------------------------------------------------------------------------
BOOL system_graphics_ready(void)
{
	if (lpDDSOne)
		return TRUE;
	else
		return FALSE;
}

//-----------------------------------------------------------------------------
// system_graphics_gdi_begin()
//-----------------------------------------------------------------------------
void system_graphics_gdi_begin(void)
{
	if (windowed == FALSE)
		IDirectDraw_FlipToGDISurface(lpDD);
}

//-----------------------------------------------------------------------------
// system_graphics_gdi_end()
//-----------------------------------------------------------------------------
void system_graphics_gdi_end(void)
{
	if (windowed == FALSE)
	{
		// Clear screen
		if SUCCEEDED(IDirectDrawSurface_Lock(lpDDSPrimary, NULL, &ddsd, 
			DDLOCK_WRITEONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL))
		{
			memset(ddsd.lpSurface, 0, ddsd.lPitch * ddsd.dwHeight);
			IDirectDrawSurface_Unlock(lpDDSPrimary, NULL);	
		}

		// Clear back surface
		if SUCCEEDED(IDirectDrawSurface_Lock(lpDDSBack, NULL, &ddsd, 
			DDLOCK_WRITEONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL))
		{
			memset(ddsd.lpSurface, 0, ddsd.lPitch * ddsd.dwHeight);
			IDirectDrawSurface_Unlock(lpDDSBack, NULL);	
		}

		// Clear render target
		if SUCCEEDED(IDirectDrawSurface_Lock(lpDDSOne, NULL, &ddsd, 
			DDLOCK_WRITEONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL))
		{
			memset(ddsd.lpSurface, 0, ddsd.lPitch * ddsd.dwHeight);
			IDirectDrawSurface_Unlock(lpDDSOne, NULL);	
		}
	}
}

//=============================================================================

//-----------------------------------------------------------------------------
// system_graphics_init()
//-----------------------------------------------------------------------------
BOOL system_graphics_init(void)
{
	WORD	dwMask16;	
	DWORD	dwMask32;

	dwRMask32 = 0; dwRShiftL32 = 8; dwRShiftR32 = 0;
	dwGMask32 = 0; dwGShiftL32 = 8; dwGShiftR32 = 0;
	dwBMask32 = 0; dwBShiftL32 = 8; dwBShiftR32 = 0;
	dwAMask32 = 0; dwAShiftL32 = 8; dwAShiftR32 = 0;

	dwRMask16 = 0; dwRShiftL16 = 8; dwRShiftR16 = 0;
	dwGMask16 = 0; dwGShiftL16 = 8; dwGShiftR16 = 0;
	dwBMask16 = 0; dwBShiftL16 = 8; dwBShiftR16 = 0;
	dwAMask16 = 0; dwAShiftL16 = 8; dwAShiftR16 = 0;

	// Create DirectDraw interface
	if FAILED(DirectDrawCreate(NULL,&lpDD,NULL))
        return FALSE;

	if (windowed)
	{
		if FAILED(IDirectDraw_SetCooperativeLevel(lpDD, g_hWnd, DDSCL_NORMAL))
		    return FALSE;
	}
	else
	{
		if FAILED(IDirectDraw_SetCooperativeLevel(lpDD, g_hWnd, 
			DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN))
		    return FALSE;

		if FAILED(IDirectDraw_SetDisplayMode(lpDD, 640, 480, 16))
			return FALSE;

		fsRect.top = 0;
		fsRect.bottom = 480;

		if (misc_stetchx)
		{
			fsRect.left = BORDER_WIDTH;
			fsRect.right = 640 - BORDER_WIDTH;
		}
		else
		{
			fsRect.left = 80 - (BORDER_WIDTH*3);
			fsRect.right = ((SCREEN_WIDTH + BORDER_WIDTH) * 3) + fsRect.left;
		}
	}

	if (windowed)
	{
		LPDIRECTDRAWCLIPPER lpClipper = NULL;
 
		// Create Primary Surface
		ZeroMemory(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

		if FAILED(IDirectDraw_CreateSurface(lpDD, &ddsd, &lpDDSPrimary, NULL))
			return FALSE;

		// Create clipper
		if FAILED(IDirectDraw_CreateClipper(lpDD, 0, &lpClipper, NULL))
			return FALSE;
		
		IDirectDrawSurface_SetClipper(lpDDSPrimary, lpClipper);
		IDirectDrawClipper_SetHWnd(lpClipper, 0, g_hWnd);
		IDirectDraw_Release(lpClipper);
	}
	else
	{
 		DDSCAPS ddsCaps;

		// Create Primary Surface
 		ZeroMemory(&ddsd, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | 
			DDSCAPS_FLIP | DDSCAPS_COMPLEX; 
 		ddsd.dwBackBufferCount = 1; 

		if FAILED(IDirectDraw_CreateSurface(lpDD, &ddsd, &lpDDSPrimary, NULL))
			return FALSE;

		// Clear surface
		if SUCCEEDED(IDirectDrawSurface_Lock(lpDDSPrimary, NULL, &ddsd, 
			DDLOCK_WRITEONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL))
		{
			memset(ddsd.lpSurface, 0, ddsd.lPitch * ddsd.dwHeight);
			IDirectDrawSurface_Unlock(lpDDSPrimary, NULL);	
		}

		// Get the Back Buffer.
		ddsCaps.dwCaps = DDSCAPS_BACKBUFFER; 
		if FAILED(IDirectDrawSurface_GetAttachedSurface(lpDDSPrimary, &ddsCaps, &lpDDSBack))
			return FALSE; 

		// Clear Back Buffer surface
		if SUCCEEDED(IDirectDrawSurface_Lock(lpDDSBack, NULL, &ddsd, 
			DDLOCK_WRITEONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL))
		{
			memset(ddsd.lpSurface, 0, ddsd.lPitch * ddsd.dwHeight);
			IDirectDrawSurface_Unlock(lpDDSBack, NULL);	
		}
	}

	// Create Off-screen surface for frame-buffer updates
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
   
	if (windowed)
	{
		ddsd.dwWidth = SCREEN_WIDTH + (BORDER_WIDTH*2);
		ddsd.dwHeight = SCREEN_HEIGHT + (BORDER_HEIGHT*2);
	}
	else
	{
		ddsd.dwWidth = SCREEN_WIDTH + (BORDER_WIDTH*2);
		ddsd.dwHeight = SCREEN_HEIGHT + (BORDER_HEIGHT*2);
	}
	

	if FAILED(IDirectDraw_CreateSurface(lpDD, &ddsd, &lpDDSOne, NULL))
        return FALSE;

	// Clear screen
	if SUCCEEDED(IDirectDrawSurface_Lock(lpDDSOne, NULL, &ddsd, 
		DDLOCK_WRITEONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL))
	{
		memset(ddsd.lpSurface, 0, ddsd.lPitch * ddsd.dwHeight);
		IDirectDrawSurface_Unlock(lpDDSOne, NULL);	
	}

	//GET COLOUR BIT MASKS

	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	if FAILED(IDirectDrawSurface_Lock(lpDDSPrimary, NULL, &ddsd, 
			DDLOCK_WAIT, NULL))
		return FALSE;
	
	IDirectDrawSurface_Unlock(lpDDSPrimary, NULL);	

    dwRMask32 = ddsd.ddpfPixelFormat.dwRBitMask;
    dwGMask32 = ddsd.ddpfPixelFormat.dwGBitMask;
    dwBMask32 = ddsd.ddpfPixelFormat.dwBBitMask;
    dwAMask32 = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;
    
	dwRMask16 = (_u16)dwRMask32;
	dwGMask16 = (_u16)dwGMask32;
	dwBMask16 = (_u16)dwBMask32;
	dwAMask16 = (_u16)dwAMask32;

	//16 bit masks
	for( dwMask16=dwRMask16; dwMask16 && !(dwMask16&1); dwMask16>>=1 ) dwRShiftR16++;
    for( ; dwMask16; dwMask16>>=1 ) dwRShiftL16--;
    for( dwMask16=dwGMask16; dwMask16 && !(dwMask16&1); dwMask16>>=1 ) dwGShiftR16++;
    for( ; dwMask16; dwMask16>>=1 ) dwGShiftL16--;
    for( dwMask16=dwBMask16; dwMask16 && !(dwMask16&1); dwMask16>>=1 ) dwBShiftR16++;
    for( ; dwMask16; dwMask16>>=1 ) dwBShiftL16--;
    for( dwMask16=dwAMask16; dwMask16 && !(dwMask16&1); dwMask16>>=1 ) dwAShiftR16++;
    for( ; dwMask16; dwMask16>>=1 ) dwAShiftL16--;

	//32 bit masks
	for( dwMask32=dwRMask32; dwMask32 && !(dwMask32&1); dwMask32>>=1 ) dwRShiftR32++;
    for( ; dwMask32; dwMask32>>=1 ) dwRShiftL32--;
    for( dwMask32=dwGMask32; dwMask32 && !(dwMask32&1); dwMask32>>=1 ) dwGShiftR32++;
    for( ; dwMask32; dwMask32>>=1 ) dwGShiftL32--;
    for( dwMask32=dwBMask32; dwMask32 && !(dwMask32&1); dwMask32>>=1 ) dwBShiftR32++;
    for( ; dwMask32; dwMask32>>=1 ) dwBShiftL32--;
    for( dwMask32=dwAMask32; dwMask32 && !(dwMask32&1); dwMask32>>=1 ) dwAShiftR32++;
    for( ; dwMask32; dwMask32>>=1 ) dwAShiftL32--;

	//Only 16 or 32 bit renderer
	if (ddsd.ddpfPixelFormat.dwRGBBitCount != 16 &&
		ddsd.ddpfPixelFormat.dwRGBBitCount != 32)
	{
		MessageBox(g_hWnd, "Only 16 or 32 bit colour modes are supported.", 
			PROGRAM_NAME, MB_OK | MB_ICONSTOP);
		return FALSE;
	}

	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	//Okay!
	return TRUE;
}

//-----------------------------------------------------------------------------
// system_graphics_update_window()
//-----------------------------------------------------------------------------
void system_graphics_update_window(void)
{
	HRESULT ddrval;

	//Update the render target
	if SUCCEEDED(IDirectDrawSurface_Lock(lpDDSOne, NULL, &ddsd, 
		DDLOCK_WRITEONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL))
	{
		int x, y;
		_u16* row;
		_u16 r,g,b;

		//16 BIT DISPLAY
		if (ddsd.ddpfPixelFormat.dwRGBBitCount == 16)
		{
			_u16 r16,g16,b16, *frame16;
			_u32 w = ddsd.lPitch >> 1;

			frame16 = (_u16*)ddsd.lpSurface + (BORDER_HEIGHT * w) + BORDER_WIDTH;
			row = cfb;

			for (y = 0; y < SCREEN_HEIGHT; y++)
			{
				for (x = 0; x < SCREEN_WIDTH; x++)
				{
					//X4B4G4R ==> [0xR0G0B0]

					r = ((row[x] & 0x00F) << 4);
					g = ((row[x] & 0x0F0));
					b = ((row[x] & 0xF00) >> 4);

					r16 = ((r>>(dwRShiftL16))<<dwRShiftR16)&dwRMask16;
					g16 = ((g>>(dwGShiftL16))<<dwGShiftR16)&dwGMask16;
					b16 = ((b>>(dwBShiftL16))<<dwBShiftR16)&dwBMask16;

					frame16[x] = (r16 | g16 | b16);
				}

				frame16 += w;
				row += SCREEN_WIDTH;
			}
		}
		else	//32 BIT DISPLAY
		{
			_u32 r32,g32,b32, *frame32;
			_u32 w = ddsd.lPitch >> 2;

			frame32 = (_u32*)ddsd.lpSurface + (BORDER_HEIGHT * w) + BORDER_WIDTH;
			row = cfb;

			for (y = 0; y < SCREEN_HEIGHT; y++)
			{
				frame32 = (_u32*)ddsd.lpSurface + ((y + BORDER_HEIGHT) * w);
				frame32 += BORDER_WIDTH;

				for (x = 0; x < SCREEN_WIDTH; x++)
				{
					//X4B4G4R ==> [0xR0G0B0]
			
					r = ((row[x] & 0x00F) << 4);
					g = ((row[x] & 0x0F0));
					b = ((row[x] & 0xF00) >> 4);

					r32 = ((r>>(dwRShiftL32))<<dwRShiftR32)&dwRMask32;
					g32 = ((g>>(dwGShiftL32))<<dwGShiftR32)&dwGMask32;
					b32 = ((b>>(dwBShiftL32))<<dwBShiftR32)&dwBMask32;

					frame32[x] = (r32 | g32 | b32);
				}

				frame32 += w;
				row += SCREEN_WIDTH;
			}
		}
	
		IDirectDrawSurface_Unlock(lpDDSOne, NULL);	
		
		//Update the primary surface
		ddrval = IDirectDrawSurface_Blt(lpDDSPrimary, &g_ScreenRect, 
			lpDDSOne, NULL, 0, NULL);

		if (ddrval == DDERR_SURFACELOST)
		{
			IDirectDrawSurface_Restore(lpDDSPrimary);
			IDirectDrawSurface_Restore(lpDDSOne);
		}
	}
	else
	{
		IDirectDrawSurface_Restore(lpDDSPrimary);
		IDirectDrawSurface_Restore(lpDDSOne);
	}
}

//-----------------------------------------------------------------------------
// system_graphics_update_fullscreen()
//-----------------------------------------------------------------------------
void system_graphics_update_fullscreen(void)
{
	HRESULT ddrval;

	//Update the render target
	if SUCCEEDED(IDirectDrawSurface_Lock(lpDDSOne, NULL, &ddsd, 
		DDLOCK_WRITEONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL))
	{
		int x, y;
		_u16 *row;
		_u16 r,g,b;

		_u16 r16,g16,b16, *frame16;
		_u32 w = ddsd.lPitch >> 1;

		frame16 = (_u16*)ddsd.lpSurface + (BORDER_HEIGHT * w) + BORDER_WIDTH;
		row = cfb;

		for (y = 0; y < SCREEN_HEIGHT; y++)
		{
			for (x = 0; x < SCREEN_WIDTH; x++)
			{
				//X4B4G4R ==> [0xR0G0B0]

				r = ((row[x] & 0x00F) << 4);
				g = ((row[x] & 0x0F0));
				b = ((row[x] & 0xF00) >> 4);

				r16 = ((r>>(dwRShiftL16))<<dwRShiftR16)&dwRMask16;
				g16 = ((g>>(dwGShiftL16))<<dwGShiftR16)&dwGMask16;
				b16 = ((b>>(dwBShiftL16))<<dwBShiftR16)&dwBMask16;

				frame16[x] = (r16 | g16 | b16);
			}

			frame16 += w;
			row += SCREEN_WIDTH;
		}
	
		IDirectDrawSurface_Unlock(lpDDSOne, NULL);	
	
		//Update the back buffer
		ddrval = IDirectDrawSurface_Blt(lpDDSBack, &fsRect, lpDDSOne, NULL, 0, NULL);

		if (ddrval == DDERR_SURFACELOST)
		{
			IDirectDrawSurface_Restore(lpDDSPrimary);
			IDirectDrawSurface_Restore(lpDDSBack);
			IDirectDrawSurface_Restore(lpDDSOne);
		}

		//Flip
		IDirectDrawSurface_Flip(lpDDSPrimary, 0, 0);
	}
	else
	{
		IDirectDrawSurface_Restore(lpDDSPrimary);
		IDirectDrawSurface_Restore(lpDDSBack);
		IDirectDrawSurface_Restore(lpDDSOne);
	}
}

//-----------------------------------------------------------------------------
// system_graphics_shutdown()
//-----------------------------------------------------------------------------
void system_graphics_shutdown(void)
{
    if (lpDD)
    {
		IDirectDraw_RestoreDisplayMode(lpDD);
		 
		if (lpDDSPrimary)	IDirectDraw_Release(lpDDSPrimary);
		if (lpDDSOne)		IDirectDraw_Release(lpDDSOne);

		lpDDSPrimary = NULL;
		lpDDSBack = NULL;
		lpDDSOne = NULL;
 
		IDirectDraw_Release(lpDD);
        lpDD = NULL;
	}
}
	
//=============================================================================
