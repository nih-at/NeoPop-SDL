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

	system_debug.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

21 JUL 2002 - neopop_uk
=======================================
- This file holds the declarations for all debugger related data,
excluding the configuration options - these are held in 'system_config.h'
- Moved this file into the 'Debugger' folder. This is to aid
abstraction, and for porters who don't need to port the debugger.

//---------------------------------------------------------------------------
*/

#ifndef __SYSTEM_DEBUG__
#define __SYSTEM_DEBUG__
//=============================================================================

#ifdef NEOPOP_DEBUG

	//Execution mode
	extern BOOL running_dis_TLCS900h;
	extern BOOL running_dis_Z80;

	void system_debug_init(void);
	void system_debug_shutdown(void);

	void system_debug_start(BOOL dis_TLCS900h, BOOL dis_Z80);

	BOOL system_debug_running(void);

	//Primary address edit box
	_s32 get_debug_address(void);
	void set_debug_address(_s32 address);
		
	//Breakpoint
	typedef struct
	{
		BOOL enabled;
		_u32 address;
	}
	BREAKPOINT;

#define MAX_BREAKPOINTS	4

	extern BREAKPOINT breakpoint[MAX_BREAKPOINTS], breakpoint_z80;

	//Returns true, prints a message and stops the debugger if
	//a breakpoint is reached.
	BOOL breakpoint_reached(void);

	//Reads the current address and sets breakpoint Zero, which is
	//cleared when reached.
	void set_breakpoint_runto(void);

	//Debugger Windows
	extern BOOL memview_enabled;
	extern BOOL regview_enabled;
	extern BOOL z80regview_enabled;
	extern BOOL codeview_enabled;
	extern BOOL breakview_enabled;
	extern BOOL watchview_enabled;

	//Register Viewer
	void reg_open(void);
	void reg_close(void);
	void reg_update(void);

	//Z80 Register Viewer
	void z80reg_open(void);
	void z80reg_close(void);
	void z80reg_update(void);

	//Memory Viewer
	void mem_open(void);
	void mem_close(void);
	void mem_update(void);

	//Code Viewer
	void code_open(void);
	void code_close(void);
	void code_update(void);

	//Breakpoint Viewer
	void break_open(void);
	void break_close(void);
	void break_update(void);

	//Watch Window
	void watch_open(void);
	void watch_close(void);
	void watch_update(void);

#endif

//=============================================================================
#endif
