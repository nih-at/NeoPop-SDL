
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

	debug_watch.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

05 SEP 2002 - neopop_uk
=======================================
- Created this file to allow memory locations to be 'watched'.
- Memory editing.

06 SEP 2002 - neopop_uk
=======================================
- Added simple loading and saving of the watch list. I wouldn't recomment
editing the file, the reader isn't very robust!

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include <windows.h>
#include <windowsx.h>
#include "..\resource.h"

#include "system_debug.h"
#include "..\system_main.h"
#include "..\system_config.h"

#include "mem.h"

#ifdef NEOPOP_DEBUG
//=============================================================================

static HWND g_WatchWnd, h_list;
BOOL watchview_enabled = FALSE;

#define MAX_WATCH 1024

typedef struct
{
	_u32 address;
	char description[32];
}
WATCH;

static WATCH watch[MAX_WATCH];

static int watch_count = 0;

//=============================================================================

BOOL CALLBACK WatchProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		return FALSE;

	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			watch_close();
			return 1;

		case IDC_WATCHLIST:
			{
				int index = ListBox_GetCurSel(h_list);
				if (index >= 0 && index < watch_count)
				{
					_u32 data;
					debug_mask_memory_error_messages = TRUE;
					data = loadL(watch[index].address);
					debug_mask_memory_error_messages = FALSE;
					if (debug_abort_memory == FALSE)
					{
						char text[16];
						sprintf(text, "%08X", data);
						Edit_SetText(GetDlgItem(hDlg, IDC_DATA), text);
					}
						
					debug_abort_memory = FALSE;
					watch_update();
				}
			}
			break;

		case IDC_ADD:
			{
				char text[10];
				char* dummy;
				_s32 address;

				Edit_GetText(GetDlgItem(hDlg, IDC_ADDRESS), text, 9);
				address = abs(strtol(text, &dummy, 16)) & 0xFFFFFF;
				if (dummy == text)
					return 1;

				if (watch_count < MAX_WATCH)
				{
					unsigned int i;

					watch[watch_count].address = (_u32)(address & 0xFFFFFF);
					Edit_GetText(GetDlgItem(hDlg, IDC_DESCRIPTION), watch[watch_count].description, 30);
					
					if (strlen(watch[watch_count].description) == 0)
						strcpy(watch[watch_count].description, "unknown");

					for (i = 0; i < strlen(watch[watch_count].description); i++)
						if (watch[watch_count].description[i] <= 32 ||
							watch[watch_count].description[i] > 127)
							watch[watch_count].description[i] = '_';

					watch_count++;
					watch_update();
					ListBox_SetCurSel(h_list, watch_count);
				}
			}
			return 1;

		case IDC_REMOVE:
			{
				int index = ListBox_GetCurSel(h_list);
				if (index >= 0 && index < watch_count)
				{
					watch_count --;

					//Delete!
					if (index < watch_count)
					{
						watch[index].address = watch[watch_count].address;
						strcpy(watch[index].description, watch[watch_count].description);
					}

					watch_update();
				}
			}
			return 1;

		case IDC_APPLY:
			{
				int index = ListBox_GetCurSel(h_list);
				if (index >= 0)
				{
					char text[16];
					char* dummy;
					_u32 data;

					Edit_GetText(GetDlgItem(hDlg, IDC_DATA), text, 15);
					data = abs(strtol(text, &dummy, 16)) & 0xFFFFFFFF;
					if (dummy == text)
						return 1;

					debug_mask_memory_error_messages = TRUE;
					storeL(watch[index].address, data);
					debug_mask_memory_error_messages = FALSE;
					debug_abort_memory = FALSE;
					watch_update();
				}
			}
			return 1;

		case IDC_SAVE:
			{
				OPENFILENAME OpenFileName;
				char Selection[256] = "\0";
				char File[256] = "\0";

				if (watch_count == 0)
				{
					system_message("Cannot save, the list is empty.");
					return 1;
				}

				if (rom.data)
				{
					strcpy(Selection, rom.filename);
					strcat(Selection, ".txt");
				}

				ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));
				OpenFileName.lStructSize	= sizeof(OPENFILENAME);
				OpenFileName.hwndOwner		= g_hWnd;
				OpenFileName.hInstance		= g_hInstance;
				OpenFileName.lpstrFilter	= "Text Files (*.txt)\0*.txt\0\0";
				OpenFileName.lpstrFile		= Selection;
				OpenFileName.lpstrFileTitle = File;
				OpenFileName.nMaxFileTitle  = 256;
				OpenFileName.nMaxFile		= 256;
				OpenFileName.lpstrInitialDir = NULL;
				OpenFileName.lpstrTitle		= NULL;
				OpenFileName.Flags			= OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |
					OFN_PATHMUSTEXIST | OFN_EXTENSIONDIFFERENT | OFN_NOCHANGEDIR;
				OpenFileName.lpstrDefExt	= "txt";

				if (GetSaveFileName(&OpenFileName) != 0)
				{
					FILE* fp = fopen(Selection, "w");
					if (fp)
					{
						int i;

						fprintf(fp,"%d\n", watch_count);

						for (i = 0; i < watch_count; i++)
							fprintf(fp, "%X,%s\n", watch[i].address, watch[i].description);

						fprintf(fp,"\n\n# Created automatically, do not edit!\n");
						fclose(fp);
					}
					else
					{
						system_message("Cannot save list to: %s", Selection);
					}
				}
			}
			return 1;
		
		case IDC_LOAD:
			{
				OPENFILENAME OpenFileName;
				char Selection[256] = "\0";
				char File[256] = "\0";

				if (rom.data)
				{
					strcpy(Selection, rom.filename);
					strcat(Selection, ".txt");
				}

				ZeroMemory(&OpenFileName, sizeof(OPENFILENAME));
				OpenFileName.lStructSize	= sizeof(OPENFILENAME);
				OpenFileName.hwndOwner		= g_hWnd;
				OpenFileName.hInstance		= g_hInstance;
				OpenFileName.lpstrFilter	= "Text Files (*.txt)\0*.txt\0\0";
				OpenFileName.lpstrFile		= Selection;
				OpenFileName.lpstrFileTitle = File;
				OpenFileName.nMaxFileTitle  = 256;
				OpenFileName.nMaxFile		= 256;
				OpenFileName.lpstrInitialDir = NULL;
				OpenFileName.lpstrTitle		= NULL;
				OpenFileName.Flags			= OFN_HIDEREADONLY | OFN_FILEMUSTEXIST |
					OFN_PATHMUSTEXIST | OFN_EXTENSIONDIFFERENT | OFN_NOCHANGEDIR;
				OpenFileName.lpstrDefExt	= "txt";

				if (GetOpenFileName(&OpenFileName) != 0)
				{
					FILE* fp = fopen(Selection, "r");
					if (fp)
					{
						int i;

						fscanf(fp,"%d\n", &watch_count);

						for (i = 0; i < watch_count; i++)
							fscanf(fp, "%X,%s\n", &(watch[i].address), watch[i].description);

						fclose(fp);
					}
					else
					{
						system_message("Cannot load list : %s", Selection);
					}
				}
		
				watch_update();
			}
			return 1;
		}

		break;
	}

	return 0;
}

