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

	system_input_dialog.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

21 JUL 2002 - neopop_uk
=======================================
- Renamed this file from 'inputdlg.c' to 'system_input_dialog.c'

25 JUL 2002 - neopop_uk
=======================================
- Renamed the callback function from "InputDlg" to "InputProc"

05 AUG 2002 - neopop_uk
=======================================
- Added the keys from the numpad.

11 AUG 2002 - neopop_uk
=======================================
- Added support for using other joysticks.

18 AUG 2002 - neopop_uk
=======================================
- Added language support.

28 AUG 2002 - neopop_uk
=======================================
- Renamed dialog handle variable names.
 
09 SEP 2002 - neopop_uk
=======================================
- Added adaptoid support.

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include <windows.h>
#include <windowsx.h>
#include "resource.h"

//For input keycodes.
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include "system_input.h"
#include "system_config.h"
#include "system_language.h"

//=============================================================================

typedef struct 
{
	int dik;
	char name[16];
}
Skey;

static Skey keymap[] =
{
	DIK_UP,			"Up",
	DIK_DOWN,		"Down",
	DIK_LEFT,		"Left",
	DIK_RIGHT,		"Right",

	DIK_SPACE,		"Space",
	DIK_ESCAPE,		"Escape",
	DIK_TAB,		"Tab",
	DIK_RETURN,		"Return",
	DIK_BACKSPACE,	"Backspace",
	DIK_LCONTROL,	"Ctrl (L)",
	DIK_RCONTROL,	"Ctrl (R)",
	DIK_LSHIFT,		"Shift (L)",
	DIK_RSHIFT,		"Shift (R)",
	DIK_LALT,		"Alt (L)",
	DIK_RALT,		"Alt (R)",

	//

	DIK_A, "A",	DIK_B, "B",	DIK_C, "C",	DIK_D, "D",	DIK_E, "E",	DIK_F, "F",
	DIK_G, "G",	DIK_H, "H",	DIK_I, "I",	DIK_J, "J",	DIK_K, "K",	DIK_L, "L",
	DIK_M, "M",	DIK_N, "N",	DIK_O, "O",	DIK_P, "P",	DIK_Q, "Q",	DIK_R, "R",
	DIK_S, "S",	DIK_T, "T",	DIK_U, "U",	DIK_V, "V",	DIK_W, "W",	DIK_X, "X",
	DIK_Y, "Y",	DIK_Z, "Z",

	//

	DIK_NUMPADENTER,	"Enter (Pad)",	

	DIK_NUMPAD0,		"0 (NumPad)",
	DIK_NUMPAD1,		"1 (NumPad)",
	DIK_NUMPAD2,		"2 (NumPad)",
	DIK_NUMPAD3,		"3 (NumPad)",
	DIK_NUMPAD4,		"4 (NumPad)",
	DIK_NUMPAD5,		"5 (NumPad)",
	DIK_NUMPAD6,		"6 (NumPad)",
	DIK_NUMPAD7,		"7 (NumPad)",
	DIK_NUMPAD8,		"8 (NumPad)",
	DIK_NUMPAD9,		"9 (NumPad)",

	DIK_ADD,			"+ (NumPad)",
	DIK_SUBTRACT,		"- (NumPad)",
	DIK_MULTIPLY,		"* (NumPad)",
	DIK_DIVIDE,			"/ (NumPad)",
	DIK_DECIMAL,		". (NumPad)",

	//

	-1, "error"
};

//=============================================================================

static HWND h_controlU, h_controlD;
static HWND h_controlL, h_controlR;
static HWND h_controlA, h_controlKA, h_controlAA;
static HWND h_controlB, h_controlKB, h_controlAB;
static HWND h_controlO, h_controlKO;

