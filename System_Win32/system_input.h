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

	system_input.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

21 JUL 2002 - neopop_uk
=======================================
- Moved the button settings to the 'system_config.h' file

11 AUG 2002 - neopop_uk
=======================================
- Added prototypes for stick_name and stick_count.

//---------------------------------------------------------------------------
*/

#ifndef __SYSTEM_INPUT__
#define __SYSTEM_INPUT__
//=============================================================================

BOOL system_input_init(void);
void system_input_update(void);
void system_input_shutdown(void);

int system_input_buttoncount(void);
char* system_input_buttonname(int i);

int system_input_stickcount(void);
char* system_input_stickname(int i);

//Dialog box for setting input
BOOL CALLBACK InputProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//=============================================================================
#endif
