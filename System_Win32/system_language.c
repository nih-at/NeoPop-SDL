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

	system_language.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

18 AUG 2002 - neopop_uk
=======================================
- Created this file to handle translations.

22 AUG 2002 - neopop_uk
=======================================
- Changed formatting of strings to be: "TOKEN=String to Translate"
- Fixed dialog box window captions

27 AUG 2002 - neopop_uk
=======================================
- Added 'Full Screen' menu option.

28 AUG 2002 - neopop_uk
=======================================
- Added support for the 'Connect' dialog box window caption.

30 AUG 2002 - neopop_uk
=======================================
- Removed frameskipping.

06 SEP 2002 - neopop_uk
=======================================
- Fixed translation of file selector filters.

08 SEP 2002 - neopop_uk
=======================================
- Added some more for the link-up dialog.

10 SEP 2002 - neopop_uk
=======================================
- Re-added frameskipping.

//---------------------------------------------------------------------------
*/

//=============================================================================

#include "neopop.h"
#include <windows.h>
#include <windowsx.h>
#include "resource.h"

#include "system_language.h"
#include "system_main.h"

//=============================================================================

typedef struct
{
	char label[9];
	int resource_id;
}
LANGUAGE_TAG;

typedef struct
{
	char label[9];
	char string[256];
}
STRING_TAG;

//=============================================================================

static LANGUAGE_TAG	menu_tags[] =
{
	//Code 				Resource Id

	//=======================

	// "Game" Menu
	"GAME",				-1,
	"LOAD",				ID_GAME_LOADROM,
	"RECENT",			-11,
	"MRUCLR",			ID_GAME_RESETMRU,
	"UNLOAD",			ID_GAME_UNLOADROM,
	"RESET",			ID_GAME_RESET,
	"PAUSE",			ID_GAME_PAUSE,
	"SSTATE",			ID_GAME_SAVESTATE,
	"LSTATE",			ID_GAME_LOADSTATE,
	"EXIT",				ID_GAME_EXIT,

	// "Options" Menu
	"OPTIONS",			-2,
	"SYSTYPE",			-21,
	"NGP",				ID_OPTIONS_DISPLAYTYPE_GREYSCALE,
	"NGPC",				ID_OPTIONS_DISPLAYTYPE_COLOUR,
	"NGAUTO",			ID_OPTIONS_DISPLAYTYPE_AUTO,
	"SYSLANG",			-22,
	"ENGSYS",			ID_OPTIONS_LANGUAGE_ENGLISH,
	"JAPSYS",			ID_OPTIONS_LANGUAGE_JAPANESE,
	"CTRL",				ID_OPTIONS_CONTROLS,
	"PATHS",			ID_OPTIONS_PATHS,
	"MMISC",			ID_OPTIONS_MISC,
	"FULLSCR",			ID_OPTIONS_FULLSCREEN,
	"WINSIZE",			-23,
	"ZOOM1",			ID_OPTIONS_ZOOMX1,
	"ZOOM2",			ID_OPTIONS_ZOOMX2,
	"ZOOM3",			ID_OPTIONS_ZOOMX3,
	"ZOOM4",			ID_OPTIONS_ZOOMX4,
	"FSKIP",			-24,
	"FS0",				ID_FRAMESKIP_FS0,				
	"FS1",				ID_FRAMESKIP_FS1,
	"FS2",				ID_FRAMESKIP_FS2,
	"FS3",				ID_FRAMESKIP_FS3,
	"FS4",				ID_FRAMESKIP_FS4,
	"FS5",				ID_FRAMESKIP_FS5,
	"FS6",				ID_FRAMESKIP_FS6,
	"FSLESS",			ID_FRAMESKIP_LESS,
	"FSMORE",			ID_FRAMESKIP_MORE,
	"MUTE",				ID_OPTIONS_MUTE,
	"MCONNECT",			ID_OPTIONS_CONNECT,

	// "Help" Menu
	"HELP",				-3,
	"LANGUAGE",			ID_HELP_LANGUAGE,		
	"ABOUT",			ID_HELP_ABOUTNEOPOP,

	//======================= END OF THE LIST
	"",0,
};

