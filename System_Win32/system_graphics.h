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

	system_graphics.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

22 AUG 2002 - neopop_uk
=======================================
- Added definition of border width and height

27 AUG 2002 - neopop_uk
=======================================
- Added necessary prototypes for windowed / full-screen mode.
- Added GDI support for full-screen file-selectors.

//---------------------------------------------------------------------------
*/

#ifndef __SYSTEM_GRAPHICS__
#define __SYSTEM_GRAPHICS__
//=============================================================================

#define BORDER_WIDTH	4
#define BORDER_HEIGHT	4

BOOL system_graphics_init(void);
void system_graphics_shutdown(void);
void system_graphics_update_window(void);
void system_graphics_update_fullscreen(void);
BOOL system_graphics_ready(void);

void system_graphics_gdi_begin(void);
void system_graphics_gdi_end(void);

//=============================================================================
#endif
