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

	system_debug.c

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

04 SEP 2002 - neopop_uk
=======================================
- Added icons to some buttons.
- Moved the breakpoint code to "debug_breakpoint.c"

05 SEP 2002 - neopop_uk
=======================================
- Added breakpoint and watch windows.

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include <windows.h>
#include <windowsx.h>
#include "..\resource.h"

#include "system_debug.h"
#include "..\system_main.h"
#include "..\system_config.h"
#include "..\system_sound.h"

#include "TLCS900h_registers.h"
#include "TLCS900h_interpret.h"
#include "Z80_interface.h"
#include "interrupt.h"
#include "mem.h"
#include "gfx.h"

#ifdef NEOPOP_DEBUG
//=============================================================================

HWND g_DebugWnd;

BOOL running_state;

BOOL running_dis_TLCS900h = FALSE;
BOOL running_dis_Z80 = FALSE;

DWORD messagecount = 0;

static HWND h_list = NULL;			//Message List
static HWND status = NULL;			//Running status
static HWND address_edit = NULL;	//Edit box

//List of PC values in the the main window
_s32 main_list[65536];

static HWND hwnd_filter_mem;
static HWND hwnd_filter_bios;
static HWND hwnd_filter_comms;
static HWND hwnd_filter_sound;
static HWND hwnd_filter_dma;

//=============================================================================

_s32 get_debug_address(void)
{
	char text[10];
	char* dummy;
	_s32 address;
	Edit_GetText(address_edit, text, 9);
	address = abs(strtol(text, &dummy, 16)) & 0xFFFFFF;
	Edit_SetText(address_edit, "");
	if (dummy == text)
		return -1;
	return (_s32)address;
}

void set_debug_address(_s32 address)
{
	if (address == -1)
	{
		Edit_SetText(address_edit, "");
	}
	else
	{
		char msg[10];
		sprintf(msg, "%06X", address);
		Edit_SetText(address_edit, msg);
	}
}

//=============================================================================

