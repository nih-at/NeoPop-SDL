/* $NiH: system_input.c,v 1.11 2004/06/20 23:48:04 dillo Exp $ */
/*
  system_input.c -- input support functions
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

#include <errno.h>
#include <ctype.h>

#include "NeoPop-SDL.h"

static const int joy_mask[] = {
    0x01, /* up */
    0x02, /* down */
    0x04, /* left */
    0x08, /* right */
    0x10, /* button a */
    0x20, /* button b */
    0x40  /* option */
};

/*
  0..SDLK_LAST			 keys
  SDLK_LAST..2*SDLK_LAST	control keys
  2*SDLK_LAST..3*SDLK_LAST	alt keys
  3*SDLK_LAST..			joysticks

  joysticks:

  0..14		axis
  14..18	hat
  18..38	buttons
*/

enum npks_shift {
    NPKS_SH_NONE,
    NPKS_SH_CTRL,
    NPKS_SH_ALT,
    NPKS_NKEY
};

#define NPKS_KEY_BASE		0
#define NPKS_KEY_SIZE		SDLK_LAST
#define NPKS_JOY_BASE		(NPKS_KEY_BASE+NPKS_NKEY*NPKS_KEY_SIZE)
#define NPKS_NJOY		1
#define NPKS_JOY_NAXIS		7
#define NPKS_JOY_NHAT		1
#define NPKS_JOY_NBUTTON	20
#define NPKS_JOY_AXIS_OFFSET	0
#define NPKS_JOY_HAT_OFFSET	(2*NPKS_JOY_NAXIS)
#define NPKS_JOY_BUTTON_OFFSET	(NPKS_JOY_HAT_OFFSET+4*NPKS_JOY_NHAT)
#define NPKS_JOY_SIZE		(NPKS_JOY_BUTTON_OFFSET+NPKS_JOY_NBUTTON)
#define NPKS_SIZE		(NPKS_NKEY*NPKS_KEY_SIZE	\
				 +NPKS_NJOY*NPKS_JOY_SIZE)

#define NPKS_KEY(c, k)		(NPKS_KEY_BASE+(c)*NPKS_KEY_SIZE+(k))
#define NPKS_JOY(n)		(NPKS_JOY_BASE+(n)*NPKS_JOY_SIZE)
#define NPKS_JOY_AXIS(n, i)	(NPKS_JOY(n)+NPKS_JOY_AXIS_OFFSET+2*(i))
#define NPKS_JOY_HAT(n, i)	(NPKS_JOY(n)+NPKS_JOY_HAT_OFFSET+4*(i))
#define NPKS_JOY_BUTTON(n, i)	(NPKS_JOY(n)+NPKS_JOY_BUTTON_OFFSET+(i))

enum neopop_event bindings[NPKS_SIZE];
int joy_pressed[NPKS_JOY_SIZE];

#define JOY_PRESSED(k)	(joy_pressed[(k)-NPKS_JOY_BASE])

#define JOYPORT_ADDR	0x6F82

int find_name(const char *p, char **end, const char **names, int n);
void handle_event(enum neopop_event ev, int down);
const char *npev_name(enum neopop_event ev);
int npev_parse(const char *name, char **end);
const char *npks_name(int k);
int npks_parse(const char *name, char **end);


