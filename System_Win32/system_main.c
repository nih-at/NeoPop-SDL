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

	system_main.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

21 JUL 2002 - neopop_uk
=======================================
- Moved a lot of functions out of this file, including get/put config.
- Re-added the performance percentage, by popular demand!

22 JUL 2002 - neopop_uk
=======================================
- Added the 'auto detect' colour mode option. Colour changes are now
	instantaneous.

24 JUL 2002 - neopop_uk
=======================================
- System type resets are only caused when the new type is different.
- Loading a state causes the System type to be re-detected.

25 JUL 2002 - neopop_uk
=======================================
- Added paths dialog box.

26 JUL 2002 - neopop_uk
=======================================
- Added a simple 'browse' button to the flash path selector.
- Added rom and state paths to the paths dialog + browse buttons.

26 JUL 2002 - neopop_uk
=======================================
- Made the paths dialog appear on first running.
	- More user friendly.

27 JUL 2002 - neopop_uk
=======================================
- Allowed the use of relative paths for roms, states and flash files.
- Added 'use last directory' options for roms and states.

01 AUG 2002 - neopop_uk
=======================================
- Added support for frameskipping.
- Changed the way the about dialog is altered.

01 AUG 2002 - neopop_uk
=======================================
- Added keyboard accelerator
- Added option to inc. or dec. frameskipping, mainly for keyboard use.
- The Frameskip value is shown on the window caption.
- Allowed ALT keys to work - uses accelerator for ALT-F4

03 AUG 2002 - neopop_uk
=======================================
- Window caption changes when paused.

08 AUG 2002 - neopop_uk
=======================================
- Added performance timers, and made the caption only update every second.

08 AUG 2002 - neopop_uk
=======================================
- New function windows_load_rom which deals with the menus.

09 AUG 2002 - neopop_uk
=======================================
- Added recent files list.

10 AUG 2002 - neopop_uk
=======================================
- MRU access moves item up the list. This allows it to maintain an
	MRU list ordered by time.

11 AUG 2002 - neopop_uk
=======================================
- Added DestroyMenu call, which should prevent a leak...

16 AUG 2002 - neopop_uk
=======================================
- Improved MRU list, opening a file in the list, moves it up.

17 AUG 2002 - neopop_uk
=======================================
- Added definition for NEOPOP_WIN32_VERSION

18 AUG 2002 - neopop_uk
=======================================
- Removed duplicate call to 'bios_install' (was biosInstall)
- Added call to 'system_language_patch'

22 AUG 2002 - neopop_uk
=======================================
- Added a border around the display.

22 AUG 2002 - neopop_uk
=======================================
- Fixed dialog window captions.

27 AUG 2002 - neopop_uk
=======================================
- Added full-screen / windowed support.
- Added GDI compatibility so that file-selectors work in FS mode!
- Added right-click popup for recent files.

28 AUG 2002 - neopop_uk
=======================================
- Disabled auto-pause for multitasking link-up testing.

30 AUG 2002 - neopop_uk
=======================================
- Removed frameskipping.
- Changed '&' to '+' in MRU menus, better for "Rockman"

06 SEP 2002 - neopop_uk
=======================================
- Removed the hack way of translating file selector titles.

10 SEP 2002 - neopop_uk
=======================================
- Put frameskipping back in.
- Added 'misc_stetchx'

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include <direct.h>
#include <windows.h>
#include <windowsx.h>
#include "resource.h"
#include <math.h>			//ceil

#include "system_main.h"
#include "system_graphics.h"
#include "system_input.h"
#include "system_sound.h"
#include "system_rom.h"
#include "system_config.h"
#include "system_language.h"
#include "system_comms.h"

#ifdef NEOPOP_DEBUG
#include "Debugger/system_debug.h"
#endif

//=============================================================================

HWND g_hWnd;
HMENU g_menu;
HINSTANCE g_hInstance;
RECT g_ScreenRect;
HACCEL g_accelerator;	//Keyboard shortcuts

BOOL paused = FALSE;

//=============================================================================

static BOOL auto_pause = FALSE;

static LARGE_INTEGER throttleTick_L, throttleRate_L;
static _u64 throttleRate, throttleLast, throttleDiff;
static _u32 throttleRate100;

static int frameRateCount;

#define FRAMERATECOUNT	60	//Number of frames between updates.

//=============================================================================

//-----------------------------------------------------------------------------
// system_set_paused()
//-----------------------------------------------------------------------------
void system_set_paused(BOOL enable)
{
	paused = enable;
	system_sound_silence();
	if (paused)
	{
		SetWindowText(g_hWnd, PROGRAM_NAME" - Paused");
		CheckMenuItem(g_menu, ID_GAME_PAUSE, MF_CHECKED);
	}
	else
	{
		if (rom.data)
		{
			char title[128];
			sprintf(title, PROGRAM_NAME" - %s", rom.name);
			SetWindowText(g_hWnd, title);
		}
		CheckMenuItem(g_menu, ID_GAME_PAUSE, MF_UNCHECKED);
	}
}

