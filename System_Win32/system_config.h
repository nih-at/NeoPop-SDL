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

	system_config.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

21 JUL 2002 - neopop_uk
=======================================
- Created this file to abstract the storage and retrieval of
configuration settings, and to provide a common location for all of the
Windows specific settings.

25 JUL 2002 - neopop_uk
=======================================
- Added saving of the flash memory path.
  
11 AUG 2002 - neopop_uk
=======================================
- Added saving of stick number
  
29 AUG 2002 - neopop_uk
=======================================
- Added saving of last IP address and port.
  
09 SEP 2002 - neopop_uk
=======================================
- Added config defintions for misc options
- Added adaptoid support.

//---------------------------------------------------------------------------
*/

#ifndef __SYSTEM_CONFIG__
#define __SYSTEM_CONFIG__
//=============================================================================

//Link-up configuration.
extern char comms_remoteaddress[64];
extern _u16 comms_port;

//Window positions
extern int mainview_x, mainview_y;
extern int debugview_x, debugview_y;
extern int codeview_x, codeview_y;
extern int memview_x, memview_y;
extern int regview_x, regview_y;
extern int z80regview_x, z80regview_y;
extern int breakview_x, breakview_y;
extern int watchview_x, watchview_y;

extern int zoom;

extern char RomDirectory[256];
extern char StateDirectory[256];
extern BOOL LastRomDirectory;
extern BOOL LastStateDirectory;
extern char FlashDirectory[256];

//Recent Files
extern char MRUFiles[8][256];

//Controls
extern int BUTTONA;
extern int BUTTONB;
extern int BUTTONO;

extern int KEYA;
extern int KEYB;
extern int KEYO;
extern int KEYU;
extern int KEYD;
extern int KEYL;
extern int KEYR;

extern BOOL AUTOA;	//Autofire A
extern BOOL AUTOB;	//Autofire B

extern int STICK;
extern BOOL adaptoid;

extern BOOL misc_autopause;
extern BOOL misc_ontop;
extern BOOL misc_stetchx;

extern BOOL windowed;

//=============================================================================

void system_put_config(void);
void system_get_config(void);

//=============================================================================
#endif
