/* $NiH: system_io.c,v 1.7 2003/10/15 12:30:03 wiz Exp $ */
/*
  system_io.c -- read/write flash files 
  Copyright (C) 2002-2003 Thomas Klausner

  This file is part of NeoPop-SDL, a NeoGeo Pocket emulator
  The author can be contacted at <wiz@danbala.tuwien.ac.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "neopop.h"

static BOOL
read_file_to_buffer(char *filename, _u8 *buffer, _u32 len)
{
    FILE *fp;
    _u32 got;

    if ((fp=fopen(filename, "rb")) == NULL)
	return FALSE;

    while ((got=fread(buffer, 1, len, fp)) < len) {
	if (feof(fp)) {
	    fclose(fp);
	    return FALSE;
	}
	if (ferror(fp) && errno != EINTR) {
	    fclose(fp);
	    return FALSE;
	}

	len -= got;
	buffer += got;
    }
	    
    if (fclose(fp) != 0)
	return FALSE;

    return TRUE;
}

static BOOL
write_file_from_buffer(char *filename, _u8 *buffer, _u32 len)
{
    FILE *fp;
    _u32 written;

    if ((fp=fopen(filename, "wb")) == NULL)
	return FALSE;

    while ((written=fwrite(buffer, 1, len, fp)) < len) {
	if (feof(fp)) {
	    fclose(fp);
	    return FALSE;
	}
	if (ferror(fp) && errno != EINTR) {
	    fclose(fp);
	    return FALSE;
	}

	len -= written;
	buffer += written;
    }
	    
    if (fclose(fp) != 0)
	return FALSE;

    return TRUE;
}


static BOOL
validate_dir(char *path)
{
    struct stat sb;

    if (stat(path, &sb) == -1 && errno == ENOENT) {
    	if (mkdir(path, 0777) == -1) {
	    fprintf(stderr, "Config directory `%s' does not exist and "
		    "creation failed: %s", path, strerror(errno));
	    return FALSE;
	}
	if (stat(path, &sb) == -1) {
	    fprintf(stderr, "Config directory `%s' creation failed: %s",
		    path, strerror(errno));
	    return FALSE;
	}
    }

    if ((sb.st_mode & S_IFDIR) != S_IFDIR) {
	fprintf(stderr, "Config directory `%s' is not a directory", path);
	return FALSE;
    }

    return TRUE;
}

void
system_state_load(void)
{
    char fn[256];

    snprintf(fn, sizeof(fn), "%s.ngs", rom.filename);
    state_restore(fn);

    return;
}

void
system_state_save(void)
{
    char fn[256];

    snprintf(fn, sizeof(fn), "%s.ngs", rom.filename);
    state_store(fn);

    return;
}

BOOL
system_io_rom_read(char *filename, _u8 *buffer, _u32 len)
{
    return read_file_to_buffer(filename, buffer, len);
}

BOOL
system_io_flash_read(_u8* buffer, _u32 len)
{
    char path[256];
    char *homedir;

    if ((homedir=getenv("HOME")) == NULL) {
	fprintf(stderr, "can't find home directory");
	return FALSE;
    }

    /* Build a path to the flash file */
    snprintf(path, sizeof(path), "%s/.neopop/%s.ngf", homedir,
	     rom.filename);

    return read_file_to_buffer(path, buffer, len);
}

BOOL
system_io_flash_write(_u8* buffer, _u32 len)
{
    char path[256];
    char *homedir;
    char flash_dir[256];

    if ((homedir=getenv("HOME")) == NULL) {
	fprintf(stderr, "can't find home directory");
	return FALSE;
    }

    /* Make sure flash dir exists */
    snprintf(flash_dir, sizeof(flash_dir), "%s/.neopop", homedir);
    if (validate_dir(flash_dir) < 0)
	return FALSE;

    /* Build a path for the flash file */
    snprintf(path, sizeof(path), "%s/%s.ngf", flash_dir,
	     rom.filename);

    return write_file_from_buffer(path, buffer, len);
}

BOOL
system_io_state_read(char *filename, _u8 *buffer, _u32 len)
{
    return read_file_to_buffer(filename, buffer, len);
}

BOOL
system_io_state_write(char *filename, _u8 *buffer, _u32 len)
{
    return write_file_from_buffer(filename, buffer, len);
}
