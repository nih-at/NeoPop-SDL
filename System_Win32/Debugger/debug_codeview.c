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

	debug_codeview.c

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

05 SEP 2002 - neopop_uk
=======================================
- General improvements, support for displaying multiple breakpoints.

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
#include "Z80_interface.h"

#ifdef NEOPOP_DEBUG
//=============================================================================

static HWND g_CodeWnd, h_TLCS900h, h_Z80;
BOOL codeview_enabled = FALSE;
BOOL view_z80 = FALSE;
#define HISTORY_ITEMS		8
#define AUTO_ITEMS			32
static HWND h_auto = NULL, h_history = NULL;
_s32 auto_list[AUTO_ITEMS];
_s32 auto_history_list[HISTORY_ITEMS];

//=============================================================================

BOOL CALLBACK CodeProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		return 0;

	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			code_close();
			return 1;

		case IDC_AUTO:
			{
				int index = ListBox_GetCurSel(h_auto);

				if (view_z80 == TRUE && Z80ACTIVE == FALSE)
					break;

				if (index >= 0 && codeview_enabled && system_debug_running() == FALSE)
					set_debug_address(auto_list[index]);
			}
			break;

		case IDC_HISTORY:
			{
				int index = ListBox_GetCurSel(h_history);
				if (index >= 0 && codeview_enabled && system_debug_running() == FALSE)
					set_debug_address(auto_history_list[index]);
			}
			break;

		case IDC_CODE_Z80:
			view_z80 = TRUE;
			code_update();
			break;

		case IDC_CODE_TLCS900h:
			view_z80 = FALSE;
			code_update();
			break;
		}

		break;
	}

	return 0;
}

//=============================================================================

void auto_history(int index)
{
	char msg[100], *s;

	if (auto_history_list[index] != -1)
	{
		_u32 current;
		pc = auto_history_list[index];
		current = pc & 0xFFFFFF;
			
		debug_abort_memory = FALSE;

		//Print some instructions
		s = disassemble();

		if (debug_abort_memory == FALSE)
		{
			int i;
			sprintf(msg, "    %s", s);
			for (i = 0; i < MAX_BREAKPOINTS; i++)
			{
				if (breakpoint[i].enabled && breakpoint[i].address == current)
				{
					sprintf(msg, "B:%d %s", i, s);
					break;
				}
			}

			ListBox_AddString(h_history, msg);
		}
		else
		{
			sprintf(msg, "    ???: Bad Execution Address");
			ListBox_AddString(h_history, msg);
			auto_history_list[index] = -1;
		}

		free(s);
	}
}

void system_debug_history_add(void)
{
	int i = 0;

	for (i = 0; i < HISTORY_ITEMS; i++)
	{
		if (auto_history_list[i] == -1)
		{
			auto_history_list[i] = pc;
			return;
		}
	}

	for (i = 1; i < HISTORY_ITEMS; i++)
		auto_history_list[i-1] = auto_history_list[i];
	auto_history_list[HISTORY_ITEMS - 1] = pc;
}

void system_debug_history_clear(void)
{
	int i;
	for (i = 0; i < HISTORY_ITEMS; i++)
		auto_history_list[i] = -1;
	ListBox_ResetContent(h_history);
}

//=============================================================================