static LANGUAGE_TAG	dialog_tags[] =
{
	//Code 				Resource Id

	//=======================

	"OK",				IDOK,
	"CANCEL",			IDCANCEL,
	"DEFAULT",			IDDEFAULT,

	//=======================

	// "Controls" Dialog

	"DCONTROL",			IDD_CONTROLS,
	"JOYSEL",			IDC_SELECT_JOYSTICK,
	"DPAD",				IDC_DPAD,
	"BUTTON",			IDC_BUTTONLABEL1,
	"BUTTON",			IDC_BUTTONLABEL2,
	"BUTTON",			IDC_BUTTONLABEL3,
	"OPTION",			IDC_OPTION,
	"BUTTONA",			IDC_BUTTONA,
	"BUTTONB",			IDC_BUTTONB,
	"AUTOA",			IDC_CHECKA,
	"AUTOB",			IDC_CHECKB,
	"KEY",				IDC_KEYLABEL1,
	"KEY",				IDC_KEYLABEL2,
	"KEY",				IDC_KEYLABEL3,
	"ADAPTOID",			IDC_ADAPTOID,

	//=======================

	// "Paths" Dialog

	"DPATHS",			IDD_PATHS,

	"ROMPATH",			IDC_ROMPATH,
	"STAPATH",			IDC_STATEPATH,
	"FLAPATH",			IDC_FLASHPATH,
	"BROWSE",			IDC_BROWSE_ROM,
	"BROWSE",			IDC_BROWSE_STATE,
	"BROWSE",			IDC_BROWSE_FLASH,
	"LAST",				IDC_LAST_ROMS,
	"LAST",				IDC_LAST_STATE,

	//=======================

	// "About" Dialog

	"DABOUT",			IDD_ABOUT,

	//=======================

	// "Connect" Dialog

	"DCONNECT",			IDD_CONNECT,
	"BCONNECT",			IDC_CONNECT_BUTTON,
	"BLISTEN",			IDC_LISTEN_BUTTON,
	"REMOTE",			IDC_REMOTE_ADDRESS,
	"LOCAL",			IDC_LOCAL_ADDRESS,
	"PORT",				IDC_PORT,

	//=======================

	// "Misc" Dialog

	"DMISC",			IDD_MISC,
	"AUTOP",			IDC_CHECK_PAUSE,
	"ONTOP",			IDC_CHECK_ONTOP,
	"STRETCHX",			IDC_CHECK_STRETCHX,

	//======================= END OF THE LIST
	"",0

};

static STRING_TAG string_tags[STRINGS_MAX] =
{
	//NOTE: This ordering must match the enumeration 'STRINGS' in neopop.h

	"SDEFAULT",	"Are you sure you want to revert to the default control setup?",
	"ROMFILT",	"Rom Files (*.ngp,*.ngc,*.npc,*.zip)\0*.ngp;*.ngc;*.npc;*.zip\0\0", 
	"STAFILT",	"State Files (*.ngs)\0*.ngs\0\0",
	"FLAFILT",	"Flash Memory Files (*.ngf)\0*.ngf\0\0",
	"BADFLASH",	"The flash data for this rom is from a different version of NeoPop, it will be destroyed soon.",
	"POWER",	"The system has been signalled to power down. You must reset or load a new rom.",
	"BADSTATE",	"State is from an unsupported version of NeoPop.",
	"ERROR1",	"An error has occured creating the application window",
	"ERROR2",	"An error has occured initialising DirectDraw",
	"ERROR3",	"An error has occured initialising DirectInput",
	"TIMER",	"This system does not have a high resolution timer.",
	"WRONGROM",	"This state is from a different rom, Ignoring.",
	"EROMFIND",	"Cannot find ROM file",
	"EROMOPEN",	"Cannot open ROM file",
	"EZIPNONE", "No roms found",
	"EZIPBAD",	"Corrupted ZIP file",
	"EZIPFIND", "Cannot find ZIP file",

	"ABORT",	"Abort",
	"DISCON",	"Disconnect",
	"CONNEC",	"Connected.",
};

//=============================================================================