//-----------------------------------------------------------------------------
// system_VBL()
//-----------------------------------------------------------------------------
void system_VBL(void)
{
	if (frameskip_count == 0)
	{
		// Update Graphics
		if (windowed)	system_graphics_update_window();
		else			system_graphics_update_fullscreen();
	}

	// Update Input
	system_input_update();

	// Update Sound
	if (mute == FALSE)
		system_sound_update();

	//Throttle speed
	do 
	{
		QueryPerformanceCounter(&throttleTick_L);
		throttleDiff = throttleTick_L.QuadPart - throttleLast;
	}
	while(throttleDiff < throttleRate);
	throttleLast = throttleTick_L.QuadPart;

	if (windowed)
	{
		//Show a ROUGH estimate of performance.
		frameRateCount++;
		if (frameRateCount >= FRAMERATECOUNT)
		{
			frameRateCount = 0;
			if (rom.data)
			{
				int p = (int)ceil((float)throttleRate100 / (_u32)throttleDiff);
				char title[128];
				sprintf(title, PROGRAM_NAME" - %s - %d%% / FS%d", 
					rom.name, p, system_frameskip_key - 1);
				SetWindowTextA(g_hWnd, title);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// system_changed_rom()
//-----------------------------------------------------------------------------
void system_changed_rom(void)
{
	char title[128];
	sprintf(title, PROGRAM_NAME" - %s", rom.name);
	SetWindowText(g_hWnd, title);
}

//=============================================================================

//-----------------------------------------------------------------------------
// setZoom()
//-----------------------------------------------------------------------------
static void setZoom(int z)
{
	UINT m;

	if (windowed)
	{
		zoom = z;	// Set new zoom level

		//Check the menu entries
		if (z == 1) m = MF_CHECKED; else m = MF_UNCHECKED;
		CheckMenuItem(g_menu, ID_OPTIONS_ZOOMX1, m);
		if (z == 2) m = MF_CHECKED; else m = MF_UNCHECKED;
		CheckMenuItem(g_menu, ID_OPTIONS_ZOOMX2, m);
		if (z == 3) m = MF_CHECKED; else m = MF_UNCHECKED;
		CheckMenuItem(g_menu, ID_OPTIONS_ZOOMX3, m);
		if (z == 4) m = MF_CHECKED; else m = MF_UNCHECKED;
		CheckMenuItem(g_menu, ID_OPTIONS_ZOOMX4, m);

		//Adjust the window
		SetWindowPos(g_hWnd, NULL, 0,0, 
			((SCREEN_WIDTH + (BORDER_WIDTH*2)) * zoom) + GetSystemMetrics(SM_CXFIXEDFRAME)*2, 
			((SCREEN_HEIGHT + (BORDER_HEIGHT*2)) * zoom) + GetSystemMetrics(SM_CYFIXEDFRAME)*2 + 
				GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU), 
			SWP_NOZORDER | SWP_NOMOVE);

		//Update the display rectangle
		GetClientRect(g_hWnd, &g_ScreenRect);
		ClientToScreen(g_hWnd, (POINT*)&g_ScreenRect.left);
		ClientToScreen(g_hWnd, (POINT*)&g_ScreenRect.right);
	}
}

//-----------------------------------------------------------------------------
// switchDisplayMode()
//-----------------------------------------------------------------------------
static void switchDisplayMode(void)
{
	system_graphics_shutdown();

	//Hide Window, this is critical otherwise SetWindowLong crashes!
	ShowWindow(g_hWnd, SW_HIDE);

	if (windowed)
	{
		//Store the window position
		if ((IsIconic(g_hWnd) == 0))
		{
			RECT r;
			GetWindowRect(g_hWnd, &r);
			mainview_x = r.left;
			mainview_y = r.top;
		}

		// ******* FULL-SCREEN MODE ********
		windowed = FALSE;
		SetWindowLong(g_hWnd, GWL_STYLE, WS_POPUP);
		SetWindowPos(g_hWnd, 0, 0,0, 640,480, SWP_NOZORDER);
		while (ShowCursor(FALSE) >= 0);
		SetMenu(g_hWnd, NULL);
	}
	else
	{
		// ******* WINDOWED MODE ********
		windowed = TRUE;
		SetWindowLong(g_hWnd, GWL_STYLE, WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
		while (ShowCursor(TRUE) < 0);
		
		if (misc_ontop)
		SetWindowPos(g_hWnd, HWND_TOPMOST, mainview_x,mainview_y, 
			((SCREEN_WIDTH + (BORDER_WIDTH*2)) * zoom) + GetSystemMetrics(SM_CXFIXEDFRAME)*2, 
			((SCREEN_HEIGHT + (BORDER_HEIGHT*2)) * zoom) + GetSystemMetrics(SM_CYFIXEDFRAME)*2 + 
				GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU), 0);
		else
		SetWindowPos(g_hWnd, HWND_NOTOPMOST, mainview_x,mainview_y, 
			((SCREEN_WIDTH + (BORDER_WIDTH*2)) * zoom) + GetSystemMetrics(SM_CXFIXEDFRAME)*2, 
			((SCREEN_HEIGHT + (BORDER_HEIGHT*2)) * zoom) + GetSystemMetrics(SM_CYFIXEDFRAME)*2 + 
				GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU), 0);
		SetMenu(g_hWnd, g_menu);

		//Update drawing region
		GetClientRect(g_hWnd, &g_ScreenRect);
		ClientToScreen(g_hWnd, (POINT*)&g_ScreenRect.left);
		ClientToScreen(g_hWnd, (POINT*)&g_ScreenRect.right);
	}

	//Show Window
	ShowWindow(g_hWnd, SW_SHOW);

	//Adjust DirectDraw, hope it doesn't fail.
	system_graphics_init();
}

//-----------------------------------------------------------------------------
// system_message()
//-----------------------------------------------------------------------------
void __cdecl system_message(char* vaMessage,...)
{
	char message[1001];
    va_list vl;

    va_start(vl, vaMessage);
    vsprintf(message, vaMessage, vl);
	va_end(vl);

	if (windowed)
		MessageBox(GetDesktopWindow(), message, PROGRAM_NAME, MB_OK | MB_ICONINFORMATION);
}

//=============================================================================

//-----------------------------------------------------------------------------
// MiscProc()
//-----------------------------------------------------------------------------
BOOL CALLBACK MiscProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		system_language_patch_dialog(IDD_MISC, hDlg);

		Button_SetCheck(GetDlgItem(hDlg,IDC_CHECK_PAUSE), misc_autopause);
		Button_SetCheck(GetDlgItem(hDlg,IDC_CHECK_ONTOP), misc_ontop);
		Button_SetCheck(GetDlgItem(hDlg,IDC_CHECK_STRETCHX), misc_stetchx);

		return 1;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
		case IDOK:
			EndDialog(hDlg, 0);
			if (misc_ontop)
				SetWindowPos(g_hWnd, HWND_TOPMOST, 0,0, 0,0, SWP_NOSIZE | SWP_NOMOVE);
			else
				SetWindowPos(g_hWnd, HWND_NOTOPMOST, 0,0, 0,0, SWP_NOSIZE | SWP_NOMOVE);
			return 1;

		case IDC_CHECK_PAUSE:	misc_autopause = !misc_autopause; return 1;
		case IDC_CHECK_ONTOP:	misc_ontop = !misc_ontop; return 1;
		case IDC_CHECK_STRETCHX:	misc_stetchx = !misc_stetchx; return 1;
		}
		break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// AboutProc()
//-----------------------------------------------------------------------------
BOOL CALLBACK AboutProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		system_language_patch_dialog(IDD_ABOUT, hDlg);
		{
			//Apply version info
			Static_SetText(GetDlgItem(hDlg, IDC_VERSION), NEOPOP_VERSION);
			Static_SetText(GetDlgItem(hDlg, IDC_WIN32VERSION), NEOPOP_WIN32_VERSION);
			return 1;
		}

	case WM_COMMAND:
		if(LOWORD(wParam) == IDCANCEL || LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, 0);
			return 1;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
// PathsProc()
//-----------------------------------------------------------------------------
static void PathBrowse(HWND handle, char* title, char* filter, char* default_dir)
{
	char Selection[256] = "\0";
	char File[256] = "\0";
	OPENFILENAME OpenFileName;

	strcpy(Selection, title);

	ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));
	OpenFileName.lStructSize	= sizeof(OPENFILENAME);
	OpenFileName.hwndOwner		= g_hWnd;
	OpenFileName.hInstance		= g_hInstance;
	OpenFileName.lpstrFilter	= filter;
	OpenFileName.lpstrFile		= Selection;
	OpenFileName.lpstrFileTitle = File;
	OpenFileName.nMaxFileTitle  = 256;
	OpenFileName.nMaxFile		= 256;
	OpenFileName.lpstrInitialDir = default_dir;
	OpenFileName.lpstrTitle		= title;
	OpenFileName.Flags			= OFN_HIDEREADONLY | OFN_PATHMUSTEXIST 
		| OFN_NOCHANGEDIR;

	if (GetOpenFileName(&OpenFileName) != 0)
	{
		//Only get path when load is a success.
		int i;

		//Get path.
		for (i = strlen(Selection)-1; i > 0; i--)
			if (Selection[i] == '\\')
				break;

		Selection[i+1] = 0;
		Edit_SetText(handle, Selection);
	}
}