void
system_input_update(void)
{
    int k, down;
    SDL_Event evt;

    while(SDL_PollEvent(&evt)) {
	k = -1;
	switch(evt.type) {
	case SDL_QUIT:
	    do_exit = 1;
	    break;
	    
	case SDL_KEYUP:
	case SDL_KEYDOWN:
	    {
		int c;

		switch(evt.key.keysym.sym) {
		case SDLK_LCTRL:
		case SDLK_RCTRL:
		case SDLK_LALT:
		case SDLK_RALT:
		    /* shit keys are never considered shifted */
		    c = NPKS_SH_NONE;
		    break;
		default:
		    if (evt.key.keysym.mod & KMOD_CTRL)
			c = NPKS_SH_CTRL;
		    else if (evt.key.keysym.mod & KMOD_ALT)
			c = NPKS_SH_ALT;
		    else
			c = NPKS_SH_NONE;
		    break;
		}
		k = NPKS_KEY(c, evt.key.keysym.sym);
		down = (evt.type == SDL_KEYDOWN);
	    }
	    break;
	    
	case SDL_JOYAXISMOTION:
	    k = NPKS_JOY_AXIS(evt.jaxis.which, evt.jaxis.axis);

	    down = 1;
	    if (evt.jaxis.value < -10922) {
		if (JOY_PRESSED(k))
		    k = -1;
		else
		    JOY_PRESSED(k) = 1;
	    }
	    else if (evt.jaxis.value > 10922) {
		k++;
		if (JOY_PRESSED(k))
		    k = -1;
		else
		    JOY_PRESSED(k) = 1;
	    }
	    else {
		down = 0;
		if (JOY_PRESSED(k))
		    JOY_PRESSED(k) = 0;
		else if (JOY_PRESSED(k+1)) {
		    k++;
		    JOY_PRESSED(k) = 0;
		}		    
		else 
		    k = -1;
	    }
	    break;
	    
        case SDL_JOYBUTTONUP:
        case SDL_JOYBUTTONDOWN:
	    k = NPKS_JOY_BUTTON(evt.jbutton.which, evt.jbutton.button);
	    down = (evt.type == SDL_JOYBUTTONDOWN);
	    if (JOY_PRESSED(k)) {
		if (down)
		    k = -1;
		else
		    JOY_PRESSED(k) = 0;
	    }
	    else {
		if (down)
		    JOY_PRESSED(k) = 1;
		else
		    k = -1;
	    }
	    break;

	/* XXX: SDL_JOYHAT */
	    
	default:
	    /* ignore */
	    break;
	}

	if (k != -1) {
#ifdef KEY_DEBUG
	    printf("key %s (%d) %s == %s\n",
		   npks_name(k), k, (down ? "down" : "up"),
		   npev_name(bindings[k]));
#endif
	    handle_event(bindings[k], down);
	}
    }
}



void
handle_event(enum neopop_event ev, int down)
{
    if (down) {
	switch (ev) {
	case NPEV_NONE:
	    break;

	case NPEV_JOY_UP:
	case NPEV_JOY_DOWN:
	case NPEV_JOY_LEFT:
	case NPEV_JOY_RIGHT:
	case NPEV_JOY_BUTTON_A:
	case NPEV_JOY_BUTTON_B:
	case NPEV_JOY_OPTION:
	    ram[JOYPORT_ADDR] |= joy_mask[ev-NPEV_JOY_UP];
	    break;

	case NPEV_GUI_FRAMESKIP_DECREMENT:
	    if (system_frameskip_key > 1)
		system_frameskip_key--;
	    break;
	case NPEV_GUI_FRAMESKIP_INCREMENT:
	    if (system_frameskip_key < 7)
		system_frameskip_key++;
	    break;
	    
	case NPEV_GUI_FULLSCREEN_OFF:
	    /* not implemented yet */
	    break;
	case NPEV_GUI_FULLSCREEN_ON:
	    /* not implemented yet */
	    break;
	case NPEV_GUI_FULLSCREEN_TOGGLE:
	    system_graphics_fullscreen_toggle();
	    break;
	    
	case NPEV_GUI_MAGNIFY_1:
	case NPEV_GUI_MAGNIFY_2:
	case NPEV_GUI_MAGNIFY_3:
	    graphics_mag_req = ev-NPEV_GUI_MAGNIFY_1+1;
	    break;
	    
	case NPEV_GUI_MENU:
	    /* not implemented yet */
	    break;
	    
	case NPEV_GUI_MUTE_OFF:
	    mute = 0;
	    break;
	case NPEV_GUI_MUTE_ON:
	    mute = 1;
	    break;
	case NPEV_GUI_MUTE_TOGGLE:
	    mute = !mute;
	    break;
	    
	case NPEV_GUI_QUIT:
	    do_exit = 1;
	    break;
	    
	case NPEV_GUI_SCREENSHOT:
	    system_screenshot();
	    break;
	    
	case NPEV_GUI_SMOOTH_OFF:
	    graphics_mag_smooth = 0;
	    break;
	case NPEV_GUI_SMOOTH_ON:
	    graphics_mag_smooth = 1;
	    break;
	case NPEV_GUI_SMOOTH_TOGGLE:
	    graphics_mag_smooth = !graphics_mag_smooth;
	    break;
	    
	case NPEV_GUI_STATE_LOAD:
	    system_state_load();
	    break;
	case NPEV_GUI_STATE_SAVE:
	    system_state_save();
	    break;

	case NPEV_LAST:
	    break;
	}
    }
    else {
	switch (ev) {
	case NPEV_JOY_UP:
	case NPEV_JOY_DOWN:
	case NPEV_JOY_LEFT:
	case NPEV_JOY_RIGHT:
	case NPEV_JOY_BUTTON_A:
	case NPEV_JOY_BUTTON_B:
	case NPEV_JOY_OPTION:
	    ram[JOYPORT_ADDR] &= ~joy_mask[ev-NPEV_JOY_UP];
	    break;

	default:
	    break;
	}
    }
}