static void update(HWND hDlg)
{
	int i, stickcount;
	DWORD item;

	h_controlAA = GetDlgItem( hDlg, IDC_CHECKA );
	h_controlAB = GetDlgItem( hDlg, IDC_CHECKB );

	h_controlU = GetDlgItem( hDlg, IDC_COMBOU );
	h_controlD = GetDlgItem( hDlg, IDC_COMBOD );
	h_controlL = GetDlgItem( hDlg, IDC_COMBOL );
	h_controlR = GetDlgItem( hDlg, IDC_COMBOR );

	h_controlKA = GetDlgItem( hDlg, IDC_COMBOKA );
	h_controlKB = GetDlgItem( hDlg, IDC_COMBOKB );
	h_controlKO = GetDlgItem( hDlg, IDC_COMBOKO );
	h_controlA = GetDlgItem( hDlg, IDC_COMBOA );
	h_controlB = GetDlgItem( hDlg, IDC_COMBOB );
	h_controlO = GetDlgItem( hDlg, IDC_COMBOO );

	//Autofire
	Button_SetCheck(h_controlAA, AUTOA);
	Button_SetCheck(h_controlAB, AUTOB);

	//Joysticks
	ComboBox_ResetContent(GetDlgItem(hDlg, IDC_COMBOJOY));
	stickcount = system_input_stickcount();
	if (stickcount > 0)
	{
		//Clamp stick number
		if (STICK >= stickcount)
			STICK = 0;

		//Add the button choices
		for (i = 0; i < stickcount; i++)
		{
			char* b = system_input_stickname(i);
			char string[256];
			wsprintf( string, "%d: \"%s\"", i+1, b);

			//Select button A
			item = ComboBox_AddString(GetDlgItem(hDlg, IDC_COMBOJOY), string);
			if (i == STICK) ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_COMBOJOY), item);

			free(b);
		}

		//Joystick?
		ComboBox_ResetContent(GetDlgItem(hDlg, IDC_COMBOA));
		ComboBox_ResetContent(GetDlgItem(hDlg, IDC_COMBOB));
		ComboBox_ResetContent(GetDlgItem(hDlg, IDC_COMBOO));
		if (system_input_buttoncount() > 0)
		{
			//Add the button choices
			for (i = 0; i < system_input_buttoncount(); i++)
			{
				char* b = system_input_buttonname(i);
				char string[256];
				sprintf( string, "%d: \"%s\"", i+1, b);

				//Select button A
				item = ComboBox_AddString(h_controlA, string);
				if (i == BUTTONA) ComboBox_SetCurSel(h_controlA, item);

				//Select button B
				item = ComboBox_AddString(h_controlB, string);
				if (i == BUTTONB) ComboBox_SetCurSel(h_controlB, item);

				//Select button Option
				item = ComboBox_AddString(h_controlO, string);
				if (i == BUTTONO) ComboBox_SetCurSel(h_controlO, item);			

				free(b);
			}
		}
	}
	else
	{
		//Disable
		ComboBox_Enable(GetDlgItem(hDlg, IDC_COMBOJOY), FALSE);
		ComboBox_Enable(h_controlA, FALSE);
		ComboBox_Enable(h_controlB, FALSE);
		ComboBox_Enable(h_controlO, FALSE);
	}

	i = 0;

	//Keyboard, Add the choices
	while(keymap[i].dik != -1)
	{
		char string[16];
		sprintf( string, "%s", keymap[i].name);
		
		//Select key UP
		item = ComboBox_AddString(h_controlU, string);
		if (keymap[i].dik == KEYU)	ComboBox_SetCurSel(h_controlU, item);
		//Select key DOWN
		item = ComboBox_AddString(h_controlD, string);
		if (keymap[i].dik == KEYD)	ComboBox_SetCurSel(h_controlD, item);
		//Select key LEFT
		item = ComboBox_AddString(h_controlL, string);
		if (keymap[i].dik == KEYL)	ComboBox_SetCurSel(h_controlL, item);
		//Select key RIGHT
		item = ComboBox_AddString(h_controlR, string);
		if (keymap[i].dik == KEYR)	ComboBox_SetCurSel(h_controlR, item);
		
		//Select key A
		item = ComboBox_AddString(h_controlKA, string);
		if (keymap[i].dik == KEYA) ComboBox_SetCurSel(h_controlKA, item);

		//Select key B
		item = ComboBox_AddString(h_controlKB, string);
		if (keymap[i].dik == KEYB) ComboBox_SetCurSel(h_controlKB, item);

		//Select key Option
		item = ComboBox_AddString(h_controlKO, string);
		if (keymap[i].dik == KEYO) ComboBox_SetCurSel(h_controlKO, item);
		
		i ++;
	}
}