//=============================================================================

void watch_update(void)
{
	int i;

	int index = ListBox_GetCurSel(h_list);

	ListBox_ResetContent(h_list);

/*	if (system_debug_running())
	{
		ListBox_AddString(h_list, "Cannot watch while the emulator is running.");
		return;
	}*/

	//Update list
	for (i = 0; i < watch_count; i++)
	{
		char line[80];
		_u32 data;
		
		debug_abort_memory = FALSE;
		debug_mask_memory_error_messages = TRUE;
		data = loadL(watch[i].address);

		if (debug_abort_memory)
		{
			sprintf(line, "%06X    n/a        %s", watch[i].address,
				watch[i].description);
		}
		else
		{
			sprintf(line, "%06X    %08X   %s", watch[i].address, data,
				watch[i].description);
		}
		ListBox_AddString(h_list, line);
		debug_mask_memory_error_messages = FALSE;
		debug_abort_memory = FALSE;
	}

	ListBox_SetCurSel(h_list, index);
}

//=============================================================================

void watch_open(void)
{
	g_WatchWnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_WATCH), g_hWnd, WatchProc);
	if (g_WatchWnd == NULL)
		return;

	//Position the window
	SetWindowPos(g_WatchWnd, NULL, watchview_x, watchview_y, 0,0,
		SWP_NOSIZE | SWP_NOOWNERZORDER);

	h_list = GetDlgItem(g_WatchWnd, IDC_WATCHLIST);
	watchview_enabled = TRUE;
	ShowWindow(g_WatchWnd, SW_SHOWNORMAL);
	watch_update();
}

void watch_close(void)
{
	if (watchview_enabled)
	{
		watchview_enabled = FALSE;
		//Get Window position
		if (IsIconic(g_WatchWnd) == 0)
		{
			RECT r;
			GetWindowRect(g_WatchWnd, &r);
			watchview_x = r.left;
			watchview_y = r.top;
		}

		DestroyWindow(g_WatchWnd);
		g_WatchWnd = NULL;
	}
}

//=============================================================================
#endif
