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

	debug_z80regview.c

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

#include "Z80_interface.h"
#include "mem.h"

#ifdef NEOPOP_DEBUG
//=============================================================================

static HWND g_RegWnd;
BOOL z80regview_enabled = FALSE;
static HWND h_z80_pc, h_z80_sp, h_z80_flags, 
			h_z80_ix, h_z80_iy,
			h_z80_af, h_z80_bc, h_z80_de, h_z80_hl,
			h_z80_af1, h_z80_bc1, h_z80_de1, h_z80_hl1,
			h_refresh, h_apply, h_s, h_z, h_h, h_v, h_n, h_c;

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

BOOL CALLBACK Z80RegProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	_u16 flags = Z80_getReg(Z80_REG_AF);

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			h_z80_pc = GetDlgItem(hDlg, IDC_Z80_PC);
			h_z80_sp = GetDlgItem(hDlg, IDC_Z80_SP);
			h_z80_flags = GetDlgItem(hDlg, IDC_Z80_FLAGS);
			h_z80_ix = GetDlgItem(hDlg, IDC_Z80_IX);
			h_z80_iy = GetDlgItem(hDlg, IDC_Z80_IY);

			h_z80_af = GetDlgItem(hDlg, IDC_Z80_AF);
			h_z80_bc = GetDlgItem(hDlg, IDC_Z80_BC);
			h_z80_de = GetDlgItem(hDlg, IDC_Z80_DE);
			h_z80_hl = GetDlgItem(hDlg, IDC_Z80_HL);

			//Shadow
			h_z80_af1 = GetDlgItem(hDlg, IDC_Z80_AF1);
			h_z80_bc1 = GetDlgItem(hDlg, IDC_Z80_BC1);
			h_z80_de1 = GetDlgItem(hDlg, IDC_Z80_DE1);
			h_z80_hl1 = GetDlgItem(hDlg, IDC_Z80_HL1);

			//Buttons
			h_refresh = GetDlgItem(hDlg, IDC_REFRESH);
			h_apply = GetDlgItem(hDlg, IDC_APPLY);
			
			//Flags
			h_s = GetDlgItem(hDlg, IDC_CHANGE_S);
			h_z = GetDlgItem(hDlg, IDC_CHANGE_Z);
			h_h = GetDlgItem(hDlg, IDC_CHANGE_H);
			h_v = GetDlgItem(hDlg, IDC_CHANGE_V);
			h_n = GetDlgItem(hDlg, IDC_CHANGE_N);
			h_c = GetDlgItem(hDlg, IDC_CHANGE_C);
		}
		return 0;

	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			z80reg_close();
			return 1;

		case IDC_CHANGE_S: flags ^= 0x80; Z80_setReg(Z80_REG_AF, flags);
			z80reg_update(); break;
		case IDC_CHANGE_Z: flags ^= 0x40; Z80_setReg(Z80_REG_AF, flags);
			z80reg_update(); break;
		case IDC_CHANGE_H: flags ^= 0x10; Z80_setReg(Z80_REG_AF, flags);
			z80reg_update(); break;
		case IDC_CHANGE_V: flags ^= 0x04; Z80_setReg(Z80_REG_AF, flags);
			z80reg_update(); break;
		case IDC_CHANGE_N: flags ^= 0x02; Z80_setReg(Z80_REG_AF, flags);
			z80reg_update(); break;
		case IDC_CHANGE_C: flags ^= 0x01; Z80_setReg(Z80_REG_AF, flags);
			z80reg_update(); break;
		
		case IDC_Z80_TOGGLE:
			if (Z80ACTIVE)
			{
				ram[0xB8] = 0xAA;
				ram[0xB9] = 0xAA;
			}
			else
			{
				ram[0xB8] = 0x55;
				ram[0xB9] = 0x55;
			}
			z80reg_update();
			break;

		case IDC_REFRESH: z80reg_update(); break;

		case IDC_APPLY:

		Z80_setReg(Z80_REG_PC, (_u16)getData(h_z80_pc, Z80_getReg(Z80_REG_PC), 3));
		Z80_setReg(Z80_REG_SP, (_u16)getData(h_z80_sp, Z80_getReg(Z80_REG_SP), 3));
		
		Z80_setReg(Z80_REG_IX, (_u16)getData(h_z80_ix, Z80_getReg(Z80_REG_IX), 4));
		Z80_setReg(Z80_REG_IY, (_u16)getData(h_z80_iy, Z80_getReg(Z80_REG_IY), 4));

		Z80_setReg(Z80_REG_AF, (_u16)getData(h_z80_af, Z80_getReg(Z80_REG_AF), 4));
		Z80_setReg(Z80_REG_BC, (_u16)getData(h_z80_bc, Z80_getReg(Z80_REG_BC), 4));
		Z80_setReg(Z80_REG_DE, (_u16)getData(h_z80_de, Z80_getReg(Z80_REG_DE), 4));
		Z80_setReg(Z80_REG_HL, (_u16)getData(h_z80_hl, Z80_getReg(Z80_REG_HL), 4));

		Z80_setReg(Z80_REG_AF1, (_u16)getData(h_z80_af1, Z80_getReg(Z80_REG_AF1), 4));
		Z80_setReg(Z80_REG_BC1, (_u16)getData(h_z80_bc1, Z80_getReg(Z80_REG_BC1), 4));
		Z80_setReg(Z80_REG_DE1, (_u16)getData(h_z80_de1, Z80_getReg(Z80_REG_DE1), 4));
		Z80_setReg(Z80_REG_HL1, (_u16)getData(h_z80_hl1, Z80_getReg(Z80_REG_HL1), 4));

			//So any changes to PC are reflected, etc.
			system_debug_refresh();	
			break;
		}
		break;
	}

	return 0;
}

