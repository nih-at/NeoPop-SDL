/* $NiH: neopop-SDL.h,v 1.9 2002/12/03 13:08:20 wiz Exp $ */

#include "neopop.h"
#include <SDL.h>

void system_graphics_fullscreen_toggle(void);
BOOL system_graphics_init(void);
void system_graphics_update(void);

void system_input_update(void);

void system_rom_changed(void);
BOOL system_rom_load(char *);
void system_rom_unload(void);

int system_screenshot(void);

void system_sound_callback(void *, Uint8 *, int);
void system_sound_chipreset(void);
BOOL system_sound_init(void);
void system_sound_shutdown(void);
void system_sound_silence(void);
void system_sound_update(void);

void system_state_load(void);
void system_state_save(void);

extern int do_exit;
extern int graphics_mag_req;
extern int graphics_mag_smooth;
