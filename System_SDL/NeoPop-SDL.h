/* $NiH: NeoPop-SDL.h,v 1.3 2003/10/16 17:29:44 wiz Exp $ */
/*
  NeoPop-SDL.h -- common header file
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

#include "neopop.h"
#include <SDL.h>

/* one per line, please */
enum neopop_event {
    /* unbound key */
    NPEV_NONE,
    
    /* console input, keep these in order */
    NPEV_JOY_UP,
    NPEV_JOY_DOWN,
    NPEV_JOY_LEFT,
    NPEV_JOY_RIGHT,
    NPEV_JOY_BUTTON_A,
    NPEV_JOY_BUTTON_B,
    NPEV_JOY_OPTION,
    
    /* gui events */
    NPEV_GUI_FRAMESKIP_DECREMENT,
    NPEV_GUI_FRAMESKIP_INCREMENT,
    NPEV_GUI_FULLSCREEN_OFF,
    NPEV_GUI_FULLSCREEN_ON,
    NPEV_GUI_FULLSCREEN_TOGGLE,
    NPEV_GUI_MAGNIFY_1,
    NPEV_GUI_MAGNIFY_2,
    NPEV_GUI_MAGNIFY_3,
    NPEV_GUI_MENU,
    NPEV_GUI_MUTE_OFF,
    NPEV_GUI_MUTE_ON,
    NPEV_GUI_MUTE_TOGGLE,
    NPEV_GUI_QUIT,
    NPEV_GUI_SCREENSHOT,
    NPEV_GUI_SMOOTH_OFF,
    NPEV_GUI_SMOOTH_ON,
    NPEV_GUI_SMOOTH_TOGGLE,
    NPEV_GUI_STATE_LOAD,
    NPEV_GUI_STATE_SAVE,

    NPEV_LAST
};

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

extern const char *event_names[];

void read_bindings(const char *fname);