static void FixPath(char* path_buffer)
{
	int i;

	//Copy, performing a basic check
	for (i = 0; i < 256; i++)
	{
		char c = path_buffer[i];

		if (c == '*' || c == '?' || c == '\"' || c == '<' || c == '>')
			c = '_';

		//Fix Linux style delimiters
		if (c == '/')
			c = '\\';

		path_buffer[i] = c;

		if (c == 0)
			break;
	}
}

BOOL CALLBACK PathsProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		system_language_patch_dialog(IDD_PATHS, hDlg);
		{
			Button_SetCheck(GetDlgItem(hDlg, IDC_LAST_ROMS), LastRomDirectory);
			Button_SetCheck(GetDlgItem(hDlg, IDC_LAST_STATE), LastStateDirectory);

			//Apply current paths
			Edit_SetText(GetDlgItem(hDlg, IDC_PATH_ROM), RomDirectory);
			Edit_SetText(GetDlgItem(hDlg, IDC_PATH_STATE), StateDirectory);
			Edit_SetText(GetDlgItem(hDlg, IDC_PATH_FLASH), FlashDirectory);
			return 1;
		}

	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
			case IDOK:
			{
				Edit_GetText(GetDlgItem(hDlg, IDC_PATH_ROM), RomDirectory, 256);		
				FixPath(RomDirectory);

				Edit_GetText(GetDlgItem(hDlg, IDC_PATH_STATE), StateDirectory, 256);
				FixPath(StateDirectory);

				Edit_GetText(GetDlgItem(hDlg, IDC_PATH_FLASH), FlashDirectory, 256);
				FixPath(FlashDirectory);

				LastRomDirectory = 
					Button_GetCheck(GetDlgItem(hDlg, IDC_LAST_ROMS));
				LastStateDirectory = 
					Button_GetCheck(GetDlgItem(hDlg, IDC_LAST_STATE));

				EndDialog(hDlg, 0);
				return 1;
			}

			case IDC_BROWSE_ROM:
			{
				char string[256];
				Static_GetText(GetDlgItem(hDlg, IDC_ROMPATH), string, 256);
				PathBrowse(GetDlgItem(hDlg, IDC_PATH_ROM), string, 
					system_get_string(IDS_ROMFILTER), RomDirectory);
				return 1;
			}

			case IDC_BROWSE_STATE:
			{
				char string[256];
				Static_GetText(GetDlgItem(hDlg, IDC_STATEPATH), string, 256);
				PathBrowse(GetDlgItem(hDlg, IDC_PATH_STATE), string, 
					system_get_string(IDS_STATEFILTER), StateDirectory);
				return 1;
			}

			case IDC_BROWSE_FLASH:
			{
				char string[256];
				Static_GetText(GetDlgItem(hDlg, IDC_FLASHPATH), string, 256);
				PathBrowse(GetDlgItem(hDlg, IDC_PATH_FLASH), string, 
					system_get_string(IDS_FLASHFILTER), FlashDirectory);
				return 1;
			}

			case IDCANCEL:
			{
				EndDialog(hDlg, 0);
				return 1;
			}
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Recent File List
//-----------------------------------------------------------------------------
static void setMRUMenu(UINT menuId, char* mru)
{
	if (strlen(mru))
	{
		int i = 0;
		char text[256] = "test";

		//Remove path
		for (i = strlen(mru)-1; i > 0; i--)
		{
			if (mru[i] == '\\')
			{
				i++;
				break;
			}
		}

		strcpy(text, mru + i);

		//Remove extension
		for (i = strlen(text)-1; i > 0; i--)
		{
			if (text[i] == '.')
			{
				text[i] = 0;
				break;
			}
		}

		//Fix ampersand
		for (i = 0; i < (int)strlen(text); i++)
		{
			if (text[i] == '&')
			{
				text[i] = '+';
				break;
			}
		}

		ModifyMenu(g_menu, menuId, MF_BYCOMMAND|MF_STRING, menuId, text);
	}
	else
	{
		ModifyMenu(g_menu, menuId, MF_BYCOMMAND|MF_STRING|MF_GRAYED, 
			menuId, "Recent Files");
	}
}