const char *
npks_name(int k)
{
    static const char *shift_name[] = {
	"", "C-", "M-"
    };

    static char b[8192];
    int n;

    if (k < NPKS_JOY_BASE) {
	n = k / NPKS_KEY_SIZE;
	k %= NPKS_KEY_SIZE;

	snprintf(b, sizeof(b), "%s%s", shift_name[n], SDL_GetKeyName(k));
    }
    else {
	k -= NPKS_JOY_BASE;
	n = k / NPKS_JOY_SIZE;
	k %= NPKS_JOY_SIZE;

	if (k < NPKS_JOY_HAT_OFFSET)
	    snprintf(b, sizeof(b), "joy %d axis %d %s",
		     n+1, (k/2)+1, (k%2 ? "pos" : "neg"));
	else if (k < NPKS_JOY_BUTTON_OFFSET)
	    snprintf(b, sizeof(b), "joy %d hat", n+1);
	else {
	    k -= NPKS_JOY_BUTTON_OFFSET;
	    snprintf(b, sizeof(b), "joy %d button %d", n+1, k+1);
	}
    }

    return b;
}



int
npks_parse(const char *name, char **end)
{
    static const char *key_name[SDLK_LAST];
    static int key_name_inited = 0;

    const char *p;
    int n, k, i;

    p = name+strspn(name, " \t");
    if (strncasecmp(p, "joy", 3) == 0) {
	p += 4;
	n = strtoul(p, (char **)&p, 10)-1;
	if (n < 0 || n >= NPKS_NJOY)
	    return -1;

	p += strspn(p, " \t");
	if (strncasecmp(p, "axis", 4) == 0) {
	    p += 4;
	    k = strtoul(p, (char **)&p, 10)-1;
	    if (k < 0 || k >= NPKS_JOY_NAXIS)
		return -1;
	    
	    p += strspn(p, " \t");
	    if (strncasecmp(p, "pos", 3) == 0)
		i = 1;
	    else if (strncasecmp(p, "neg", 3) == 0)
		i = 0;
	    else
		return -1;

	    p += 3;

	    if (end)
		*end = (char *)p;
	    return NPKS_JOY_AXIS(n, k) + i;
	}
	else if (strncasecmp(p, "button", 6) == 0) {
	    p += 6;

	    k = strtoul(p, (char **)&p, 10)-1;
	    if (k < 0 || k >= NPKS_JOY_NBUTTON)
		return -1;

	    if (end)
		*end = (char *)p;
	    return NPKS_JOY_BUTTON(n, k);
	}
	else
	    return -1;
    }
    else {
	if (!key_name_inited) {
	    for (i=0; i<SDLK_LAST; i++) {
		key_name[i] = SDL_GetKeyName(i);
		if (strcmp(key_name[i], "unknown key") == 0)
		    key_name[i] = NULL;
	    }
	    key_name_inited = 1;
	}

	n = NPKS_SH_NONE;
	if (p[1] == '-') {
	    switch (p[0]) {
	    case 'C':
	    case 'c':
		n = NPKS_SH_CTRL;
		p += 2; 
		break;
	    case 'A':
	    case 'a':
	    case 'M':
	    case 'm':
		n = NPKS_SH_ALT;
		p += 2; 
		break;
	    }
	}

	if ((k=find_name(p, (char **)&p, key_name, SDLK_LAST)) != -1) {
	    if (end)
		*end = (char *)p;
	    return NPKS_KEY(n, k);
	}
	return -1;
    }    
}



