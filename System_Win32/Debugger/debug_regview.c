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

	debug_regview.c

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

#ifdef NEOPOP_DEBUG
//=============================================================================

static HWND g_RegWnd;
BOOL regview_enabled = FALSE;
static HWND h_flags, h_sr, h_pc, h_iff, h_rfp;
static HWND h_bank[4][4], h_gpr[4];
static HWND h_bank0[4], h_bank1[4], h_bank2[4], h_bank3[4];

//=============================================================================

static _u32 getData(HWND editbox, _u32 original, _u8 length)
{
	char text[10];
	char* dummy;
	_u32 data;
	Edit_GetText(editbox, text, length + 1);

	if (length == 6)
		data = abs(strtol(text, &dummy, 16)) & 0xFFFFFF;
	else
		data = ((_u32)strtoul(text, &dummy, 16)) & 0xFFFFFFFF;

	Edit_SetText(editbox, "");
	if (dummy == text)
		return original;
	return data;
}

BOOL CALLBACK RegProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			h_flags = GetDlgItem(hDlg, IDC_FLAGS);
			h_sr = GetDlgItem(hDlg, IDC_SR);
			h_pc = GetDlgItem(hDlg, IDC_PC);
			h_iff = GetDlgItem(hDlg, IDC_IFF);
			h_rfp = GetDlgItem(hDlg, IDC_RFP);

			//GPR
			h_gpr[0] = GetDlgItem(hDlg, IDC_GPR_XIX);
			h_gpr[1] = GetDlgItem(hDlg, IDC_GPR_XIY);
			h_gpr[2] = GetDlgItem(hDlg, IDC_GPR_XIZ);
			h_gpr[3] = GetDlgItem(hDlg, IDC_GPR_XSP);

			//Bank0
			h_bank0[0] = GetDlgItem(hDlg, IDC_GPR_0_XWA);
			h_bank0[1] = GetDlgItem(hDlg, IDC_GPR_0_XBC);
			h_bank0[2] = GetDlgItem(hDlg, IDC_GPR_0_XDE);
			h_bank0[3] = GetDlgItem(hDlg, IDC_GPR_0_XHL);

			//Bank1
			h_bank1[0] = GetDlgItem(hDlg, IDC_GPR_1_XWA);
			h_bank1[1] = GetDlgItem(hDlg, IDC_GPR_1_XBC);
			h_bank1[2] = GetDlgItem(hDlg, IDC_GPR_1_XDE);
			h_bank1[3] = GetDlgItem(hDlg, IDC_GPR_1_XHL);

			//Bank2
			h_bank2[0] = GetDlgItem(hDlg, IDC_GPR_2_XWA);
			h_bank2[1] = GetDlgItem(hDlg, IDC_GPR_2_XBC);
			h_bank2[2] = GetDlgItem(hDlg, IDC_GPR_2_XDE);
			h_bank2[3] = GetDlgItem(hDlg, IDC_GPR_2_XHL);

			//Bank3
			h_bank3[0] = GetDlgItem(hDlg, IDC_GPR_3_XWA);
			h_bank3[1] = GetDlgItem(hDlg, IDC_GPR_3_XBC);
			h_bank3[2] = GetDlgItem(hDlg, IDC_GPR_3_XDE);
			h_bank3[3] = GetDlgItem(hDlg, IDC_GPR_3_XHL);
		}
		return 0;

	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			reg_close();
			return 1;

		case IDC_CHANGE_S:	SETFLAG_S(!FLAG_S);	reg_update();	break;
		case IDC_CHANGE_Z:	SETFLAG_Z(!FLAG_Z);	reg_update();	break;
		case IDC_CHANGE_H:	SETFLAG_H(!FLAG_H);	reg_update();	break;
		case IDC_CHANGE_V:	SETFLAG_V(!FLAG_V);	reg_update();	break;
		case IDC_CHANGE_N:	SETFLAG_N(!FLAG_N);	reg_update();	break;
		case IDC_CHANGE_C:	SETFLAG_C(!FLAG_C);	reg_update();	break;

		case IDC_REFRESH: reg_update(); break;

		case IDC_APPLY:

			pc = getData(h_pc, pc, 6);

			//GPR
			gpr[0] = getData(h_gpr[0], gpr[0], 8);
			gpr[1] = getData(h_gpr[1], gpr[1], 8);
			gpr[2] = getData(h_gpr[2], gpr[2], 8);
			gpr[3] = getData(h_gpr[3], gpr[3], 8);

			//Bank0
			gprBank[0][0] = getData(h_bank0[0], gprBank[0][0], 8);
			gprBank[0][1] = getData(h_bank0[1], gprBank[0][1], 8);
			gprBank[0][2] = getData(h_bank0[2], gprBank[0][2], 8);
			gprBank[0][3] = getData(h_bank0[3], gprBank[0][3], 8);

			//Bank1
			gprBank[1][0] = getData(h_bank1[0], gprBank[1][0], 8);
			gprBank[1][1] = getData(h_bank1[1], gprBank[1][1], 8);
			gprBank[1][2] = getData(h_bank1[2], gprBank[1][2], 8);
			gprBank[1][3] = getData(h_bank1[3], gprBank[1][3], 8);

			//Bank2
			gprBank[2][0] = getData(h_bank2[0], gprBank[2][0], 8);
			gprBank[2][1] = getData(h_bank2[1], gprBank[2][1], 8);
			gprBank[2][2] = getData(h_bank2[2], gprBank[2][2], 8);
			gprBank[2][3] = getData(h_bank2[3], gprBank[2][3], 8);

			//Bank3
			gprBank[3][0] = getData(h_bank3[0], gprBank[3][0], 8);
			gprBank[3][1] = getData(h_bank3[1], gprBank[3][1], 8);
			gprBank[3][2] = getData(h_bank3[2], gprBank[3][2], 8);
			gprBank[3][3] = getData(h_bank3[3], gprBank[3][3], 8);

			//So any changes to PC are reflected, etc.
			system_debug_refresh();	
			break;

		}
		break;
	}

	return 0;
}