static void patch_menu(int id, char* new_text)
{
	char text[256];
	HMENU popup;

	//Special Cases
	switch(id)
	{
	case -1: ModifyMenu(g_menu, 0, MF_BYPOSITION|MF_STRING, 0, new_text);	return;
	case -2: ModifyMenu(g_menu, 1, MF_BYPOSITION|MF_STRING, 1, new_text);	return;
	case -3: ModifyMenu(g_menu, 2, MF_BYPOSITION|MF_STRING, 2, new_text);	return;

				//Recent
	case -11:	popup = GetSubMenu(g_menu, 0);
				ModifyMenu(popup, 1, MF_BYPOSITION|MF_STRING, 1, new_text);
				return;

				//System Type
	case -21:	popup = GetSubMenu(g_menu, 1);
				ModifyMenu(popup, 0, MF_BYPOSITION|MF_STRING, 0, new_text);
				return;

				//System Language
	case -22:	popup = GetSubMenu(g_menu, 1);
				ModifyMenu(popup, 1, MF_BYPOSITION|MF_STRING, 1, new_text);
				return;

				//Window Size
	case -23:	popup = GetSubMenu(g_menu, 1);
				ModifyMenu(popup, 8, MF_BYPOSITION|MF_STRING, 8, new_text);
				return;

				//Frameskip
	case -24:	popup = GetSubMenu(g_menu, 1);
				ModifyMenu(popup, 9, MF_BYPOSITION|MF_STRING, 9, new_text);
				return;
	}

	strcpy(text, new_text);

	//Append a keyboard shortcut?
	switch(id)
	{
	case ID_GAME_LOADROM:			strcat(text, "\tF1");			break;
	case ID_GAME_RESET:				strcat(text, "\tF3");			break;
	case ID_GAME_PAUSE:				strcat(text, "\tPause");		break;
	case ID_GAME_SAVESTATE:			strcat(text, "\tF6");			break;
	case ID_GAME_LOADSTATE:			strcat(text, "\tF9");			break;
	case ID_OPTIONS_MUTE:			strcat(text, "\tF8");			break;
	case ID_OPTIONS_FULLSCREEN:		strcat(text, "\tEsc");			break;
	case ID_FRAMESKIP_LESS:			strcat(text, "\tF11");			break;
	case ID_FRAMESKIP_MORE:			strcat(text, "\tF12");			break;
	}

	ModifyMenu(g_menu, id, MF_BYCOMMAND|MF_STRING, id, text);
}

static void patch_dialog(UINT dialog_id, HWND dialog, UINT id, char* text)
{
	switch(dialog_id)
	{
	case IDD_CONTROLS:

		switch(id)
		{
		case IDD_CONTROLS:	SetWindowText(dialog, text); return;
		case IDOK:		Button_SetText(GetDlgItem(dialog, id), text);	return;
		case IDC_CHECKA:	Button_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_CHECKB:	Button_SetText(GetDlgItem(dialog, id), text); return;
		case IDDEFAULT:		Button_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_ADAPTOID:	Button_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_BUTTONLABEL1: Static_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_BUTTONLABEL2: Static_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_BUTTONLABEL3: Static_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_KEYLABEL1: Static_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_KEYLABEL2: Static_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_KEYLABEL3: Static_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_SELECT_JOYSTICK: Static_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_DPAD: Static_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_OPTION: Static_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_BUTTONB: Static_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_BUTTONA: Static_SetText(GetDlgItem(dialog, id), text); return;
		}

		break;

	case IDD_PATHS:

		switch(id)
		{
		case IDD_PATHS: SetWindowText(dialog, text); return;
		case IDOK:		Button_SetText(GetDlgItem(dialog, id), text);	return;
		case IDCANCEL:	Button_SetText(GetDlgItem(dialog, id), text);	return;
		case IDC_ROMPATH: Static_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_STATEPATH: Static_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_FLASHPATH: Static_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_BROWSE_ROM: Button_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_BROWSE_STATE: Button_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_BROWSE_FLASH: Button_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_LAST_ROMS: Button_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_LAST_STATE: Button_SetText(GetDlgItem(dialog, id), text); return;
		}

		break;

	case IDD_CONNECT:

		switch(id)
		{
		case IDD_CONNECT: SetWindowText(dialog, text); return;
		case IDOK:		Button_SetText(GetDlgItem(dialog, id), text);	return;
		case IDC_CONNECT_BUTTON: Button_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_LISTEN_BUTTON: Button_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_REMOTE_ADDRESS: Static_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_LOCAL_ADDRESS: Static_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_PORT: Static_SetText(GetDlgItem(dialog, id), text); return;
		}

		break;

	case IDD_ABOUT:		

		switch(id)
		{
		case IDD_ABOUT:	SetWindowText(dialog, text); return;
		case IDOK:		Button_SetText(GetDlgItem(dialog, id), text);	return;
		}

		break;

	case IDD_MISC:		

		switch(id)
		{
		case IDD_MISC: SetWindowText(dialog, text); return;
		case IDC_CHECK_PAUSE: Button_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_CHECK_ONTOP: Button_SetText(GetDlgItem(dialog, id), text); return;
		case IDC_CHECK_STRETCHX: Button_SetText(GetDlgItem(dialog, id), text); return;
		case IDOK:		Button_SetText(GetDlgItem(dialog, id), text);	return;
		}

		break;
	}
}