void
read_bindings(const char *fname)
{
    char b[8192], *p;
    FILE *f;
    int lno;
    int k;
    int ev;

    if ((f=fopen("keysyms.txt", "w")) != NULL) {
	for (k=0; k<SDLK_LAST; k++)
	    fprintf(f, "%d\t%s\n", k, SDL_GetKeyName(k));
	fclose(f);
    }

    if ((f=fopen(fname, "r")) == NULL) {
	printf("cannot open file `%s': %s\n",
	       fname, strerror(errno));
	return;
    }

    lno = 0;
    while (fgets(b, sizeof(b), f)) {
	lno++;
	if (b[strlen(b)-1] == '\n')
	    b[strlen(b)-1] = '\0';
	if (b[0] == '#' || b[0] == '\0')
	    continue;

	p = b+strspn(b, " \t");
	if (strncasecmp(p, "map", 3) == 0) {
	    p += 3;
	    if ((k=npks_parse(p, (char **)&p)) < 0) {
		printf("%s:%d: cannot parse key [%s]\n",
		       fname, lno, p);
		continue;
	    }
	    p += strspn(p, " \t");
	    if (p[0] != '=') {
		printf("%s:%d: missing = in map [%s]\n",
		       fname, lno, p);
		continue;
	    }
	    p += strspn(p+1, " \t");
	    if ((ev=npev_parse(p, (char **)&p)) < 0) {
		printf("%s:%d: cannot parse event [%s]\n",
		       fname, lno, p);
		continue;
	    }
	    p += strspn(p, " \t");
	    if (p[0] != '\0') {
		printf("%s:%d: trailing garbage in map [%s]\n",
		       fname, lno, p);
		continue;
	    }
	    bindings[k] = ev;
	}
	else {
	    printf("%s:%d: unknown command [%s]\n",
		   fname, lno, p);
	}
    }
}



const char *
npev_name(enum neopop_event ev)
{
    static const char *unknown="[unknown]";

    if (ev < 0 || ev >= NPEV_LAST)
	return unknown;

    return event_names[ev];
}



int
npev_parse(const char *name, char **end)
{
    return find_name(name, end, event_names, NPEV_LAST);
}



int
find_name(const char *p, char **end, const char **names, int n)
{
    int i, l, k, l2;

    k = l2 = -1;
    p += strspn(p, " \t");
    
    for (i=0; i<n; i++) {
	if (names[i] == NULL)
	    continue;

	l = strlen(names[i]);
	if (l>l2 && strncasecmp(p, names[i], l) == 0
	    && (p[l] == '\0' || isspace(p[l]))) {
	    k = i;
	    l2 = l;
	}
    }

    if (k != -1) {
	if (end)
	    *end = (char *)(p+l2);
    }
    return k;
}

