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

	system_io.c

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

21 JUL 2002 - neopop_uk
=======================================
- Added the new IO functions.

25 JUL 2002 - neopop_uk
=======================================
- Added the two flash io functions - they completely govern where
	the given buffer is stored. This should provide the flexibility
	required to store to an exotic device such as a memory card.
  
//---------------------------------------------------------------------------
*/

#include "neopop.h"
#include "system_config.h"
#include <direct.h>

//=============================================================================

//-----------------------------------------------------------------------------
// system_io_rom_read()
//-----------------------------------------------------------------------------
BOOL system_io_rom_read(char* filename, _u8* buffer, _u32 bufferLength)
{
	FILE* file;
	file = fopen(filename, "rb");

	if (file)
	{
		fread(buffer, bufferLength, sizeof(_u8), file);
		fclose(file);
		return TRUE;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
// system_io_flash_read()
//-----------------------------------------------------------------------------
BOOL system_io_flash_read(_u8* buffer, _u32 bufferLength)
{
	char path[256];
	FILE* file;

	//Build a path to the flash file
	sprintf(path, "%s\\%s.ngf", FlashDirectory, rom.filename);

	file = fopen(path, "rb");

	if (file)
	{
		fread(buffer, bufferLength, sizeof(_u8), file);
		fclose(file);
		return TRUE;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
// system_io_flash_write()
//-----------------------------------------------------------------------------
BOOL system_io_flash_write(_u8* buffer, _u32 bufferLength)
{
	char path[256];
	FILE* file;

	//Build a path for the flash file
	sprintf(path, "%s\\%s.ngf", FlashDirectory, rom.filename);
	
	file = fopen(path, "wb");

	if (file)
	{
		fwrite(buffer, bufferLength, sizeof(_u8), file);
		fclose(file);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//-----------------------------------------------------------------------------
// system_io_state_read()
//-----------------------------------------------------------------------------
BOOL system_io_state_read(char* filename, _u8* buffer, _u32 bufferLength)
{
	FILE* file;
	file = fopen(filename, "rb");

	if (file)
	{
		fread(buffer, bufferLength, sizeof(_u8), file);
		fclose(file);
		return TRUE;
	}

	return FALSE;
}

//-----------------------------------------------------------------------------
// system_io_state_write()
//-----------------------------------------------------------------------------
BOOL system_io_state_write(char* filename, _u8* buffer, _u32 bufferLength)
{
	FILE* file;
	file = fopen(filename, "wb");

	if (file)
	{
		fwrite(buffer, bufferLength, sizeof(_u8), file);
		fclose(file);
		return TRUE;
	}

	return FALSE;
}


//=============================================================================
