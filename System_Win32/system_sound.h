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

	system_sound.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

08 AUG 2002 - neopop_uk
=======================================
- Replaced sound start and stop with 'system_sound_clear'. Prototype
	is in neopop.h because it's used by the core.

//---------------------------------------------------------------------------
*/

#ifndef __SYSTEM_SOUND__
#define __SYSTEM_SOUND__
//=============================================================================

BOOL system_sound_init(void);
void system_sound_update(void);
void system_sound_shutdown(void);

//=============================================================================
#endif