static void buildMRUMenu(void)
{
 	setMRUMenu(ID_GAME_MRU1, MRUFiles[0]);
	setMRUMenu(ID_GAME_MRU2, MRUFiles[1]);
	setMRUMenu(ID_GAME_MRU3, MRUFiles[2]);
	setMRUMenu(ID_GAME_MRU4, MRUFiles[3]);
	setMRUMenu(ID_GAME_MRU5, MRUFiles[4]);
	setMRUMenu(ID_GAME_MRU6, MRUFiles[5]);
	setMRUMenu(ID_GAME_MRU7, MRUFiles[6]);
	setMRUMenu(ID_GAME_MRU8, MRUFiles[7]);
}

static void addMRUMenu(char* filename)
{
	int i, j;

	//Detect the file...
	for (i = 0; i < 8; i++)
	{
		if (stricmp(MRUFiles[i], filename) == 0)
		{
			//Move entry up
			char temp[256];
			strcpy(temp, MRUFiles[i]);
			for (j = i; j >= 1; j--)
				strcpy(MRUFiles[j], MRUFiles[j-1]);
			sprintf(MRUFiles[0], temp);

			//Re-build the MRU menu
			buildMRUMenu();

			return;	//Already there!
		}
	}

	//Move all items down
	for (j = 6; j >= 0; j--)
		strcpy(MRUFiles[j+1], MRUFiles[j]);

	//Add new item
	strcpy(MRUFiles[0], filename);

	//Re-build the MRU menu
	buildMRUMenu();
}

