#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <SDL.h>
#include "neopop.h"

static BOOL load_rom(char *filename);
void system_changed_rom(void);
void system_unload_rom(void);

static BOOL
load_rom(char *filename)
{
    struct stat st;

    if (stat(filename, &st) == -1) {
	system_message("%s `%s': %s", system_get_string(IDS_EROMFIND),
		       filename, strerror(errno));
	return FALSE;
    }

    rom.length = st.st_size;
    rom.data = (char *)calloc(rom.length, 1);

    if (system_io_rom_read(filename, rom.data, rom.length))
	return TRUE;

    system_message("%s `%s': %s", system_get_string(IDS_EROMOPEN),
		   filename, strerror(errno));
    return FALSE;
}

void
system_changed_rom(void)
{
    char title[128];

    (void)snprintf(title, sizeof(title), PROGRAM_NAME " - %s", rom.name);

    /* set window caption */
    SDL_WM_SetCaption(title, NULL);
}
    
BOOL
system_load_rom(char *filename)
{
    char *fn;
    BOOL ret;

    /* Remove old ROM from memory */
    system_unload_rom();

    ret = load_rom(filename);

    if (ret == FALSE)
	return FALSE;

    memset(rom.filename, 0, sizeof(rom.filename));
    if ((fn=strrchr(filename, '/')) == NULL)
	fn = filename;
    else
	*fn++ = '\0';

    /* don't copy extension */
    strncpy(rom.filename, fn, min(sizeof(strncpy), strlen(fn)-5));

    rom_loaded();
    system_changed_rom();

    return TRUE;
}

void
system_unload_rom(void)
{
    rom_unload();

    /* reset window caption */
    SDL_WM_SetCaption(PROGRAM_NAME, NULL);
}
