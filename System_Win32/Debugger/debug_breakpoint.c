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

	debug_breakpoint.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

04 SEP 2002 - neopop_uk
=======================================
- Created this file to deal with increasingly more complex breakpoints.

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
#include "Z80_interface.h"

#ifdef NEOPOP_DEBUG
//=============================================================================

static HWND g_BreakWnd; //Haw Haw Haw, sorry.
BOOL breakview_enabled = FALSE;

BREAKPOINT breakpoint[MAX_BREAKPOINTS], breakpoint_z80;

//=============================================================================

BOOL breakpoint_reached(void)
{
	int i;

	if (breakpoint[0].enabled && pc == breakpoint[0].address)
	{
		system_debug_message("Run to address reached.");
		breakpoint[0].address = -1;
		breakpoint[0].enabled = FALSE;
		system_debug_stop();
		system_debug_refresh();
		return TRUE;
	}

	for (i = 1; i < MAX_BREAKPOINTS; i++)
	{
		if (breakpoint[i].enabled && pc == breakpoint[i].address)
		{
			system_debug_message("TLCS-900h Breakpoint %d reached.", i);
			system_debug_stop();
			system_debug_refresh();
			return TRUE;
		}
	}

	if (Z80ACTIVE)
	{
		if (breakpoint_z80.enabled && 
			Z80_getReg(Z80_REG_PC) == breakpoint_z80.address)
		{
			system_debug_message("Z80 Breakpoint reached.");
			system_debug_stop();
			system_debug_refresh();
			return TRUE;
		}
	}

	return FALSE;
}


void set_breakpoint_runto(void)
{
	_s32 start = get_debug_address();
	if (start != -1)
	{
		breakpoint[0].address = (_u32)(start & 0xFFFFFF);
		breakpoint[0].enabled = TRUE;
		system_debug_start(FALSE, FALSE);
		system_debug_refresh();
		SetFocus(g_hWnd);
	}
}

//=============================================================================

BOOL CALLBACK BreakProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		return FALSE;

	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			break_close();
			return 1;

		case IDC_BREAKSET1:
			{
				_s32 start = get_debug_address();
				if (start != -1)
				{
					breakpoint[1].address = (_u32)(start & 0xFFFFFF);
					breakpoint[1].enabled = TRUE;
					system_debug_refresh();
				}
			}
			break;
			
		case IDC_BREAKSET2:
			{
				_s32 start = get_debug_address();
				if (start != -1)
				{
					breakpoint[2].address = (_u32)(start & 0xFFFFFF);
					breakpoint[2].enabled = TRUE;
					system_debug_refresh();
				}
			}
			break;
			
		case IDC_BREAKSET3:
			{
				_s32 start = get_debug_address();
				if (start != -1)
				{
					breakpoint[3].address = (_u32)(start & 0xFFFFFF);
					breakpoint[3].enabled = TRUE;
					system_debug_refresh();
				}
			}
			break;
			
		case IDC_BREAKCLEAR1:
			breakpoint[1].enabled = FALSE;
			system_debug_refresh();
			break;

		case IDC_BREAKCLEAR2:
			breakpoint[2].enabled = FALSE;
			system_debug_refresh();
			break;

		case IDC_BREAKCLEAR3:
			breakpoint[3].enabled = FALSE;
			system_debug_refresh();
			break;

		case IDC_BREAKCLEAR0:
			breakpoint[0].enabled = FALSE;
			breakpoint[0].address = -1;
			system_debug_refresh();
			break;

		case IDC_BREAKPOINT_Z80:
			{
				_s32 start = get_debug_address();
				if (start != -1)
				{
					if (start >= 0x7000 && start <= 0x7FFF)
						start &= 0xFFF;

					if (start >= 0 && start <= 0xFFF)
					{
						breakpoint_z80.address = start;
						breakpoint_z80.enabled = TRUE;
					}
					system_debug_refresh();
				}
			}
			break;
			
		case IDC_BREAKCLEAR_Z80:
			breakpoint_z80.address = -1;
			breakpoint_z80.enabled = FALSE;
			system_debug_refresh();
			break;
		}

		break;
	}

	return 0;
}

//=============================================================================

void break_update(void)
{
	//TLCS-900h Breakpoint 1
	if (breakpoint[1].enabled == FALSE)
		Edit_SetText(GetDlgItem(g_BreakWnd,IDC_BREAKSTAT1), "Not Set");
	else
	{
		char msg[100];
		sprintf(msg, "%06X", breakpoint[1].address);
		Edit_SetText(GetDlgItem(g_BreakWnd,IDC_BREAKSTAT1), msg);
	}

	//TLCS-900h Breakpoint 2
	if (breakpoint[2].enabled == FALSE)
		Edit_SetText(GetDlgItem(g_BreakWnd,IDC_BREAKSTAT2), "Not Set");
	else
	{
		char msg[100];
		sprintf(msg, "%06X", breakpoint[2].address);
		Edit_SetText(GetDlgItem(g_BreakWnd,IDC_BREAKSTAT2), msg);
	}

	//TLCS-900h Breakpoint 3
	if (breakpoint[3].enabled == FALSE)
		Edit_SetText(GetDlgItem(g_BreakWnd,IDC_BREAKSTAT3), "Not Set");
	else
	{
		char msg[100];
		sprintf(msg, "%06X", breakpoint[3].address);
		Edit_SetText(GetDlgItem(g_BreakWnd,IDC_BREAKSTAT3), msg);
	}

	//Run to... breakpoint
	if (breakpoint[0].enabled == FALSE)
		Edit_SetText(GetDlgItem(g_BreakWnd,IDC_BREAKSTAT0), "Not Set");
	else
	{
		char msg[100];
		sprintf(msg, "%06X", breakpoint[0].address);
		Edit_SetText(GetDlgItem(g_BreakWnd,IDC_BREAKSTAT0), msg);
	}

	//Z80 Breakpoint
	if (breakpoint_z80.enabled == FALSE)
		Edit_SetText(GetDlgItem(g_BreakWnd,IDC_BREAKSTAT_Z80), "Not Set");
	else
	{
		char msg[100];
		sprintf(msg, "%03X", breakpoint_z80.address);
		Edit_SetText(GetDlgItem(g_BreakWnd,IDC_BREAKSTAT_Z80), msg);
	}
}

//=============================================================================

void break_open(void)
{
	g_BreakWnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_BREAKPOINTS), g_hWnd, BreakProc);
	if (g_BreakWnd == NULL)
		return;

	//Position the window
	SetWindowPos(g_BreakWnd, NULL, breakview_x, breakview_y, 0,0,
		SWP_NOSIZE | SWP_NOOWNERZORDER);

	breakview_enabled = TRUE;
	ShowWindow(g_BreakWnd, SW_SHOWNORMAL);
	break_update();
}

void break_close(void)
{
	if (breakview_enabled)
	{
		breakview_enabled = FALSE;
		//Get Window position
		if (IsIconic(g_BreakWnd) == 0)
		{
			RECT r;
			GetWindowRect(g_BreakWnd, &r);
			breakview_x = r.left;
			breakview_y = r.top;
		}

		DestroyWindow(g_BreakWnd);
		g_BreakWnd = NULL;
	}
}

//=============================================================================
#endif
