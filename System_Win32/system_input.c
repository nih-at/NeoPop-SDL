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

	system_input.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

21 JUL 2002 - neopop_uk
=======================================
- Moved the button settings to the 'system_config.h' file

25 JUL 2002 - neopop_uk
=======================================
- Converted to earliest DirectInput version possible (Dx5).
it's also now possible to compile this without C++ also! = faster.

02 AUG 2002 - neopop_uk
=======================================
- Added autofire for keyboard.

11 AUG 2002 - neopop_uk
=======================================
- Added functions: stick_name and stick_count.
 
09 SEP 2002 - neopop_uk
=======================================
- Added adaptoid support.

//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include <windows.h>
#define DIRECTINPUT_VERSION 0x0500
#include <dinput.h>
#include "resource.h"		//For shortcuts

#include "system_main.h"
#include "system_config.h"

//=============================================================================

static LPDIRECTINPUT	di = NULL;		// DirectInput Device
static LPDIRECTINPUTDEVICE2 joy = NULL; // DirectInput Joystick
static LPDIRECTINPUTDEVICE keyboard = NULL; // DirectInput Keyboard

#define INPUT_MAXAXISVALUE 64
#define DEADZONE	16

//This value toggles on and off to create the turbo effect.
static BOOL auto_flipflop = FALSE;

//I/O state
static _u8 up,down,left,right,button_a,button_b,option;

//Used to hold joystick data
static char joy_string[256];
static int joy_count = 0;

//=============================================================================

//-----------------------------------------------------------------------------
// setDIRange()      
//-----------------------------------------------------------------------------
static HRESULT setDIRange(DWORD obj, long max, long min)
{
	DIPROPRANGE diprg;

	diprg.diph.dwSize       = sizeof(diprg);
	diprg.diph.dwHeaderSize = sizeof(diprg.diph);
	diprg.diph.dwObj        = obj;
	diprg.diph.dwHow        = DIPH_BYOFFSET;
	diprg.lMin              = min; 
	diprg.lMax              = max; 

	return IDirectInputDevice_SetProperty(joy, DIPROP_RANGE, &diprg.diph);
}

//-----------------------------------------------------------------------------
// system_input_buttoncount()
//-----------------------------------------------------------------------------
int system_input_buttoncount(void)
{
	if (joy)
	{
		// Get the Caps
		DIDEVCAPS caps;
		ZeroMemory(&caps, sizeof(DIDEVCAPS));
		caps.dwSize = sizeof(DIDEVCAPS);
		IDirectInputDevice_GetCapabilities(joy, &caps);
		return caps.dwButtons;
	}
	else
	{
		return -1;	//No stick
	}
}

//-----------------------------------------------------------------------------
// system_input_buttonname()
//-----------------------------------------------------------------------------
BOOL CALLBACK DIEnumDeviceObjectsCallback(LPCDIDEVICEOBJECTINSTANCE obj,
										  void* ref) 
{
	if (joy_count == *(int*)ref)
	{
		sprintf(joy_string, "%s", obj->tszName);
		return DIENUM_STOP;
	}

	joy_count++;
	return DIENUM_CONTINUE;
} 

char* system_input_buttonname(int i)
{
	joy_count = 0;
	sprintf(joy_string, "Button %d", i+1);
	IDirectInputDevice2_EnumObjects(joy, DIEnumDeviceObjectsCallback, 
		(void*)&i, DIDFT_BUTTON); 
	return strdup(joy_string);
}

//-----------------------------------------------------------------------------
// system_input_stickcount()
//-----------------------------------------------------------------------------
BOOL CALLBACK CountJoysticksCallback(const DIDEVICEINSTANCE* instance, void* data)
{
	joy_count++;
	return DIENUM_CONTINUE;
}

int system_input_stickcount(void)
{
	joy_count = 0;
	IDirectInput_EnumDevices(di, DIDEVTYPE_JOYSTICK, CountJoysticksCallback,
		NULL, DIEDFL_ATTACHEDONLY);
	return joy_count;
}

