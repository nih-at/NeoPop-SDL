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

	system_config.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

21 JUL 2002 - neopop_uk
=======================================
- Created this file to abstract the storage and retrieval of
configuration settings

24 JUL 2002 - neopop_uk
=======================================
- Fixed the default sound frequency when no registry setting is present.

31 JUL 2002 - neopop_uk
=======================================
- Added an error message if the put function fails, 
	not needed for get - may be a newbie.

01 AUG 2002 - neopop_uk
=======================================
- Added support for frameskipping.

09 AUG 2002 - neopop_uk
=======================================
- Fixed the three path strings, the buffer length was not being reset.
- Add MRU file saving and loading.
  
11 AUG 2002 - neopop_uk
=======================================
- Added saving of stick number
  
17 AUG 2002 - neopop_uk
=======================================
- Moved common default settings to 'system_set_defaults'
 
30 AUG 2002 - neopop_uk
=======================================
- Removed frameskipping.
  
05 SEP 2002 - neopop_uk
=======================================
- Save/Load breakpoints

09 SEP 2002 - neopop_uk
=======================================
- Added config declarations for misc options
- Added adaptoid support.

10 SEP 2002 - neopop_uk
=======================================
- Added 'misc_stetchx'

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include <windows.h>

//For default keys
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#ifdef NEOPOP_DEBUG
#include "Debugger/system_debug.h"
#endif

//=============================================================================

//Link-up configuration.
char comms_remoteaddress[64];
_u16 comms_port;

//Window positions
int mainview_x, mainview_y;
int debugview_x, debugview_y;
int codeview_x, codeview_y;
int memview_x, memview_y;
int regview_x, regview_y;
int z80regview_x, z80regview_y;
int breakview_x, breakview_y;
int watchview_x, watchview_y;

char RomDirectory[256];
char StateDirectory[256];
char FlashDirectory[256];
BOOL LastRomDirectory;
BOOL LastStateDirectory;

//Recent Files
char MRUFiles[8][256];

//Controls
int BUTTONA;
int BUTTONB;
int BUTTONO;

int KEYA;
int KEYB;
int KEYO;
int KEYU;
int KEYD;
int KEYL;
int KEYR;

BOOL AUTOA;	//Autofire A
BOOL AUTOB;	//Autofire B

int zoom;

int STICK = 0;
BOOL adaptoid;

BOOL misc_autopause;
BOOL misc_ontop;
BOOL misc_stetchx;

BOOL windowed;

_u8 system_frameskip_key;

//=============================================================================

//-----------------------------------------------------------------------------
// system_set_defaults()
//-----------------------------------------------------------------------------
static void system_set_defaults(void)
{
	int i;

	strcpy(comms_remoteaddress, "127.0.0.1");
	comms_port = 7846;

	mainview_x = (int)(GetSystemMetrics(SM_CXSCREEN) * 0.25f);
	mainview_y = (int)(GetSystemMetrics(SM_CYSCREEN) * 0.25f);

	sprintf(RomDirectory, ".\\Roms\\");
	sprintf(StateDirectory, ".\\States\\");
	sprintf(FlashDirectory, ".\\Battery\\");

	LastRomDirectory = TRUE;
	LastStateDirectory = TRUE;

	for (i = 0; i < 8; i++)
		sprintf(MRUFiles[i], "\0");

	BUTTONA = 0;
	BUTTONB = 1;
	BUTTONO = 2;

	KEYA = DIK_Z;
	KEYB = DIK_X;
	KEYO = DIK_TAB;
	KEYU = DIK_UP;
	KEYD = DIK_DOWN;
	KEYL = DIK_LEFT;
	KEYR = DIK_RIGHT;

	AUTOA = FALSE;
	AUTOB = FALSE;

	zoom = 2;

	STICK = 0;
	adaptoid = FALSE;

	misc_autopause = TRUE;
	misc_ontop = FALSE;
	misc_stetchx = FALSE;

	windowed = TRUE;

	system_frameskip_key = 8;		// 1 - 7

	//Core settings
	mute = FALSE;
	language_english = TRUE;
	system_colour = COLOURMODE_AUTO;

	//Debugger Settings
#ifdef NEOPOP_DEBUG

	debugview_x = 10;		debugview_y = 10;
	codeview_x = 20;		codeview_y = 20;
	regview_x = 30;			regview_y = 30;	
	z80regview_x = 50;		z80regview_y = 50;
	memview_x = 40;			memview_y = 40;
	breakview_x = 60;		breakview_y = 60;
	watchview_x = 55;		watchview_y = 55;

	//Clear breakpoints
	for (i = 0; i < MAX_BREAKPOINTS; i++)
		breakpoint[i].enabled = FALSE;
	breakpoint_z80.enabled = FALSE;

	filter_sound = FALSE;
	filter_dma = FALSE;
	filter_bios = FALSE;
	filter_comms = FALSE;
	filter_mem = FALSE;

#endif
}
	