static void patch_string(int id, char* text)
{
	//Special Cases
	switch(id)
	{
	case IDS_ROMFILTER:
		memset(string_tags[id].string, 0, sizeof(string_tags[id].string));
		strcpy(string_tags[id].string, text);
		strcat(string_tags[id].string, " (*.ngp,*.ngc,*.npc,*.zip)");
		strcpy(string_tags[id].string + strlen(string_tags[id].string) + 1,
			"*.ngp;*.ngc;*.npc;*.zip");
		return;

	case IDS_STATEFILTER:	
		memset(string_tags[id].string, 0, sizeof(string_tags[id].string));
		strcpy(string_tags[id].string, text);
		strcat(string_tags[id].string, " (*.ngs)");
		strcpy(string_tags[id].string + strlen(string_tags[id].string) + 1,
			"*.ngs");
		return;

	case IDS_FLASHFILTER:	
		memset(string_tags[id].string, 0, sizeof(string_tags[id].string));
		strcpy(string_tags[id].string, text);
		strcat(string_tags[id].string, " (*.ngf)");
		strcpy(string_tags[id].string + strlen(string_tags[id].string) + 1,
			"*.ngf");
		return;
	}

	//Default
	strcpy(string_tags[id].string, text);
}

//=============================================================================

//-----------------------------------------------------------------------------
// system_language_patch()
//-----------------------------------------------------------------------------
void system_language_patch(void)
{
	char line[1024], *ret, *tok;
	int i;

	FILE* fp = fopen("language.txt", "r");
	if (fp == NULL)
		return;	//No pack found
	
	//Read lines
	do
	{
		ret = fgets(line, 1024, fp);

		//Error, probably the end of file
		if (ret == NULL)
			break;

		//Parse the line

		//Get token, if NULL then no tokens here, maybe the next line?
		tok = strtok(line, "=\t ");
		if (tok == NULL) continue;
		
		//Get token, if NULL then no tokens here, maybe the next line?
		ret = strtok(NULL, "\n\t");
		if (ret == NULL) continue;

		// Decode the menu token
		i = 0;
		while(1)
		{
			//End of the list
			if (menu_tags[i].resource_id == 0)
				break;

			//Got one!
			if (_stricmp(tok, menu_tags[i].label) == 0)
				patch_menu(menu_tags[i].resource_id, ret);

			i++;
		}

		// Decode the string token
		i = 0;
		while(1)
		{
			//End of the list
			if (i == STRINGS_MAX)
				break;

			//Got one!
			if (_stricmp(tok, string_tags[i].label) == 0)
				patch_string(i, ret);

			i++;
		}
	}
	while(feof(fp) == 0);

	fclose(fp);

	//Grey the Help/Language menu item
	EnableMenuItem(g_menu, ID_HELP_LANGUAGE, MF_GRAYED);
}

//-----------------------------------------------------------------------------
// system_language_patch_dialog()
//-----------------------------------------------------------------------------
void system_language_patch_dialog(int dialog_id, HWND dialog)
{
	char line[1024], *ret, *tok;
	int i;

	FILE* fp = fopen("language.txt", "r");
	if (fp == NULL)
		return;	//No pack found

	//Read lines
	do
	{
		ret = fgets(line, 1024, fp);

		//Error, probably the end of file
		if (ret == NULL)
			break;

		//Parse the line

		//Get token, if NULL then no tokens here, maybe the next line?
		tok = strtok(line, "=\t ");
		if (tok == NULL) continue;
		
		//Get token, if NULL then no tokens here, maybe the next line?
		ret = strtok(NULL, "\n\t");
		if (ret == NULL) continue;

		// Decode the token
		i = 0;
		while(1)
		{
			//End of the list
			if (dialog_tags[i].resource_id == 0)
				break;

			//Got one!
			if (_stricmp(tok, dialog_tags[i].label) == 0)
				patch_dialog(dialog_id, dialog, dialog_tags[i].resource_id, ret);

			i++;
		}
	}
	while(feof(fp) == 0);

	fclose(fp);
}

//-----------------------------------------------------------------------------
// system_get_string()
//-----------------------------------------------------------------------------
char* system_get_string(STRINGS string_id)
{
	if (string_id >= STRINGS_MAX)
		return "Unknown String";

	return string_tags[string_id].string;
}

//=============================================================================

