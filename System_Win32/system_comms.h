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

	system_comms.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

28 AUG 2002 - neopop_uk
=======================================
- Created this file to handle inter-NeoPop link-up communications.

//---------------------------------------------------------------------------
*/

#ifndef __SYSTEM_COMMS__
#define __SYSTEM_COMMS__
//=============================================================================

void system_comms_connect_dialog(void);
void system_comms_shutdown(void);

void system_comms_pause(BOOL remote_enable);

void system_comms_pause_check(void);

//=============================================================================
#endif