//-----------------------------------------------------------------------------
// windows_load_rom()
//-----------------------------------------------------------------------------
static BOOL windows_load_rom(char* filename)
{
	if (system_load_rom(filename))	//Load the rom
	{
		//Add to MRU
		addMRUMenu(filename);

		//Enable the menu
		EnableMenuItem(g_menu, ID_GAME_UNLOADROM, MF_ENABLED);
		EnableMenuItem(g_menu, ID_GAME_LOADSTATE, MF_ENABLED);
		EnableMenuItem(g_menu, ID_GAME_SAVESTATE, MF_ENABLED);

		reset();
#ifndef NEOPOP_DEBUG
		EnableMenuItem(g_menu, ID_GAME_PAUSE, MF_ENABLED);
		CheckMenuItem(g_menu, ID_GAME_PAUSE, MF_UNCHECKED);
		paused = FALSE;
#endif
		return TRUE;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
// setFrameSkip()
//-----------------------------------------------------------------------------
static void setFrameSkip(void)
{
	system_frameskip_key = max(min(system_frameskip_key,7),1);

	//Check the menu entries
	CheckMenuItem(g_menu, ID_FRAMESKIP_FS0, system_frameskip_key == 1 ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(g_menu, ID_FRAMESKIP_FS1, system_frameskip_key == 2 ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(g_menu, ID_FRAMESKIP_FS2, system_frameskip_key == 3 ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(g_menu, ID_FRAMESKIP_FS3, system_frameskip_key == 4 ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(g_menu, ID_FRAMESKIP_FS4, system_frameskip_key == 5 ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(g_menu, ID_FRAMESKIP_FS5, system_frameskip_key == 6 ? MF_CHECKED : MF_UNCHECKED);
	CheckMenuItem(g_menu, ID_FRAMESKIP_FS6, system_frameskip_key == 7 ? MF_CHECKED : MF_UNCHECKED);
}
	
//-----------------------------------------------------------------------------
// WindowProc()
//-----------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_CLOSE:
		//Get Window position
		if (windowed && (IsIconic(g_hWnd) == 0))
		{
			RECT r;
			GetWindowRect(g_hWnd, &r);
			mainview_x = r.left;
			mainview_y = r.top;
		}

		PostQuitMessage(0);
		return 0;

	case WM_SYSKEYDOWN:	//Allow ALT keys to work
		return 0;

	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
		//Game Menu
		case ID_GAME_LOADROM:
			{
				OPENFILENAME OpenFileName;
				char Selection[256] = "\0";
				char File[256] = "\0";

				if (windowed == FALSE)
				{
					system_graphics_gdi_begin();
					ShowCursor(TRUE);
				}

				ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));
				OpenFileName.lStructSize	= sizeof(OPENFILENAME);
				OpenFileName.hwndOwner		= g_hWnd;
				OpenFileName.hInstance		= g_hInstance;
				OpenFileName.lpstrFilter	= system_get_string(IDS_ROMFILTER);
				OpenFileName.lpstrFile		= Selection;
				OpenFileName.lpstrFileTitle = File;
				OpenFileName.nMaxFileTitle  = 256;
				OpenFileName.nMaxFile		= 256;
				OpenFileName.lpstrInitialDir = RomDirectory;
				OpenFileName.lpstrTitle		= NULL;
				OpenFileName.Flags			= OFN_HIDEREADONLY | OFN_FILEMUSTEXIST
												| OFN_NOCHANGEDIR;

				if (GetOpenFileName(&OpenFileName) != 0)
				{
					if (windows_load_rom(Selection)) //Load the rom
					{
						//Get path.
						if (LastRomDirectory)
						{
							int i;
							for (i = strlen(Selection)-1; i > 0; i--)
								if (Selection[i] == '\\')
									break;

							Selection[i+1] = 0;
							strcpy(RomDirectory, Selection);
						}
					}
				}

				if (windowed == FALSE)
				{
					system_graphics_gdi_end();
					ShowCursor(FALSE);
				}

				return 0;
			}
	
		case ID_GAME_UNLOADROM:
			system_unload_rom();
			return 0;

		case ID_GAME_RESET:
			paused = FALSE;
			CheckMenuItem(g_menu, ID_GAME_PAUSE, MF_UNCHECKED);
			reset();

			return 0;

		case ID_GAME_PAUSE:

			//Allowed?
			if (rom.data == NULL)
				return 0;

			system_set_paused(!paused);
		
			if (paused)
			{
				system_comms_pause(TRUE);
			}
			else
			{
				system_comms_pause(FALSE);
			}
			return 0;

		//Load State
		case ID_GAME_LOADSTATE:
			{
				OPENFILENAME OpenFileName;
				char Selection[256] = "\0";
				char File[256] = "\0";

				//Allowed?
				if (rom.data == NULL)
					return 0;

				if (windowed == FALSE)
				{
					system_graphics_gdi_begin();
					ShowCursor(TRUE);
				}

				if (rom.data)
				{
					strcpy(Selection, rom.filename);
					strcat(Selection, ".ngs");
				}

				ZeroMemory(&OpenFileName, sizeof(OPENFILENAMEW));
				OpenFileName.lStructSize	= sizeof(OPENFILENAMEW);
				OpenFileName.hwndOwner		= g_hWnd;
				OpenFileName.hInstance		= g_hInstance;
				OpenFileName.lpstrFilter	= system_get_string(IDS_STATEFILTER);
				OpenFileName.lpstrFile		= Selection;
				OpenFileName.lpstrFileTitle = File;
				OpenFileName.nMaxFileTitle  = 256;
				OpenFileName.nMaxFile		= 256;
				OpenFileName.lpstrInitialDir = StateDirectory;
				OpenFileName.lpstrTitle		= NULL;
				OpenFileName.Flags			= OFN_HIDEREADONLY | OFN_FILEMUSTEXIST |
					OFN_PATHMUSTEXIST | OFN_EXTENSIONDIFFERENT | OFN_NOCHANGEDIR;
				OpenFileName.lpstrDefExt	= "ngs";

				if (GetOpenFileName(&OpenFileName) != 0)
				{
					state_restore(Selection);

					//Get path.
					if (LastStateDirectory)
					{
						int i;
						for (i = strlen(Selection)-1; i > 0; i--)
							if (Selection[i] == '\\')
								break;
						Selection[i+1] = 0;
						strcpy(StateDirectory, Selection);
					}
				}

				if (windowed == FALSE)
				{
					system_graphics_gdi_end();
					ShowCursor(FALSE);
				}

				//Detect if the colour mode is wrong
				if (system_colour != COLOURMODE_AUTO)
				{
					if (ram[0x6F95] == 0x00)	//Greyscale
					{
						system_colour = COLOURMODE_GREYSCALE;
						CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_COLOUR, MF_UNCHECKED);
						CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_GREYSCALE, MF_CHECKED);
						CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_AUTO, MF_UNCHECKED);
					}

					if (ram[0x6F95] == 0x10)	//Colour
					{
						system_colour = COLOURMODE_COLOUR;
						CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_COLOUR, MF_CHECKED);
						CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_GREYSCALE, MF_UNCHECKED);
						CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_AUTO, MF_UNCHECKED);
					}
				}

				//Unpause
				paused = FALSE;
				system_sound_silence();
				CheckMenuItem(g_menu, ID_GAME_PAUSE, MF_UNCHECKED);
			}
			return 0;

		//Save State
		case ID_GAME_SAVESTATE:
			{
				OPENFILENAME OpenFileName;
				char Selection[256] = "\0";
				char File[256] = "\0";

				//Allowed?
				if (rom.data == NULL)
					return 0;

				if (windowed == FALSE)
				{
					system_graphics_gdi_begin();
					ShowCursor(TRUE);
				}

				if (rom.data)
				{
					strcpy(Selection, rom.filename);
					strcat(Selection, ".ngs");
				}

				ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));
				OpenFileName.lStructSize	= sizeof(OPENFILENAME);
				OpenFileName.hwndOwner		= g_hWnd;
				OpenFileName.hInstance		= g_hInstance;
				OpenFileName.lpstrFilter	= system_get_string(IDS_STATEFILTER);
				OpenFileName.lpstrFile		= Selection;
				OpenFileName.lpstrFileTitle = File;
				OpenFileName.nMaxFileTitle  = 256;
				OpenFileName.nMaxFile		= 256;
				OpenFileName.lpstrInitialDir = StateDirectory;
				OpenFileName.lpstrTitle		= NULL;
				OpenFileName.Flags			= OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |
					OFN_PATHMUSTEXIST | OFN_EXTENSIONDIFFERENT | OFN_NOCHANGEDIR;
				OpenFileName.lpstrDefExt	= "ngs";

				if (GetSaveFileName(&OpenFileName) != 0)
				{
					state_store(Selection);

					//Get path.
					if (LastStateDirectory)
					{
						int i;
						for (i = strlen(Selection)-1; i > 0; i--)
							if (Selection[i] == '\\')
								break;
						Selection[i+1] = 0;
						strcpy(StateDirectory, Selection);
					}
				}

				if (windowed == FALSE)
				{
					system_graphics_gdi_end();
					ShowCursor(FALSE);
				}

				return 0;
			}

		case ID_GAME_MRU1:
		case ID_GAME_MRU2:
		case ID_GAME_MRU3:
		case ID_GAME_MRU4:
		case ID_GAME_MRU5:
		case ID_GAME_MRU6:
		case ID_GAME_MRU7:
		case ID_GAME_MRU8:
			{
				int id = LOWORD(wParam) - ID_GAME_MRU1;
				
				if (strlen(MRUFiles[id]) == 0)
					return 0;
				
				//Load the rom?
				if (windows_load_rom(MRUFiles[id]) == FALSE)
				{
					int i;

					//Remove offending entry!
					for (i = id; i < 7; i++)
						strcpy(MRUFiles[i], MRUFiles[i+1]);
					sprintf(MRUFiles[7], "\0");
				}
			
				buildMRUMenu();
			}
			return 0;

		case ID_GAME_RESETMRU:
			{
				int i;
				for (i = 0; i < 8; i++)
					sprintf(MRUFiles[i], "\0");
				buildMRUMenu();
			}
			return 0;

		case ID_GAME_EXIT:
			//Get Window position
			if (windowed && (IsIconic(g_hWnd) == 0))
			{
				RECT r;
				GetWindowRect(g_hWnd, &r);
				mainview_x = r.left;
				mainview_y = r.top;
			}

			PostQuitMessage(0);
			return 0;

		//Options Menu
		case ID_OPTIONS_DISPLAYTYPE_GREYSCALE:
			if (system_colour != COLOURMODE_GREYSCALE)
			{
				system_colour = COLOURMODE_GREYSCALE;
				CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_COLOUR, MF_UNCHECKED);
				CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_GREYSCALE, MF_CHECKED);
				CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_AUTO, MF_UNCHECKED);
				PostMessage(g_hWnd, WM_COMMAND, ID_GAME_RESET, 0);
			}
			return 0;

		case ID_OPTIONS_DISPLAYTYPE_COLOUR:
			if (system_colour != COLOURMODE_COLOUR)
			{
				system_colour = COLOURMODE_COLOUR;
				CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_COLOUR, MF_CHECKED);
				CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_GREYSCALE, MF_UNCHECKED);
				CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_AUTO, MF_UNCHECKED);
				PostMessage(g_hWnd, WM_COMMAND, ID_GAME_RESET, 0);
			}
			return 0;

		case ID_OPTIONS_DISPLAYTYPE_AUTO:
			if (system_colour != COLOURMODE_AUTO)
			{
				system_colour = COLOURMODE_AUTO;
				CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_COLOUR, MF_UNCHECKED);
				CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_GREYSCALE, MF_UNCHECKED);
				CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_AUTO, MF_CHECKED);
				PostMessage(g_hWnd, WM_COMMAND, ID_GAME_RESET, 0);
			}
			return 0;

		case ID_OPTIONS_LANGUAGE_ENGLISH:
			language_english = TRUE;
			CheckMenuItem(g_menu, ID_OPTIONS_LANGUAGE_ENGLISH, MF_CHECKED);
			CheckMenuItem(g_menu, ID_OPTIONS_LANGUAGE_JAPANESE, MF_UNCHECKED);
			return 0;

		case ID_OPTIONS_LANGUAGE_JAPANESE:
			language_english = FALSE;
			CheckMenuItem(g_menu, ID_OPTIONS_LANGUAGE_ENGLISH, MF_UNCHECKED);
			CheckMenuItem(g_menu, ID_OPTIONS_LANGUAGE_JAPANESE, MF_CHECKED);
			return 0;

		case ID_OPTIONS_PATHS:
			DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_PATHS), g_hWnd, PathsProc);
 			return 0;

		case ID_OPTIONS_CONTROLS:
			DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_CONTROLS), g_hWnd, InputProc);
 			return 0;

		case ID_OPTIONS_MISC:
			DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_MISC), g_hWnd, MiscProc);
 			return 0;

		case ID_OPTIONS_FULLSCREEN:	