//=============================================================================

void z80reg_update(void)
{
	if (z80regview_enabled)
	{
		char string[20] = {0};
		_u16 z80_flags;

		if (system_debug_running())
		{
			//Clear All
			Edit_SetText(h_z80_pc, string);
			Edit_SetText(h_z80_sp, string);
			Static_SetText(h_z80_flags, string);
		
			Edit_SetText(h_z80_af, string);
			Edit_SetText(h_z80_bc, string);
			Edit_SetText(h_z80_de, string);
			Edit_SetText(h_z80_hl, string);
			
			Edit_SetText(h_z80_af1, string);
			Edit_SetText(h_z80_bc1, string);
			Edit_SetText(h_z80_de1, string);
			Edit_SetText(h_z80_hl1, string);

			Edit_SetText(h_z80_ix, string);
			Edit_SetText(h_z80_iy, string);
		}
		else
		{
			char flags[9] = {0};

			//Update
			sprintf(string, "%03X", Z80_getReg(Z80_REG_PC));
			Edit_SetText(h_z80_pc, string);
			sprintf(string, "%03X", Z80_getReg(Z80_REG_SP));
			Edit_SetText(h_z80_sp, string);

			//Flags
			z80_flags = Z80_getReg(Z80_REG_AF) & 0xFF;
			if (z80_flags & 0x80)	flags[0] = 'S';	else flags[0] = 's';
			if (z80_flags & 0x40)	flags[1] = 'Z';	else flags[1] = 'z';
			if (z80_flags & 0x10)	flags[3] = 'H';	else flags[3] = 'h';
			if (z80_flags & 0x04)	flags[5] = 'V';	else flags[5] = 'v';
			if (z80_flags & 0x02)	flags[6] = 'N';	else flags[6] = 'n';
			if (z80_flags & 0x01)	flags[7] = 'C';	else flags[7] = 'c';
			flags[2] = flags[4] = ':'; flags[8] = 0;
			Static_SetText(h_z80_flags, flags);

			sprintf(string, "%04X", Z80_getReg(Z80_REG_AF));
			Edit_SetText(h_z80_af, string);

			sprintf(string, "%04X", Z80_getReg(Z80_REG_BC));
			Edit_SetText(h_z80_bc, string);

			sprintf(string, "%04X", Z80_getReg(Z80_REG_DE));
			Edit_SetText(h_z80_de, string);

			sprintf(string, "%04X", Z80_getReg(Z80_REG_HL));
			Edit_SetText(h_z80_hl, string);

			sprintf(string, "%04X", Z80_getReg(Z80_REG_AF1));
			Edit_SetText(h_z80_af1, string);

			sprintf(string, "%04X", Z80_getReg(Z80_REG_BC1));
			Edit_SetText(h_z80_bc1, string);

			sprintf(string, "%04X", Z80_getReg(Z80_REG_DE1));
			Edit_SetText(h_z80_de1, string);

			sprintf(string, "%04X", Z80_getReg(Z80_REG_HL1));
			Edit_SetText(h_z80_hl1, string);

			sprintf(string, "%04X", Z80_getReg(Z80_REG_IX));
			Edit_SetText(h_z80_ix, string);
	
			sprintf(string, "%04X", Z80_getReg(Z80_REG_IY));
			Edit_SetText(h_z80_iy, string);
		}

		if (Z80ACTIVE && system_debug_running() == FALSE)
		{
			// Enable All
			Edit_Enable(h_z80_pc, TRUE);
			Edit_Enable(h_z80_sp, TRUE);
		
			Edit_Enable(h_z80_af, TRUE);
			Edit_Enable(h_z80_bc, TRUE);
			Edit_Enable(h_z80_de, TRUE);
			Edit_Enable(h_z80_hl, TRUE);
			
			Edit_Enable(h_z80_af1, TRUE);
			Edit_Enable(h_z80_bc1, TRUE);
			Edit_Enable(h_z80_de1, TRUE);
			Edit_Enable(h_z80_hl1, TRUE);

			Edit_Enable(h_z80_ix, TRUE);
			Edit_Enable(h_z80_iy, TRUE);

			Button_Enable(h_s, TRUE);
			Button_Enable(h_z, TRUE);
			Button_Enable(h_h, TRUE);
			Button_Enable(h_v, TRUE);
			Button_Enable(h_n, TRUE);
			Button_Enable(h_c, TRUE);

			//Buttons
			Button_Enable(h_refresh, TRUE);
			Button_Enable(h_apply, TRUE);
		}
		else
		{
			// Disable All
			Edit_Enable(h_z80_pc, FALSE);
			Edit_Enable(h_z80_sp, FALSE);
		
			Edit_Enable(h_z80_af, FALSE);
			Edit_Enable(h_z80_bc, FALSE);
			Edit_Enable(h_z80_de, FALSE);
			Edit_Enable(h_z80_hl, FALSE);
			
			Edit_Enable(h_z80_af1, FALSE);
			Edit_Enable(h_z80_bc1, FALSE);
			Edit_Enable(h_z80_de1, FALSE);
			Edit_Enable(h_z80_hl1, FALSE);

			Edit_Enable(h_z80_ix, FALSE);
			Edit_Enable(h_z80_iy, FALSE);

			Button_Enable(h_s, FALSE);
			Button_Enable(h_z, FALSE);
			Button_Enable(h_h, FALSE);
			Button_Enable(h_v, FALSE);
			Button_Enable(h_n, FALSE);
			Button_Enable(h_c, FALSE);

			//Buttons
			Button_Enable(h_refresh, FALSE);
			Button_Enable(h_apply, FALSE);
		}
	}
}

//=============================================================================

void z80reg_open(void)
{
	g_RegWnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_Z80REGVIEW), g_hWnd, Z80RegProc);
	if (g_RegWnd == NULL)
		return;

	//Position the window
	SetWindowPos(g_RegWnd, NULL, z80regview_x, z80regview_y, 0,0,
		SWP_NOSIZE | SWP_NOOWNERZORDER);

	z80regview_enabled = TRUE;
	ShowWindow(g_RegWnd, SW_SHOWNORMAL);
	z80reg_update();
}

void z80reg_close(void)
{
	if (z80regview_enabled)
	{
		//Get Window position
		if (IsIconic(g_RegWnd) == 0)
		{
			RECT r;
			GetWindowRect(g_RegWnd, &r);
			z80regview_x = r.left;
			z80regview_y = r.top;
		}

		DestroyWindow(g_RegWnd);
		z80regview_enabled = FALSE;
		g_RegWnd = NULL;
	}
}

//=============================================================================
#endif