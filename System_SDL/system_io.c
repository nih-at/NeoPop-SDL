/* $NiH: system_io.c,v 1.11 2004/07/10 02:39:35 dillo Exp $ */
/*
  system_io.c -- read/write flash files 
  Copyright (C) 2002-2004 Thomas Klausner and Dieter Baron

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

#include "NeoPop-SDL.h"

char *state_dir;
char *flash_dir;
int use_rom_name;
int state_slot;

static char *make_file_name(const char *, const char *, int);


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
validate_dir(const char *path)
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
    char *fn, ext[4];

    sprintf(ext, "ng%c", (state_slot ? state_slot+'0' : 's'));
    if ((fn=make_file_name(state_dir, ext, FALSE)) == NULL)
	return;
    state_restore(fn);
    free(fn);

    return;
}

void
system_state_save(void)
{
    char *fn, ext[4];

    sprintf(ext, "ng%c", (state_slot ? state_slot+'0' : 's'));
    if ((fn=make_file_name(state_dir, ext, TRUE)) == NULL)
	return;
    state_store(fn);
    free(fn);

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
    char *fn;
    int ret;

    if ((fn=make_file_name(flash_dir, "ngf", FALSE)) == NULL)
	return FALSE;
    ret = read_file_to_buffer(fn, buffer, len);
    free(fn);
    return ret;
}

BOOL
system_io_flash_write(_u8* buffer, _u32 len)
{
    char *fn;
    int ret;

    if ((fn=make_file_name(flash_dir, "ngf", TRUE)) == NULL)
	return FALSE;
    ret = write_file_from_buffer(fn, buffer, len);
    free(fn);
    return ret;
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

static char *
make_file_name(const char *dir, const char *ext, int writing)
{
    char *fname, *name, *home, *p;
    int len;

    if (use_rom_name)
	name = rom.name;
    else
	name = rom.filename;
    len = strlen(dir)+strlen(name)+strlen(ext)+3;

    home = NULL;
    if (strncmp(dir, "~/", 2) == 0) {
	home = getenv("HOME");
	if (home == NULL)
	    return NULL;
	len += strlen(home)-1;
    }

    if ((fname=malloc(len)) == NULL)
	return NULL;

    if (strncmp(dir, "~/", 2) == 0)
	sprintf(fname, "%s%s", home, dir+1);
    else
	strcpy(fname, dir);

    if (writing && !validate_dir(fname))
	return NULL;

    p = fname+strlen(fname);
    sprintf(p, "/%s.%s", name, ext);
    while (*(++p))
	if (*p == '/')
	    *p = '_';

    return fname;
}
