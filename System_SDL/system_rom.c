/* $NiH: system_rom.c,v 1.8 2003/10/16 17:29:45 wiz Exp $ */
/*
  system_rom.c -- ROM loading support
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
#include <zip.h>

#include "config.h"
#include "NeoPop-SDL.h"

static BOOL rom_load(char *);

static BOOL
rom_load(char *filename)
{
    struct stat st;

    if (stat(filename, &st) == -1) {
	system_message("%s `%s': %s", system_get_string(IDS_EROMFIND),
		       filename, strerror(errno));
	return FALSE;
    }

#ifdef HAVE_LIBZIP
 {
     struct zip *z;
     struct zip_file *zf;
     struct zip_stat zst;
     int i, n, l;

     if ((z=zip_open(filename, 0, NULL)) != NULL) {
	 n = zip_get_num_files(z);
	 for (i=0; i<n; i++) {
	     if (zip_stat_index(z, i, 0, &zst) != 0)
		 continue;
	     l = strlen(zst.name);
	     if (l < 4)
		 continue;
	     if (strcasecmp(zst.name+l-4, ".ngp") == 0) {
		 rom.length = zst.size;
		 rom.data = (char *)calloc(rom.length, 1);

		 if ((zf=zip_fopen_index(z, i, 0)) == NULL
		     || zip_fread(zf, rom.data, rom.length) != rom.length) {
		     /* XXX: free(rom.data); ?! */
		     system_message("%s `%s': %s",
				    system_get_string(IDS_EROMOPEN),
				    filename, zip_strerror(z));
		     return FALSE;
		 }
		 zip_fclose(zf);
		 zip_close(z);
			
		 return TRUE;
	     }
	 }
	 zip_close(z);
	 system_message("%s `%s': no rom found",
			system_get_string(IDS_EROMOPEN), filename);
	 return FALSE;
     }
 }
#endif

    rom.length = st.st_size;
    rom.data = (char *)calloc(rom.length, 1);

    if (system_io_rom_read(filename, rom.data, rom.length))
	return TRUE;

    system_message("%s `%s': %s", system_get_string(IDS_EROMOPEN),
		   filename, strerror(errno));
    return FALSE;
}

void
system_rom_changed(void)
{
    char title[128];

    (void)snprintf(title, sizeof(title), PROGRAM_NAME " - %s", rom.name);

    /* set window caption */
    SDL_WM_SetCaption(title, NULL);
}
    
BOOL
system_rom_load(char *filename)
{
    char *fn;
    BOOL ret;

    /* Remove old ROM from memory */
    system_rom_unload();

    ret = rom_load(filename);

    if (ret == FALSE)
	return FALSE;

    memset(rom.filename, 0, sizeof(rom.filename));
    if ((fn=strrchr(filename, '/')) == NULL)
	fn = filename;
    else
	*fn++ = '\0';

    /* don't copy extension */
    strncpy(rom.filename, fn, min(sizeof(rom.filename), strlen(fn)-4));

    rom_loaded();
    system_rom_changed();

    return TRUE;
}

void
system_rom_unload(void)
{
    rom_unload();

    /* reset window caption */
    SDL_WM_SetCaption(PROGRAM_NAME, NULL);
}