//=============================================================================

BOOL CALLBACK InputProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		system_input_shutdown();	//Restart the input code.
		system_input_init();
		update(hDlg);
		system_language_patch_dialog(IDD_CONTROLS, hDlg);
		Button_SetCheck(GetDlgItem(hDlg,IDC_ADAPTOID), adaptoid);
		return 1;

	case WM_COMMAND:
		{
			DWORD selA = ComboBox_GetCurSel(h_controlA);
			DWORD selB = ComboBox_GetCurSel(h_controlB);
			DWORD selO = ComboBox_GetCurSel(h_controlO);

			DWORD selKA = ComboBox_GetCurSel(h_controlKA);
			DWORD selKB = ComboBox_GetCurSel(h_controlKB);
			DWORD selKO = ComboBox_GetCurSel(h_controlKO);
		
			DWORD selU = ComboBox_GetCurSel(h_controlU);
			DWORD selD = ComboBox_GetCurSel(h_controlD);
			DWORD selL = ComboBox_GetCurSel(h_controlL);
			DWORD selR = ComboBox_GetCurSel(h_controlR);

			switch(LOWORD(wParam))
			{
			case IDOK:
			case IDCANCEL:
				EndDialog(hDlg, 0);
				return 1;

			case IDDEFAULT:
				{
					if (MessageBox(hDlg, system_get_string(IDS_DEFAULT),
						PROGRAM_NAME, MB_YESNO | MB_ICONQUESTION) == IDYES)
					{					
						KEYU = DIK_UP;
						KEYD = DIK_DOWN;
						KEYL = DIK_LEFT;
						KEYR = DIK_RIGHT;
						KEYA = DIK_Z;
						KEYB = DIK_X;
						KEYO = DIK_TAB;
						BUTTONA = 0;
						BUTTONB = 1;
						BUTTONO = 2;
					}
				}
			
				update(hDlg);
				break;

			case IDC_ADAPTOID:	adaptoid = ! adaptoid;	return 1;

			case IDC_CHECKA:	AUTOA = ! AUTOA;	return 1;
			case IDC_CHECKB:	AUTOB = ! AUTOB;	return 1;
			}

			if (HIWORD(wParam) == CBN_SELENDOK)
			{
				switch (LOWORD(wParam))
				{
				//Joy buttons
				case IDC_COMBOA:	BUTTONA = selA; break;
				case IDC_COMBOB:	BUTTONB = selB; break;
				case IDC_COMBOO:	BUTTONO = selO; break;

				//Key buttons
				case IDC_COMBOKA:	KEYA = keymap[selKA].dik; break;
				case IDC_COMBOKB:	KEYB = keymap[selKB].dik; break;
				case IDC_COMBOKO:	KEYO = keymap[selKO].dik; break;

				//Key D-Pad
				case IDC_COMBOU:	KEYU = keymap[selU].dik; break;
				case IDC_COMBOD:	KEYD = keymap[selD].dik; break;
				case IDC_COMBOL:	KEYL = keymap[selL].dik; break;
				case IDC_COMBOR:	KEYR = keymap[selR].dik; break;

				case IDC_COMBOJOY:
					{
						int sel = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_COMBOJOY));
						
						if (sel != STICK)
						{						
							STICK = sel;
							EndDialog(hDlg, 0);
							system_input_shutdown();
							system_input_init();
						}
					}

					break;
				}

				return 1;
			}
		}

		break;
	}

	return 0;
}

//=============================================================================
