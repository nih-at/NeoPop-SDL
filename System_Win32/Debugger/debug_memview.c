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

	debug_memview.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

21 JUL 2002 - neopop_uk
=======================================
- Moved this file into the 'Debugger' folder. This is to aid
abstraction, and for porters who don't need to port the debugger.

21 JUL 2002 - neopop_uk
=======================================
- Made the memory alter address box so that it doesn't clear. This
makes it a little easier to enter multiple bytes.


//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include <windows.h>
#include <windowsx.h>
#include "..\resource.h"

#include "system_debug.h"
#include "..\system_main.h"
#include "..\system_config.h"

#include "TLCS900h_registers.h"
#include "mem.h"

#ifdef NEOPOP_DEBUG
//=============================================================================

BOOL memview_enabled = FALSE;
static HWND g_MemWnd;
static _u32 address = 0;
static HWND edit, alter, databox;

//=============================================================================

static _s32 getAddress(HWND editbox)
{
	char text[10];
	char* dummy;
	_s32 address;
	Edit_GetText(editbox, text, 9);
	address = abs(strtol(text, &dummy, 16)) & 0xFFFFFF;
//	Edit_SetText(editbox, "");
	if (dummy == text)
		return -1;
	return (_s32)address;
}

static _s16 getData()
{
	char text[10];
	char* dummy;
	_s16 address;
	Edit_GetText(databox, text, 3);
	address = abs(strtol(text, &dummy, 16)) & 0xFF;
//	Edit_SetText(databox, "");
	if (dummy == text)
		return -1;
	return (_s16)address;
}

BOOL CALLBACK MemProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			//
		}
		return 0;

	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			mem_close();
			return 1;

		case IDC_UP:
			if (address > 0)
				address -= 0x10;
			mem_update();
			break;

		case IDC_DOWN:
			if (address < 0xFFFFFF)
				address += 0x10;
			mem_update();
			break;

		case IDC_SET:
			{
				_s32 get = getAddress(edit);
				if (get != -1)
				{
					address = get & 0xFFFFF0;
					mem_update();
				}
			}
			break;

		case IDC_SETDATA:
			{
				_s32 get = getAddress(alter);
				_s16 data = getData();

				if (get != -1 && data != -1)
				{
					memory_unlock_flash_write = TRUE;	//Allow rom changes
					storeB(get, (_u8)data);
					memory_unlock_flash_write = FALSE;
					system_debug_refresh();
				}
			}
			break;

		case IDC_0000:	address = 0x0000;	mem_update();	break;
		case IDC_4000:	address = 0x4000;	mem_update();	break;
		case IDC_6F00:	address = 0x6F00;	mem_update();	break;
		case IDC_7000:	address = 0x7000;	mem_update();	break;
		case IDC_8000:	address = 0x8000;	mem_update();	break;
		case IDC_ROM:	address = 0x200000;	mem_update();	break;
		}
	}

	return 0;
}

//=============================================================================

#pragma auto_inline(off)
static void dump_memory(_u32 start, _u32 length)
{
	_u32 i, j;
	char str[80];

	//Align the dump on a 16 byte boundary
	address = start = start & 0xFFFFF0;
	
	sprintf(str, "%06X", start);
	Edit_SetText(edit, str);

	debug_abort_memory = FALSE;
	debug_mask_memory_error_messages = TRUE;

	for (i = 0; i < length; i+= 16)
	{
		sprintf(str, "%06X : ", i + start);

		//Print data in hexadecimal
		for (j = 0; j < 16; j++)
		{
			_u8 b = loadB(i + j + start);

			//Ignore memory location 0
			if ((i + j + start) == 0)
				debug_abort_memory = FALSE;

			if (debug_abort_memory)
			{
				debug_mask_memory_error_messages = FALSE;
				sprintf(str, "%06X : ...no memory here...", i + start);
				SendDlgItemMessage(g_MemWnd, IDC_LIST, LB_ADDSTRING, 0, (long)(&str));
				return;
			}

			if (i + j >= length)
				strcat(str, "   ");
			else
			{
				char add[4];
				sprintf(add, "%0.2x ", b); 
				strcat(str, add);
			}
			if (j % 8 == 7)
				strcat(str, ": ");
		}

		//And character form
		for (j = 0; j < 16; j++)
		{
			_u8 byte;

			if (i + j >= length)
				break;

			byte = loadB(i + j + start);

			//Ignore memory location 0
			if ((i + j + start) == 0)
				debug_abort_memory = FALSE;

			if (byte >= 32) 
			{
				char add[2];
				add[0] = byte;
				add[1] = 0;
				strcat(str, add);
			}
			else 
				strcat(str, ".");
		}
		
		SendDlgItemMessage(g_MemWnd, IDC_LIST, LB_ADDSTRING, 0, (long)(&str));
	}

	debug_mask_memory_error_messages = FALSE;
}
#pragma auto_inline(off)

//=============================================================================

void mem_update(void)
{
	if (memview_enabled && edit && g_MemWnd)
	{
		SendDlgItemMessage(g_MemWnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
		dump_memory(address, 0x200);
	}
}

//=============================================================================

void mem_open(void)
{
	g_MemWnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_MEMVIEW), g_hWnd, MemProc);
	if (g_MemWnd == NULL)
		return;

	//Position the window
	SetWindowPos(g_MemWnd, NULL, memview_x, memview_y, 0,0,
		SWP_NOSIZE | SWP_NOOWNERZORDER);

	memview_enabled = TRUE;
	ShowWindow(g_MemWnd, SW_SHOWNORMAL);

	edit = GetDlgItem(g_MemWnd, IDC_EDIT);
	alter = GetDlgItem(g_MemWnd, IDC_ADDRESS);
	databox = GetDlgItem(g_MemWnd, IDC_DATA);

	mem_update();
}

void mem_close(void)
{
	if (memview_enabled)
	{
		//Get Window position
		if (IsIconic(g_MemWnd) == 0)
		{
			RECT r;
			GetWindowRect(g_MemWnd, &r);
			memview_x = r.left;
			memview_y = r.top;
		}

		DestroyWindow(g_MemWnd);
		memview_enabled = FALSE;
		edit = NULL;
		g_MemWnd = NULL;
	}
}

//=============================================================================
#endif