//=============================================================================

void reg_update(void)
{
	if (regview_enabled)
	{
		char flags[9], string[20] = {0};

		//Program Counter
		if (system_debug_running() == FALSE) sprintf(string, "%06X", pc);
		Edit_SetText(h_pc, string);

		//Status Register
		if (system_debug_running() == FALSE) sprintf(string, "%04X", sr);
		Static_SetText(h_sr, string);

		//Flags
		if (sr & 0x80)	flags[0] = 'S';	else flags[0] = 's';
		if (sr & 0x40)	flags[1] = 'Z';	else flags[1] = 'z';
		if (sr & 0x10)	flags[3] = 'H';	else flags[3] = 'h';
		if (sr & 0x04)	flags[5] = 'V';	else flags[5] = 'v';
		if (sr & 0x02)	flags[6] = 'N';	else flags[6] = 'n';
		if (sr & 0x01)	flags[7] = 'C';	else flags[7] = 'c';
		
		flags[2] = flags[4] = ':'; flags[8] = 0;
		Static_SetText(h_flags, flags);
		
		if (system_debug_running() == TRUE) 
			Static_SetText(h_flags, string);
						
		//IFF
		if (system_debug_running() == FALSE)
		{ 
			if (statusIFF() == 0)
				sprintf(string, "0/1");
			else
				sprintf(string, "%01X", statusIFF());
		}
		Static_SetText(h_iff, string);

		//RFP
		if (system_debug_running() == FALSE) sprintf(string, "%01X", ((sr & 0x300) >> 8));
		Static_SetText(h_rfp, string);

		//GPR
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gpr[0]);Edit_SetText(h_gpr[0], string);
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gpr[1]);Edit_SetText(h_gpr[1], string);
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gpr[2]);Edit_SetText(h_gpr[2], string);
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gpr[3]);Edit_SetText(h_gpr[3], string);

		//Bank0
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[0][0]);Edit_SetText(h_bank0[0], string);
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[0][1]);Edit_SetText(h_bank0[1], string);
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[0][2]);Edit_SetText(h_bank0[2], string);
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[0][3]);Edit_SetText(h_bank0[3], string);

		//Bank1
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[1][0]);Edit_SetText(h_bank1[0], string);
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[1][1]);Edit_SetText(h_bank1[1], string);
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[1][2]);Edit_SetText(h_bank1[2], string);
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[1][3]);Edit_SetText(h_bank1[3], string);

		//Bank2
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[2][0]);Edit_SetText(h_bank2[0], string);
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[2][1]);Edit_SetText(h_bank2[1], string);
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[2][2]);Edit_SetText(h_bank2[2], string);
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[2][3]);Edit_SetText(h_bank2[3], string);

		//Bank3
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[3][0]);Edit_SetText(h_bank3[0], string);
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[3][1]);Edit_SetText(h_bank3[1], string);
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[3][2]);Edit_SetText(h_bank3[2], string);
		if (system_debug_running() == FALSE) sprintf(string, "%08X", gprBank[3][3]);Edit_SetText(h_bank3[3], string);
	}
}

//=============================================================================

void reg_open(void)
{
	g_RegWnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_REGVIEW), g_hWnd, RegProc);
	if (g_RegWnd == NULL)
		return;

	//Position the window
	SetWindowPos(g_RegWnd, NULL, regview_x, regview_y, 0,0,
		SWP_NOSIZE | SWP_NOOWNERZORDER);

	regview_enabled = TRUE;
	ShowWindow(g_RegWnd, SW_SHOWNORMAL);
	reg_update();
}

void reg_close(void)
{
	if (regview_enabled)
	{
		//Get Window position
		if (IsIconic(g_RegWnd) == 0)
		{
			RECT r;
			GetWindowRect(g_RegWnd, &r);
			regview_x = r.left;
			regview_y = r.top;
		}

		DestroyWindow(g_RegWnd);
		regview_enabled = FALSE;
		g_RegWnd = NULL;
	}
}

//=============================================================================
#endif