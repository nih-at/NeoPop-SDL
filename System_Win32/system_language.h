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

	system_language.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

18 AUG 2002 - neopop_uk
=======================================
- Created this file to handle translations.

//---------------------------------------------------------------------------
*/

#ifndef __SYSTEM_LANGUAGE__
#define __SYSTEM_LANGUAGE__
//=============================================================================

void system_language_patch(void);
void system_language_patch_dialog(int dialog_id, HWND dialog);

//=============================================================================
#endif