BOOL CALLBACK DebugProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		return FALSE;

	case WM_COMMAND:

		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			return 1;

		case IDC_LIST:
			{
				int index = ListBox_GetCurSel(h_list);
				if (index >= 0)	set_debug_address(main_list[index]);
			}
			break;

		case IDC_GO:
			if (system_debug_running() == FALSE)
			{
				//Don't check for breakpoint on first instruction,
				//this allows resuming from breakpoint without stopping
				//immediately
				system_debug_start(FALSE, FALSE);
				emulate_debug(FALSE, FALSE);
				system_debug_refresh();
				SetFocus(g_hWnd);
			}
			break;

		case IDC_STOP:
			if (system_debug_running())
			{
				system_debug_stop();
				system_debug_refresh();
			}
			break;

			//==========================

		case IDC_STEP:
			{
				//Interpret
				emulate_debug(TRUE, FALSE);
				system_debug_stop();

				//Update views
				system_debug_refresh();
			}
			break;

		case IDC_GO_DIS:
			if (system_debug_running() == FALSE)
			{
				//Don't check for breakpoint on first instruction,
				//this allows resuming from breakpoint without stopping
				//immediately
				system_debug_start(TRUE, FALSE);
				emulate_debug(TRUE, FALSE);
				system_debug_refresh();
				SetFocus(g_hWnd);
			}
			break;

		case IDC_STEP_BOTH:
			{
				//Interpret
				emulate_debug(TRUE, TRUE);
				system_debug_stop();

				//Update views
				system_debug_refresh();
			}
			break;

		case IDC_STEP_Z80:
			{
				if (Z80ACTIVE)
				{
					char* s;
					_u16 pc = Z80_getReg(Z80_REG_PC);
					_u16 store_pc = pc;

					//Disassemble
					s = Z80_disassemble(&pc);
					system_debug_message("STEP %s", s);
					system_debug_message_associate_address(store_pc + 0x7000);
					free(s);

					//Interpret Z80 only
					Z80EMULATE;
					system_debug_stop();

					//Update views
					system_debug_refresh();
				}
				else
				{
					system_debug_message("z80: Not Active, Command Ignored");
				}
			}
			break;

		case IDC_RUNTOADDRESS:
			set_breakpoint_runto(); 
			break;

		case IDC_JUMPTOADDRESS:
			{
				_s32 start = get_debug_address();
				if (start != -1)
				{
					pc = (_u32)(start & 0xFFFFFF);
					system_debug_refresh();
				}
			}
			break;

			//==========================

		case IDC_DISASSEMBLE:
			{
				_s32 start = get_debug_address();

				if (start < 0)
					system_debug_message("No address specified.");
				else
				{
					_u32 i;
					_u32 oldpc = pc;
					debug_abort_memory = FALSE;

					pc = start;

					debug_mask_memory_error_messages = TRUE;
					
					system_debug_message(" ");

					//Print some instructions
					for (i = 0; i < 16; i++)
					{
						_s32 store_pc = pc;
						char* s = disassemble();

						if (debug_abort_memory)
						{
							pc = oldpc;
							free(s);
							break;
						}

						system_debug_message("DIS  %s", s);
						system_debug_message_associate_address(store_pc);
						free(s);
					}

					debug_mask_memory_error_messages = FALSE;
					pc = oldpc;
				}
			}
			break;

			//==========================

		case IDC_CODEVIEW:
			if (codeview_enabled) code_close(); else code_open(); break;

		case IDC_MEMVIEW:
			if (memview_enabled) mem_close(); else mem_open(); break;

		case IDC_REGVIEW:
			if (regview_enabled) reg_close(); else reg_open(); break;

		case IDC_Z80REGVIEW:
			if (z80regview_enabled) z80reg_close(); else z80reg_open(); break;

		case IDC_BREAKVIEW:	
			if (breakview_enabled) break_close(); else break_open(); break;

		case IDC_WATCHVIEW:	
			if (watchview_enabled) watch_close(); else watch_open(); break;

		case IDC_CLEAR:
			SendDlgItemMessage(g_DebugWnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
			messagecount = 0;
			break;

		case IDC_CHECK_MEM:		filter_mem = !filter_mem;		break;
		case IDC_CHECK_BIOS:	filter_bios = !filter_bios;		break;
		case IDC_CHECK_COMMS:	filter_comms = !filter_comms;	break;
		case IDC_CHECK_SOUND:	filter_sound = !filter_sound;	break;
		case IDC_CHECK_DMA:		filter_dma = !filter_dma;		break;
	
		case IDC_SAVE:
			{
				FILE* fp = fopen("c:\\neopop_debug_log.txt", "w");

				if (fp != NULL)
				{
					_u32 i = 0;
					_u8 buffer[2560];
					for (i = 0; i < messagecount; i++)
					{
						ListBox_GetText(h_list, i, &buffer);
						fprintf(fp, "%s\n", buffer);
					}

					fclose(fp);

					system_message("Saved debugger messages to \"c:\\neopop_debug_log.txt\"");
				}
				else
					system_debug_message("Error creating \"c:\\neopop_debug_log.txt\"");
			}
		}
		break;
	}

	return 0;
}

//=============================================================================

void system_debug_init(void)
{
	int i;

	g_DebugWnd = CreateDialog(g_hInstance, MAKEINTRESOURCE(IDD_DEBUG), g_hWnd, DebugProc);
	if (g_DebugWnd == NULL)
		return;

	//Position the window
	SetWindowPos(g_DebugWnd, NULL, debugview_x, debugview_y, 0,0,
		SWP_NOSIZE | SWP_NOOWNERZORDER);

	ShowWindow(g_DebugWnd, SW_SHOWNORMAL);

	status = GetDlgItem(g_DebugWnd, IDC_STATUS);
	address_edit = GetDlgItem(g_DebugWnd, IDC_EDIT);
	h_list = GetDlgItem(g_DebugWnd, IDC_LIST);
	
	// Add icons to buttons
	PostMessage(GetDlgItem(g_DebugWnd,IDC_GO), BM_SETIMAGE, IMAGE_ICON, 
		(long)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_GO), IMAGE_ICON, 16,16, 0));
	PostMessage(GetDlgItem(g_DebugWnd,IDC_STOP), BM_SETIMAGE, IMAGE_ICON, 
		(long)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_STOP), IMAGE_ICON, 16,16, 0));
	
	//Clear the messages
	for (i = 0; i < 65536; i++)
		main_list[i] = -1;

	//Message Filters
	hwnd_filter_sound =		GetDlgItem(g_DebugWnd, IDC_CHECK_SOUND);
	hwnd_filter_dma =		GetDlgItem(g_DebugWnd, IDC_CHECK_DMA);
	hwnd_filter_bios =		GetDlgItem(g_DebugWnd, IDC_CHECK_BIOS);
	hwnd_filter_comms =		GetDlgItem(g_DebugWnd, IDC_CHECK_COMMS);
	hwnd_filter_mem =		GetDlgItem(g_DebugWnd, IDC_CHECK_MEM);

	//Settings
	Button_SetCheck(hwnd_filter_mem,	filter_mem ? BST_CHECKED : BST_UNCHECKED);
	Button_SetCheck(hwnd_filter_sound,	filter_sound ? BST_CHECKED : BST_UNCHECKED);
	Button_SetCheck(hwnd_filter_dma,	filter_dma ? BST_CHECKED : BST_UNCHECKED);
	Button_SetCheck(hwnd_filter_bios,	filter_bios ? BST_CHECKED : BST_UNCHECKED);
	Button_SetCheck(hwnd_filter_comms,	filter_comms ? BST_CHECKED : BST_UNCHECKED);

	system_debug_refresh();

	SendDlgItemMessage(g_DebugWnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
	messagecount = 0;
}