#ifndef NEOPOP_DEBUG
			switchDisplayMode();
			system_graphics_gdi_begin();
			system_graphics_gdi_end();
#endif
			return 0;
			
		case ID_OPTIONS_ZOOMX1: setZoom(1);	return 0;
		case ID_OPTIONS_ZOOMX2: setZoom(2);	return 0;
		case ID_OPTIONS_ZOOMX3: setZoom(3);	return 0;
		case ID_OPTIONS_ZOOMX4: setZoom(4);	return 0;

		case ID_FRAMESKIP_FS0: system_frameskip_key = 1; setFrameSkip(); return 0;
		case ID_FRAMESKIP_FS1: system_frameskip_key = 2; setFrameSkip(); return 0;
		case ID_FRAMESKIP_FS2: system_frameskip_key = 3; setFrameSkip(); return 0;
		case ID_FRAMESKIP_FS3: system_frameskip_key = 4; setFrameSkip(); return 0;
		case ID_FRAMESKIP_FS4: system_frameskip_key = 5; setFrameSkip(); return 0;
		case ID_FRAMESKIP_FS5: system_frameskip_key = 6; setFrameSkip(); return 0;
		case ID_FRAMESKIP_FS6: system_frameskip_key = 7; setFrameSkip(); return 0;
		case ID_FRAMESKIP_LESS: system_frameskip_key --; setFrameSkip(); return 0;
		case ID_FRAMESKIP_MORE: system_frameskip_key ++; setFrameSkip(); return 0;

		case ID_OPTIONS_MUTE:
			mute = !mute;
			system_sound_silence();
			if (mute)
			{
				CheckMenuItem(g_menu, ID_OPTIONS_MUTE, MF_CHECKED);
			}
			else
			{
				CheckMenuItem(g_menu, ID_OPTIONS_MUTE, MF_UNCHECKED);
			}
			return 0;

		case ID_OPTIONS_CONNECT:
			system_comms_connect_dialog();
			return 0;

		//Help Menu
		case ID_HELP_ABOUTNEOPOP:
			DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_ABOUT), g_hWnd, AboutProc);
 			return 0;
		}
		break;

		//Avoid nasty sound stuttering when interacting with the
		//Windows interface. Sound will be restarted on the next
		//screen update
	case WM_MENUSELECT:
	case WM_ENTERSIZEMOVE:
		system_sound_silence();
		return 0;

	case WM_SIZE:
		system_sound_silence();
		if (wParam == SIZE_MINIMIZED)
		{
			auto_pause = TRUE;
			break;
		}
		if (wParam == SIZE_RESTORED)
		{
			auto_pause = FALSE;
			break;
		}
		//<=== Fall through!
	case WM_MOVE:
		GetClientRect(hWnd, &g_ScreenRect);
		ClientToScreen(hWnd, (POINT*)&g_ScreenRect.left);
		ClientToScreen(hWnd, (POINT*)&g_ScreenRect.right);
		return 0;

	case WM_RBUTTONDOWN:
		{
			HMENU sub = GetSubMenu(GetSubMenu(g_menu, 0), 1);

			system_sound_silence();

			GetClientRect(hWnd, &g_ScreenRect);
			ClientToScreen(hWnd, (POINT*)&g_ScreenRect.left);
			ClientToScreen(hWnd, (POINT*)&g_ScreenRect.right);

			ShowCursor(TRUE);
			system_graphics_gdi_begin();
			TrackPopupMenu(sub, TPM_RIGHTBUTTON,
				LOWORD(lParam) + g_ScreenRect.left, 
				HIWORD(lParam) + g_ScreenRect.top, 
				0, g_hWnd, 0);
			system_graphics_gdi_end();
			ShowCursor(FALSE);
		}
 
		return 0;

	case WM_PAINT:
		if (windowed && system_graphics_ready())
			system_graphics_update_window();
		break;