//-----------------------------------------------------------------------------
// system_get_config()
//-----------------------------------------------------------------------------
void system_get_config(void)
{
	HKEY handle = NULL;
	DWORD value;
	DWORD size;
	char path[256];

	//Apply defaults
	system_set_defaults();

	//Try and open
	RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\NeoPop", 0, KEY_QUERY_VALUE, &handle);

	//================================= DWORDS

	//***** comms_port
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "comms_port", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		comms_port = (_u16)(min(max(value, 1), 65535));

	//***** comms_remoteaddress
	size = 64;
	if (RegQueryValueEx(handle, "comms_remoteaddress", NULL, NULL, (BYTE*)(path), &size) == ERROR_SUCCESS)
		sprintf(comms_remoteaddress, "%s", path);

	//***** Zoom
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "zoom", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		zoom = min(max(value, 1), 4);

	//***** Mute
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "mute", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		mute = max(min((BOOL)value, TRUE), FALSE);

	//***** adaptoid
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "adaptoid", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		adaptoid = max(min((BOOL)value, TRUE), FALSE);

	//***** Windowed
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "windowed", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		windowed = max(min((BOOL)value, TRUE), FALSE);

	//***** misc_autopause
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "misc_autopause", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		misc_autopause = max(min((BOOL)value, TRUE), FALSE);

	//***** misc_ontop
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "misc_ontop", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		misc_ontop = max(min((BOOL)value, TRUE), FALSE);

	//***** misc_stetchx
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "misc_stetchx", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		misc_stetchx = max(min((BOOL)value, TRUE), FALSE);

	//***** Language
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "language", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		language_english = max(min((BOOL)value, TRUE), FALSE);

	//***** Colour
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "colour", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		system_colour = (COLOURMODE)(max(min(value, COLOURMODE_AUTO), COLOURMODE_GREYSCALE)); 

	//***** Joystick
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "stick", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		STICK = min(max(value, 0), 31);

	//***** system_frameskip_key
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "system_frameskip_key", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		system_frameskip_key = (_u8)(min(max(value, 1), 7));

	//***** Joystick Buttons
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "button_a", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		BUTTONA = min(max(value, 0), 31);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "button_b", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		BUTTONB = min(max(value, 0), 31);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "button_o", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		BUTTONO = min(max(value, 0), 31);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "auto_a", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		AUTOA = max(min((BOOL)value, TRUE), FALSE);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "auto_b", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		AUTOB = max(min((BOOL)value, TRUE), FALSE);

	//***** Keyboard Buttons
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "key_a", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		KEYA = min(max(value, 0), 0xFF);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "key_b", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		KEYB = min(max(value, 0), 0xFF);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "key_o", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		KEYO = min(max(value, 0), 0xFF);

	//***** Keyboard Directions
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "key_u", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		KEYU = min(max(value, 0), 0xFF);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "key_d", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		KEYD = min(max(value, 0), 0xFF);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "key_l", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		KEYL = min(max(value, 0), 0xFF);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "key_r", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		KEYR = min(max(value, 0), 0xFF);

	//***** Main Window Positions
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "mainview_x", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		mainview_x = min(max((int)value, 0), GetSystemMetrics(SM_CXSCREEN) - 20); 
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "mainview_y", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		mainview_y = min(max((int)value, 0), GetSystemMetrics(SM_CYSCREEN) - 10); 

#ifdef NEOPOP_DEBUG
	//***** Debugger Window Positions
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "debugview_x", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		debugview_x = min(max((int)value, 0), GetSystemMetrics(SM_CXSCREEN) - 40);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "debugview_y", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		debugview_y = min(max((int)value, 0), GetSystemMetrics(SM_CYSCREEN) - 10);

	//***** Code View Window Positions
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "codeview_x", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		codeview_x = min(max((int)value, 0), GetSystemMetrics(SM_CXSCREEN) - 40);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "codeview_y", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		codeview_y = min(max((int)value, 0), GetSystemMetrics(SM_CYSCREEN) - 10);

	//***** Reg View Window Positions
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "regview_x", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		regview_x = min(max((int)value, 0), GetSystemMetrics(SM_CXSCREEN) - 40);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "regview_y", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		regview_y = min(max((int)value, 0), GetSystemMetrics(SM_CYSCREEN) - 10);

	//***** Z80 Reg View Window Positions
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "z80regview_x", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		z80regview_x = min(max((int)value, 0), GetSystemMetrics(SM_CXSCREEN) - 40);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "z80regview_y", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		z80regview_y = min(max((int)value, 0), GetSystemMetrics(SM_CYSCREEN) - 10);

	//***** Mem View Window Positions
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "memview_x", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		memview_x = min(max((int)value, 0), GetSystemMetrics(SM_CXSCREEN) - 40);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "memview_y", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		memview_y = min(max((int)value, 0), GetSystemMetrics(SM_CYSCREEN) - 10);

	//***** Breakpoint View Window Positions
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "breakview_x", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		breakview_x = min(max((int)value, 0), GetSystemMetrics(SM_CXSCREEN) - 40);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "breakview_y", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		breakview_y = min(max((int)value, 0), GetSystemMetrics(SM_CYSCREEN) - 10);

	//***** Breakpoint View Window Positions
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "watchview_x", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		watchview_x = min(max((int)value, 0), GetSystemMetrics(SM_CXSCREEN) - 40);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "watchview_y", 0, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		watchview_y = min(max((int)value, 0), GetSystemMetrics(SM_CYSCREEN) - 10);

	//***** Breakpoint enabled
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "breakpoint1_enabled", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		breakpoint[1].enabled = min(max(value, 0), 1);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "breakpoint2_enabled", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		breakpoint[2].enabled = min(max(value, 0), 1);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "breakpoint3_enabled", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		breakpoint[3].enabled = min(max(value, 0), 1);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "breakpoint_z80_enabled", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		breakpoint_z80.enabled = min(max(value, 0), 1);
	
	//***** Breakpoint addresses
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "breakpoint1_address", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		breakpoint[1].address = value & 0xFFFFFF;
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "breakpoint2_address", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		breakpoint[2].address = value & 0xFFFFFF;
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "breakpoint3_address", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		breakpoint[3].address = value & 0xFFFFFF;
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "breakpoint_z80_address", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		breakpoint_z80.address = value & 0xFFF;
	
	//***** Debug Filters
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "filter_sound", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		filter_sound = min(max(value, 0), 1);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "filter_dma", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		filter_dma = min(max(value, 0), 1);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "filter_bios", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		filter_bios = min(max(value, 0), 1);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "filter_comms", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		filter_comms = min(max(value, 0), 1);
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "filter_mem", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		filter_mem = min(max(value, 0), 1);
#endif

	//***** LastRomDirectory
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "last_rom_dir", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		LastRomDirectory = max(min((BOOL)value, TRUE), FALSE);

	//***** LastStateDirectory
	size = sizeof(DWORD);
	if (RegQueryValueEx(handle, "last_state_dir", NULL, NULL, (BYTE*)(&value), &size) == ERROR_SUCCESS)
		LastStateDirectory = max(min((BOOL)value, TRUE), FALSE);

	//================================= STRINGS

	//***** RomDirectory
	size = 255;
	if (RegQueryValueEx(handle, "dir", NULL, NULL, (BYTE*)(path), &size) == ERROR_SUCCESS)
		sprintf(RomDirectory, "%s", path);

	//***** StateDirectory
	size = 255;
	if (RegQueryValueEx(handle, "state_dir", NULL, NULL, (BYTE*)(path), &size) == ERROR_SUCCESS)
		sprintf(StateDirectory, "%s", path);

	//***** FlashDirectory
	size = 255;
	if (RegQueryValueEx(handle, "flash_dir", NULL, NULL, (BYTE*)(path), &size) == ERROR_SUCCESS)
		sprintf(FlashDirectory, "%s", path);
	
	//***** MRU
	size = 255;
	if (RegQueryValueEx(handle, "mru0", NULL, NULL, (BYTE*)(path), &size) == ERROR_SUCCESS)
		sprintf(MRUFiles[0], "%s", path);
	size = 255;
	if (RegQueryValueEx(handle, "mru1", NULL, NULL, (BYTE*)(path), &size) == ERROR_SUCCESS)
		sprintf(MRUFiles[1], "%s", path);
	size = 255;
	if (RegQueryValueEx(handle, "mru2", NULL, NULL, (BYTE*)(path), &size) == ERROR_SUCCESS)
		sprintf(MRUFiles[2], "%s", path);
	size = 255;
	if (RegQueryValueEx(handle, "mru3", NULL, NULL, (BYTE*)(path), &size) == ERROR_SUCCESS)
		sprintf(MRUFiles[3], "%s", path);
	size = 255;
	if (RegQueryValueEx(handle, "mru4", NULL, NULL, (BYTE*)(path), &size) == ERROR_SUCCESS)
		sprintf(MRUFiles[4], "%s", path);
	size = 255;
	if (RegQueryValueEx(handle, "mru5", NULL, NULL, (BYTE*)(path), &size) == ERROR_SUCCESS)
		sprintf(MRUFiles[5], "%s", path);
	size = 255;
	if (RegQueryValueEx(handle, "mru6", NULL, NULL, (BYTE*)(path), &size) == ERROR_SUCCESS)
		sprintf(MRUFiles[6], "%s", path);
	size = 255;
	if (RegQueryValueEx(handle, "mru7", NULL, NULL, (BYTE*)(path), &size) == ERROR_SUCCESS)
		sprintf(MRUFiles[7], "%s", path);

	// =================================

	//Close the key
	if (handle)
		RegCloseKey(handle);
}

