#include "neopop.h"

/* XXX */
char *FlashDirectory="/home/wiz/.neopop";

BOOL
system_io_rom_read(char* filename, _u8* buffer, _u32 bufferLength)
{
    FILE* file;
    file = fopen(filename, "rb");

    if (file) {
	fread(buffer, bufferLength, sizeof(_u8), file);
	fclose(file);
	return TRUE;
    }

    return FALSE;
}

BOOL
system_io_flash_read(_u8* buffer, _u32 bufferLength)
{
    char path[256];
    FILE* file;

    /* Build a path to the flash file */
    sprintf(path, "%s/%s.ngf", FlashDirectory, rom.filename);

    file = fopen(path, "rb");

    if (file) {
	fread(buffer, bufferLength, sizeof(_u8), file);
	fclose(file);
	return TRUE;
    }

    return FALSE;
}

BOOL
system_io_flash_write(_u8* buffer, _u32 bufferLength)
{
    char path[256];
    FILE* file;

    /* Build a path for the flash file */
    sprintf(path, "%s/%s.ngf", FlashDirectory, rom.filename);
	
    file = fopen(path, "wb");

    if (file) {
	fwrite(buffer, bufferLength, sizeof(_u8), file);
	fclose(file);
	return TRUE;
    }

    return FALSE;
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