#ifndef NEOPOP_DEBUG
	case WM_ACTIVATE:
		system_sound_silence();

		if (misc_autopause)
		{
			auto_pause = ! ((LOWORD(wParam) == WA_ACTIVE) || 
							(LOWORD(wParam) == WA_CLICKACTIVE));

			if (auto_pause)
			{
				if (rom.data)
					SetWindowText(g_hWnd, PROGRAM_NAME" - Paused");
			}
			else
			{
				if (rom.data)
				{
					char title[128];
					sprintf(title, PROGRAM_NAME" - %s", rom.name);
					SetWindowText(g_hWnd, title);
				}
			}
		}

		return 0;
#endif
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


//=============================================================================

//-----------------------------------------------------------------------------
// WinInit()
//-----------------------------------------------------------------------------
BOOL WinInit(void)
{
    WNDCLASSEX wc;
	UINT m;

	// Set up and register window class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;					
	wc.hInstance = g_hInstance;
	wc.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE( IDI_ICON ));
	wc.hCursor = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "NeoPop";
	wc.hIconSm = NULL;
	RegisterClassEx(&wc);	// Register the class
	
	g_menu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_MENU));

	//Apply language pack straight away
	system_language_patch();

	//Check the Zoom menu entries
	if (zoom == 1) m = MF_CHECKED; else m = MF_UNCHECKED;
	CheckMenuItem(g_menu, ID_OPTIONS_ZOOMX1, m);
	if (zoom == 2) m = MF_CHECKED; else m = MF_UNCHECKED;
	CheckMenuItem(g_menu, ID_OPTIONS_ZOOMX2, m);
	if (zoom == 3) m = MF_CHECKED; else m = MF_UNCHECKED;
	CheckMenuItem(g_menu, ID_OPTIONS_ZOOMX3, m);
	if (zoom == 4) m = MF_CHECKED; else m = MF_UNCHECKED;
	CheckMenuItem(g_menu, ID_OPTIONS_ZOOMX4, m);

	//Check the pause menu item
	CheckMenuItem(g_menu, ID_GAME_PAUSE, MF_UNCHECKED);
	
	//Set the mute state
	if (mute)
		CheckMenuItem(g_menu, ID_OPTIONS_MUTE, MF_CHECKED);
	else
		CheckMenuItem(g_menu, ID_OPTIONS_MUTE, MF_UNCHECKED);

	//Check the language
	if (language_english)
	{
		CheckMenuItem(g_menu, ID_OPTIONS_LANGUAGE_ENGLISH, MF_CHECKED);
		CheckMenuItem(g_menu, ID_OPTIONS_LANGUAGE_JAPANESE, MF_UNCHECKED);
	}
	else
	{
		CheckMenuItem(g_menu, ID_OPTIONS_LANGUAGE_ENGLISH, MF_UNCHECKED);
		CheckMenuItem(g_menu, ID_OPTIONS_LANGUAGE_JAPANESE, MF_CHECKED);
	}

	//Check the display
	if (system_colour == COLOURMODE_COLOUR)	//Colour
	{
		CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_COLOUR, MF_CHECKED);
		CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_GREYSCALE, MF_UNCHECKED);
		CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_AUTO, MF_UNCHECKED);
	}
	
	if (system_colour == COLOURMODE_GREYSCALE) //Mono
	{
		CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_COLOUR, MF_UNCHECKED);
		CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_GREYSCALE, MF_CHECKED);
		CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_AUTO, MF_UNCHECKED);
	}

	if (system_colour == COLOURMODE_AUTO) //Auto
	{
		CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_COLOUR, MF_UNCHECKED);
		CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_GREYSCALE, MF_UNCHECKED);
		CheckMenuItem(g_menu, ID_OPTIONS_DISPLAYTYPE_AUTO, MF_CHECKED);
	}
	
	//Disable the unload + pause
	EnableMenuItem(g_menu, ID_GAME_UNLOADROM, MF_GRAYED);
	
	//Can't pause the bios!
	EnableMenuItem(g_menu, ID_GAME_PAUSE, MF_GRAYED);
	EnableMenuItem(g_menu, ID_GAME_LOADSTATE, MF_GRAYED);
	EnableMenuItem(g_menu, ID_GAME_SAVESTATE, MF_GRAYED);

	//Create the application window
	if (windowed)
	{
		if (misc_ontop)
		g_hWnd = CreateWindowEx(WS_EX_TOPMOST, "NeoPop", PROGRAM_NAME,
			WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			mainview_x,	mainview_y,	SCREEN_WIDTH, SCREEN_HEIGHT,
			NULL, NULL, g_hInstance, NULL);
		else
		g_hWnd = CreateWindowEx(0, "NeoPop", PROGRAM_NAME,
			WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			mainview_x,	mainview_y,	SCREEN_WIDTH, SCREEN_HEIGHT,
			NULL, NULL, g_hInstance, NULL);

	}
	else
	{
		if (misc_ontop)
		g_hWnd = CreateWindowEx(WS_EX_TOPMOST, "NeoPop", PROGRAM_NAME,
			WS_POPUP, 
			0, 0, 640, 480,
			NULL, NULL, g_hInstance, NULL);
		else
		g_hWnd = CreateWindowEx(0, "NeoPop", PROGRAM_NAME,
			WS_POPUP, 
			0, 0, 640, 480,
			NULL, NULL, g_hInstance, NULL);
		ShowCursor(FALSE);
	}
	if (g_hWnd == NULL)	//Do an error check
		return FALSE;
	
	//Attach menu?
	if (windowed) SetMenu(g_hWnd, g_menu);

	//Setup "Window Size" menu
	setZoom(zoom);

	//Setup "Frameskip" menu
	setFrameSkip();

	//Make the window appear on top
	ShowWindow(g_hWnd, SW_SHOWNORMAL); 
  	SetFocus(g_hWnd);

	//Load keyboard accelerator
	g_accelerator = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_ACCEL));
	if (g_accelerator == NULL)
		return FALSE;

	//Build recent file list
	buildMRUMenu();

	//Successful initialisation
	return TRUE;
}

