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

	system_main.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

21 JUL 2002 - neopop_uk
=======================================
- This include file is for 'system_main.c' functions only

26 JUL 2002 - neopop_uk
=======================================
- Made the menu external for 'unload rom'

22 AUG 2002 - neopop_uk
=======================================
- Moved NEOPOP_WIN32_VERSION here for more importance.

27 AUG 2002 - neopop_uk
=======================================
- Added "BOOL windowed"
  
28 AUG 2002 - neopop_uk
=======================================
- Made the keyboard accelerator global for external message pumps.
- Added 'set paused' function so comms can pause things.

//---------------------------------------------------------------------------
*/

#ifndef __SYSTEM_MAIN__
#define __SYSTEM_MAIN__
//=============================================================================

#define NEOPOP_WIN32_VERSION	"Win32 v1.06b"

extern HWND g_hWnd;
extern HMENU g_menu;
extern HINSTANCE g_hInstance;
extern RECT g_ScreenRect;
extern HACCEL g_accelerator;	//Keyboard shortcuts

void system_changed_rom(void);
void system_set_paused(BOOL enable);

//=============================================================================
#endif