//-----------------------------------------------------------------------------
// system_input_stickname()
//-----------------------------------------------------------------------------
BOOL CALLBACK NameJoysticksCallback(const DIDEVICEINSTANCE* instance, void* data)
{
	if (joy_count == *(int*)data)
	{
		sprintf(joy_string, "%s", instance->tszProductName);
		return DIENUM_STOP;
	}

	joy_count++;
	return DIENUM_CONTINUE;
}

char* system_input_stickname(int i)
{
	sprintf(joy_string, "Joystick %d", i+1);
	joy_count = 0;
	IDirectInput_EnumDevices(di, DIDEVTYPE_JOYSTICK, NameJoysticksCallback,
		&i, DIEDFL_ATTACHEDONLY);
	return strdup(joy_string);
}

//=============================================================================

//-----------------------------------------------------------------------------
// key_init()
//-----------------------------------------------------------------------------
static BOOL key_init(void)
{
	// Create a device
	if FAILED(IDirectInput_CreateDevice(di, &GUID_SysKeyboard, &keyboard, 0))
		return FALSE;

	// Set data format
	if FAILED(IDirectInputDevice_SetDataFormat(keyboard, &c_dfDIKeyboard))
	{
		IDirectInputDevice_Release(keyboard);	keyboard = NULL;
		return FALSE;
	}

	// Set the cooperative level of the keyboard 
 	if FAILED(IDirectInputDevice_SetCooperativeLevel(keyboard, g_hWnd, 
			DISCL_FOREGROUND | DISCL_NONEXCLUSIVE))
	{
		IDirectInputDevice_Release(keyboard);	keyboard = NULL;
		return FALSE;
	}

	IDirectInputDevice_Acquire(keyboard);	// Try to Acquire the keyboard
	return TRUE;
}

//-----------------------------------------------------------------------------
// EnumJoysticksCallback()
//-----------------------------------------------------------------------------
BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* instance, void* data)
{
    HRESULT hr;

	if (joy_count != STICK)
	{
		joy_count++;
        return DIENUM_CONTINUE;
	}

    // Obtain an interface to the enumerated joystick.
	if FAILED(IDirectInput_CreateDevice(di, &instance->guidInstance, (LPDIRECTINPUTDEVICE*)&joy, 0))
	{
		STICK++;
		joy_count++;
		return DIENUM_CONTINUE;
	}
   
	if FAILED(hr = IDirectInputDevice_SetDataFormat(joy, &c_dfDIJoystick2))
	{
		IDirectInputDevice_Release(joy); joy = NULL;
		return DIENUM_STOP;
	}

	if FAILED(hr = IDirectInputDevice_SetCooperativeLevel(joy, g_hWnd, 
		DISCL_EXCLUSIVE | DISCL_FOREGROUND))
	{
		IDirectInputDevice_Release(joy); joy = NULL;
		return DIENUM_STOP;
	}

	//X-AXIS
	if FAILED(hr = setDIRange(DIJOFS_X, +INPUT_MAXAXISVALUE, -INPUT_MAXAXISVALUE))
	{
		IDirectInputDevice_Release(joy); joy = NULL;
		return DIENUM_STOP;
	}
		
	//Y-AXIS
	if FAILED(hr = setDIRange(DIJOFS_Y, +INPUT_MAXAXISVALUE, -INPUT_MAXAXISVALUE))
	{
		IDirectInputDevice_Release(joy); joy = NULL;
		return DIENUM_STOP;
	}

	IDirectInputDevice_Acquire(joy); // Try to Acquire the joystick

    return DIENUM_STOP;
}