//=============================================================================

//-----------------------------------------------------------------------------
// WinMain()
//-----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE a, LPSTR cmdline, int c)
{
	MSG msg;
	g_hInstance = hInstance; // Copy instance for use by DirectX

	//Fill the bios buffer however it needs to be...
	if (bios_install() == FALSE)
		return 0;

	system_get_config();	//Read configuration settings

#ifdef NEOPOP_DEBUG
	windowed = TRUE;
#endif

	//Create the window and setup the menus
	if (WinInit() == FALSE)
	{
		system_message(system_get_string(IDS_ERROR1));
		return 0;
	}

	//Initialise Draw
	if (system_graphics_init() == FALSE)
	{
		system_message(system_get_string(IDS_ERROR2));
		return 0;
	}

	//Initialise Input
	if (system_input_init() == FALSE)
	{
		system_message(system_get_string(IDS_ERROR3));
		system_graphics_shutdown();
		return 0;
	}

	//Initialise Sound
	if (system_sound_init() == FALSE)
	{
		//Don't care too much, just disable the mute option to
		//give the user some small indication.
		EnableMenuItem(g_menu, ID_OPTIONS_MUTE, MF_GRAYED);
		CheckMenuItem(g_menu, ID_OPTIONS_MUTE, MF_CHECKED);
		mute = TRUE;
	}

#ifdef NEOPOP_DEBUG
	EnableMenuItem(g_menu, ID_OPTIONS_FULLSCREEN, MF_GRAYED);
	system_debug_init();	//Open the debugger
#endif

	//=========

	//Attempt to load from the command line
	if (strlen(cmdline) > 4)
		windows_load_rom(__argv[1]);

	reset();

	//Get Throttle speed
	if (QueryPerformanceFrequency(&throttleRate_L))
	{
		throttleRate = (_u64)(throttleRate_L.QuadPart / 59.95);
		throttleRate100 = (_u32)((throttleRate_L.QuadPart * 100) / 59.95);
	}
	else
	{
		system_message(system_get_string(IDS_TIMER));
		return 0;
	}

	//=========

	do
	{
#ifdef NEOPOP_DEBUG
		//======================================
		// DEBUGGER BUILD - MAIN LOOP
		//======================================
		if (system_debug_running())
		{
			//Stop BEFORE the breakpoint
			if (breakpoint_reached() == FALSE)
				emulate_debug(running_dis_TLCS900h, running_dis_Z80);
		}
		//======================================
		//======================================
#else
		//======================================
		// MAIN BUILD - MAIN LOOP
		//======================================
		if (paused || (auto_pause && misc_autopause))
		{
			system_comms_pause_check();
			Sleep(50);
		}
		else
			emulate();
		//======================================
		//======================================
#endif

		//Message Pump
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (!TranslateAccelerator(g_hWnd, g_accelerator, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	while(msg.message != WM_QUIT);

	//=========

	//Shutdown
#ifdef NEOPOP_DEBUG
	system_debug_shutdown();
#endif

	//Close any connection.
	system_comms_shutdown();

	system_unload_rom();

	system_sound_shutdown();
	system_input_shutdown();
	system_graphics_shutdown();

	system_put_config();	//Write configuration settings

	//exit.
	DestroyMenu(g_menu);
	UnregisterClass("NeoPop", g_hInstance);
	return msg.wParam;
}

//=============================================================================