void system_debug_shutdown(void)
{
	break_close();
	watch_close();
	reg_close();
	z80reg_close();
	mem_close();
	code_close();

	//Get Window position
	if (IsIconic(g_DebugWnd) == 0)
	{
		RECT r;
		GetWindowRect(g_DebugWnd, &r);
		debugview_x = r.left;
		debugview_y = r.top;
	}

	DestroyWindow(g_DebugWnd);
	status = NULL;
	address_edit = NULL;
	h_list = NULL;
}

//=============================================================================

void system_debug_clear(void)
{
	int i;

	SendDlgItemMessage(g_DebugWnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
	messagecount = 0;

	for (i = 0; i < 65536; i++)
		main_list[i] = -1;
}

void system_debug_start(BOOL dis_TLCS900h, BOOL dis_Z80)
{
	running_dis_TLCS900h = dis_TLCS900h;
	running_dis_Z80 = dis_Z80;
	running_state = TRUE;
	system_debug_refresh();
	system_sound_silence();
}

BOOL system_debug_running(void) { return running_state; }

void system_debug_stop(void) 
{ 
	system_sound_silence(); 
	running_state = FALSE; 
	system_debug_refresh();
}

//=============================================================================

void system_debug_message_associate_address(_u32 address)
{
	if (messagecount > 0)
		main_list[messagecount - 1] = address;
}

void __cdecl system_debug_message(char* vaMessage,...)
{
	char message[1001], print[1001];
    va_list vl;
	DWORD i, start;

    va_start(vl, vaMessage);
    vsprintf(message, vaMessage, vl);
	va_end(vl);

	start = 0;

	for (i = 0; i < strlen(message); i++)
	{
		if (message[i] == '\n')
		{
			memset(print, 0, 1000);
			memcpy(print, &(message[start]), i - start);
			start = i+1;

			SendDlgItemMessage(g_DebugWnd, IDC_LIST, LB_ADDSTRING, 0, (long)(&print));
			SendDlgItemMessage(g_DebugWnd, IDC_LIST, LB_SETCURSEL, messagecount, 0);
			messagecount++;
		}

		if (message[i] < 32)
			message[i] = '?';
	}

	//Last bit
	if (start <= strlen(message))
	{
		memset(print, 0, 1000);
		memcpy(print, &(message[start]), i - start);
		start = i+1;

		SendDlgItemMessage(g_DebugWnd, IDC_LIST, LB_ADDSTRING, 0, (long)(&print));
		SendDlgItemMessage(g_DebugWnd, IDC_LIST, LB_SETCURSEL, messagecount, 0);
		messagecount++;
	}
}

//=============================================================================

void system_debug_refresh(void)
{
	code_update();
	reg_update();
	z80reg_update();
	mem_update();
	break_update();
	watch_update();

	if (status)
	{
		if (system_debug_running())
			Static_SetText(status, "Running");
		else
			Static_SetText(status, "Stopped");
	}
}

//=============================================================================
#endif