//=============================================================================

//-----------------------------------------------------------------------------
// system_put_config()
//-----------------------------------------------------------------------------
void system_put_config(void)
{
	HKEY handle;
	DWORD disposition;
	DWORD value;
	DWORD size;

	if (RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\NeoPop", 0,
		"Settings", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
		&handle, &disposition) != ERROR_SUCCESS)
		return;

	//***** comms_port
	value = comms_port;
	RegSetValueEx(handle, "comms_port", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** comms_remoteaddress
	size = strlen(comms_remoteaddress) + 1;
	RegSetValueEx(handle, "comms_remoteaddress", 0, REG_SZ, comms_remoteaddress, size);

	//***** Zoom
	value = zoom;
	RegSetValueEx(handle, "zoom", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Mute
	value = mute;
	RegSetValueEx(handle, "mute", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** adaptoid
	value = adaptoid;
	RegSetValueEx(handle, "adaptoid", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** misc_autopause
	value = misc_autopause;
	RegSetValueEx(handle, "misc_autopause", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** misc_ontop
	value = misc_ontop;
	RegSetValueEx(handle, "misc_ontop", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** misc_stetchx
	value = misc_stetchx;
	RegSetValueEx(handle, "misc_stetchx", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Windowed
	value = windowed;
	RegSetValueEx(handle, "windowed", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Language
	value = language_english;
	RegSetValueEx(handle, "language", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Language
	value = system_colour;
	RegSetValueEx(handle, "colour", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** RomDirectory
	size = strlen(RomDirectory) + 1;
	RegSetValueEx(handle, "dir", 0, REG_SZ, RomDirectory, size);

	//***** FlashDirectory
	size = strlen(FlashDirectory) + 1;
	RegSetValueEx(handle, "flash_dir", 0, REG_SZ, FlashDirectory, size);

	//***** StateDirectory
	size = strlen(StateDirectory) + 1;
	RegSetValueEx(handle, "state_dir", 0, REG_SZ, StateDirectory, size);

	//***** MRU
	size = strlen(MRUFiles[0]) + 1;
	RegSetValueEx(handle, "mru0", 0, REG_SZ, MRUFiles[0], size);
	size = strlen(MRUFiles[1]) + 1;
	RegSetValueEx(handle, "mru1", 0, REG_SZ, MRUFiles[1], size);
	size = strlen(MRUFiles[2]) + 1;
	RegSetValueEx(handle, "mru2", 0, REG_SZ, MRUFiles[2], size);
	size = strlen(MRUFiles[3]) + 1;
	RegSetValueEx(handle, "mru3", 0, REG_SZ, MRUFiles[3], size);
	size = strlen(MRUFiles[4]) + 1;
	RegSetValueEx(handle, "mru4", 0, REG_SZ, MRUFiles[4], size);
	size = strlen(MRUFiles[5]) + 1;
	RegSetValueEx(handle, "mru5", 0, REG_SZ, MRUFiles[5], size);
	size = strlen(MRUFiles[6]) + 1;
	RegSetValueEx(handle, "mru6", 0, REG_SZ, MRUFiles[6], size);
	size = strlen(MRUFiles[7]) + 1;
	RegSetValueEx(handle, "mru7", 0, REG_SZ, MRUFiles[7], size);

	//***** LastRomDirectory
	value = LastRomDirectory;
	RegSetValueEx(handle, "last_rom_dir", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** LastStateDirectory
	value = LastStateDirectory;
	RegSetValueEx(handle, "last_state_dir", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Joystick
	value = STICK;RegSetValueEx(handle, "stick", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** system_frameskip_key
	value = system_frameskip_key;
	RegSetValueEx(handle, "system_frameskip_key", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Joystick Buttons
	value = BUTTONA;RegSetValueEx(handle, "button_a", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = BUTTONB;RegSetValueEx(handle, "button_b", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = BUTTONO;RegSetValueEx(handle, "button_o", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = AUTOA;RegSetValueEx(handle, "auto_a", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = AUTOB;RegSetValueEx(handle, "auto_b", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Keyboard Buttons
	value = KEYA;RegSetValueEx(handle, "key_a", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = KEYB;RegSetValueEx(handle, "key_b", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = KEYO;RegSetValueEx(handle, "key_o", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Keyboard Directions
	value = KEYU;RegSetValueEx(handle, "key_u", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = KEYD;RegSetValueEx(handle, "key_d", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = KEYL;RegSetValueEx(handle, "key_l", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = KEYR;RegSetValueEx(handle, "key_r", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Main Window Positions
	value = mainview_x;RegSetValueEx(handle, "mainview_x", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = mainview_y;RegSetValueEx(handle, "mainview_y", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

#ifdef NEOPOP_DEBUG
	//***** Debugger Window Positions
	value = debugview_x;RegSetValueEx(handle, "debugview_x", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = debugview_y;RegSetValueEx(handle, "debugview_y", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Code View Window Positions
	value = codeview_x;RegSetValueEx(handle, "codeview_x", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = codeview_y;RegSetValueEx(handle, "codeview_y", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Reg View Window Positions
	value = regview_x;RegSetValueEx(handle, "regview_x", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = regview_y;RegSetValueEx(handle, "regview_y", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Z80 Reg View Window Positions
	value = z80regview_x;RegSetValueEx(handle, "z80regview_x", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = z80regview_y;RegSetValueEx(handle, "z80regview_y", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Mem View Window Positions
	value = memview_x;RegSetValueEx(handle, "memview_x", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = memview_y;RegSetValueEx(handle, "memview_y", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Breakpoint Window Positions
	value = breakview_x;RegSetValueEx(handle, "breakview_x", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = breakview_y;RegSetValueEx(handle, "breakview_y", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Breakpoint enabled
	value = breakpoint[1].enabled;RegSetValueEx(handle, "breakpoint1_enabled", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = breakpoint[2].enabled;RegSetValueEx(handle, "breakpoint2_enabled", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = breakpoint[3].enabled;RegSetValueEx(handle, "breakpoint3_enabled", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = breakpoint_z80.enabled;RegSetValueEx(handle, "breakpoint_z80_enabled", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	
	//***** Breakpoint addresses
	value = breakpoint[1].address;RegSetValueEx(handle, "breakpoint1_address", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = breakpoint[2].address;RegSetValueEx(handle, "breakpoint2_address", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = breakpoint[3].address;RegSetValueEx(handle, "breakpoint3_address", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = breakpoint_z80.address;RegSetValueEx(handle, "breakpoint_z80_address", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Watch Window Positions
	value = watchview_x;RegSetValueEx(handle, "watchview_x", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = watchview_y;RegSetValueEx(handle, "watchview_y", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));

	//***** Debug Filters
	value = filter_sound;RegSetValueEx(handle, "filter_sound", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = filter_dma;RegSetValueEx(handle, "filter_dma", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = filter_bios;RegSetValueEx(handle, "filter_bios", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = filter_comms;RegSetValueEx(handle, "filter_comms", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
	value = filter_mem;RegSetValueEx(handle, "filter_mem", 0, REG_DWORD, (BYTE*)(&value), sizeof(DWORD));
#endif
	
	//Close the key
	RegCloseKey(handle);
}

//=============================================================================