void auto_add(_u32 oldpc, int index)
{
	char msg[100], *s;

	if (view_z80)
	{
		if (Z80ACTIVE)
		{
			//Disassemble
			_u16 pc = Z80_getReg(Z80_REG_PC);
			_u32 current = pc & 0xFFFFFF;

			auto_list[index] = pc + 0x7000;
			s = Z80_disassemble(&pc);
			Z80_setReg(Z80_REG_PC, pc);
			if (index == 0)
				sprintf(msg, "==> %s", s);
			else
			{
				if (breakpoint_z80.enabled && breakpoint_z80.address == current)
					sprintf(msg, "Brk %s", s);
				else
					sprintf(msg, "    %s", s);
			}

			ListBox_AddString(h_auto, msg);
		}
		else
		{
			debug_abort_memory = TRUE;
			if (index == 0)
			{
				sprintf(msg, "    Z80 is not active, cannot auto disassemble");
				ListBox_AddString(h_auto, msg);
			}
		}
	}
	else
	{
		_u32 current = pc & 0xFFFFFF;
		debug_abort_memory = FALSE;

		//Print some instructions
		s = disassemble();

		if (debug_abort_memory)
		{
			if (index == 0)
			{
				sprintf(msg, "    ???: $PC = 0x%06x", current);
				ListBox_AddString(h_auto, msg);
			}

			auto_list[index] = -1;
		}
		else
		{
			auto_list[index] = (_s32)current;
		
			if (index == 0)
				sprintf(msg, "==> %s", s);
			else
			{
				int i;
				sprintf(msg, "    %s", s);
				for (i = 0; i < MAX_BREAKPOINTS; i++)
				{
					if (breakpoint[i].enabled && breakpoint[i].address == current)
					{
						sprintf(msg, "B:%d %s", i, s);
						break;
					}
				}
			}

			ListBox_AddString(h_auto, msg);
		}
	}

	free(s);
}

//=============================================================================

void code_update(void)
{
	if (system_debug_running())
	{
		char msg[100];
		int i;

		//Clear the history list
		ListBox_ResetContent(h_history);
		sprintf(msg, "  Cannot list history while the emulator is running. ");
		ListBox_AddString(h_history, msg);

		//Clear the auto-disassemble list
		ListBox_ResetContent(h_auto);
		sprintf(msg, "  Cannot auto disassemble while the emulator is running. ");
		ListBox_AddString(h_auto, msg);
		for (i = 0; i < AUTO_ITEMS; i++)
			auto_list[i] = -1;
	}
	else
	{
		_s32 oldpc, i;
		_u16 oldz80pc;

		//Build the history list
		ListBox_ResetContent(h_history);

		//Add instruction history
		oldpc = pc;	
		debug_mask_memory_error_messages = TRUE;
		for (i = 0; i < HISTORY_ITEMS; i++)
			auto_history(i);
		ListBox_SetCurSel(h_history, ListBox_GetCount(h_history) - 1);
		debug_mask_memory_error_messages = FALSE;
		pc = oldpc;
		
		//Build the auto-disassemble list
		ListBox_ResetContent(h_auto);

		//Add auto disassemble
		oldpc = pc;	
		oldz80pc = Z80_getReg(Z80_REG_PC);
		debug_mask_memory_error_messages = TRUE;
		for (i = 0; i < AUTO_ITEMS; i++)
			auto_add(oldpc, i);
		debug_mask_memory_error_messages = FALSE;
		Z80_setReg(Z80_REG_PC, oldz80pc);
		pc = oldpc;
	}
}

//=============================================================================

void code_open(void)
{
	g_CodeWnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_CODEVIEW), g_hWnd, CodeProc);
	if (g_CodeWnd == NULL)
		return;

	//Position the window
	SetWindowPos(g_CodeWnd, NULL, codeview_x, codeview_y, 0,0,
		SWP_NOSIZE | SWP_NOOWNERZORDER);

	h_auto = GetDlgItem(g_CodeWnd, IDC_AUTO);
	h_history = GetDlgItem(g_CodeWnd, IDC_HISTORY);

	//Radio Buttons
	h_TLCS900h = GetDlgItem(g_CodeWnd, IDC_CODE_TLCS900h);
	h_Z80 = GetDlgItem(g_CodeWnd, IDC_CODE_Z80);
	view_z80 = FALSE;
	Button_SetCheck(h_TLCS900h, TRUE);
	Button_SetCheck(h_Z80, FALSE);

	codeview_enabled = TRUE;
	ShowWindow(g_CodeWnd, SW_SHOWNORMAL);
	code_update();
}

void code_close(void)
{
	if (codeview_enabled)
	{
		codeview_enabled = FALSE;
		//Get Window position
		if (IsIconic(g_CodeWnd) == 0)
		{
			RECT r;
			GetWindowRect(g_CodeWnd, &r);
			codeview_x = r.left;
			codeview_y = r.top;
		}

		h_auto = NULL;
		h_history = NULL;
		DestroyWindow(g_CodeWnd);
		g_CodeWnd = NULL;
	}
}

//=============================================================================
#endif