//-----------------------------------------------------------------------------
// system_input_init()
//-----------------------------------------------------------------------------
int system_input_init(void)
{
	// Create DirectInput interface
	if FAILED(DirectInputCreate(g_hInstance, DIRECTINPUT_VERSION, &di, 0))
		return 0;

	key_init();

	// Ignore big stick identifier
	if (STICK >= system_input_stickcount())
		STICK = 0;

	// Find the joystick
	joy_count = 0;
	IDirectInput_EnumDevices(di, DIDEVTYPE_JOYSTICK, EnumJoysticksCallback,
		NULL, DIEDFL_ATTACHEDONLY);

	if (joy || keyboard)
		return 1;
	else
		return 0;
}

//-----------------------------------------------------------------------------
// system_input_shutdown()
//-----------------------------------------------------------------------------
void system_input_shutdown(void)
{
	if (joy)
	{
		IDirectInputDevice_Unacquire(joy);
		IDirectInputDevice_Release(joy);
		joy = NULL;
	}

	if (keyboard)
	{
		IDirectInputDevice_Unacquire(keyboard);
		IDirectInputDevice_Release(keyboard);
		keyboard = NULL;
	}

	if (di)	
	{
		IDirectInput_Release(di);
		di = NULL;
	}
}

//-----------------------------------------------------------------------------
// system_input_update()
//-----------------------------------------------------------------------------
void system_input_update(void)
{
	//Clear the controller status
	up = down = left = right = button_a = button_b = option = 0;

	//Autofire flip flop.
	auto_flipflop = !auto_flipflop;

    if (joy)	//Check to see if the joystick is active
	{
		DIJOYSTATE2 js;           // Direct Input joystick state 

		if FAILED(IDirectInputDevice2_Poll(joy))
		{
			IDirectInputDevice_Acquire(joy);
			return;
		}

		if FAILED(IDirectInputDevice_GetDeviceState(joy, sizeof(DIJOYSTATE2), &js))
		{
			IDirectInputDevice_Acquire(joy);
			return;
		}

		//D-Pad
		if (js.lX < -DEADZONE)	left = 1;
		if (js.lX > +DEADZONE)	right = 1;
		if (js.lY < -DEADZONE)	up = 1;
		if (js.lY > +DEADZONE)	down = 1;

		//Adaptoid D-Pad
		if (adaptoid)
		{
			if (js.rgbButtons[11] & 0x80)	up = 1;
			if (js.rgbButtons[12] & 0x80)	down = 1;
			if (js.rgbButtons[13] & 0x80)	left = 1;
			if (js.rgbButtons[14] & 0x80)	right = 1;
		}

		//Buttons
		if (js.rgbButtons[BUTTONA] & 0x80)	button_a = 1;
		if (js.rgbButtons[BUTTONB] & 0x80)	button_b = 1;
		if (js.rgbButtons[BUTTONO] & 0x80)	option = 1;

		//Autofire
		if (AUTOA && auto_flipflop)		button_a = 0;
		if (AUTOB && auto_flipflop)		button_b = 0;
	}

	if (keyboard) 
    {
		char ks[256];
 
		if FAILED(IDirectInputDevice_GetDeviceState(keyboard, sizeof(ks), &ks))
		{
			IDirectInputDevice_Acquire(keyboard);
			return;
		}

		//D-Pad
		if (ks[KEYL] & 0x80)	left = 1;
		if (ks[KEYR] & 0x80)	right = 1;
		if (ks[KEYU] & 0x80)	up = 1;
		if (ks[KEYD] & 0x80)	down = 1;

		//Buttons
		if (ks[KEYA] & 0x80)	button_a = 1;
		if (ks[KEYB] & 0x80)	button_b = 1;
		if (ks[KEYO] & 0x80)	option = 1;

		//Autofire
		if (AUTOA && auto_flipflop)		button_a = 0;
		if (AUTOB && auto_flipflop)		button_b = 0;
	}

	//Write the controller status to memory
	ram[0x6F82] = up | (down << 1) | (left << 2) | (right << 3) | 
		(button_a << 4) | (button_b << 5) | (option << 6);
}

//=============================================================================
