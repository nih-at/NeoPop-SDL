/* $NiH: system_input.c,v 1.5 2002/12/03 13:09:37 wiz Exp $ */

#include "neopop-SDL.h"

#define INPUT_MASK_UP		0x1
#define INPUT_MASK_DOWN		0x2
#define INPUT_MASK_LEFT		0x4
#define INPUT_MASK_RIGHT	0x8
#define INPUT_MASK_BUTTON_A	0x10
#define INPUT_MASK_BUTTON_B	0x20
#define INPUT_MASK_OPTION	0x40

void
system_input_update(void)
{
    SDL_Event evt;

    while(SDL_PollEvent(&evt)) {
	switch(evt.type) {
	case SDL_KEYDOWN:
	    switch(evt.key.keysym.sym) {
	    case SDLK_F3:
		system_state_load();
		break;
	    case SDLK_F4:
		system_state_save();
		break;
	    case SDLK_F12:
		system_screenshot();
		break;
	    case SDLK_MINUS:
	    case SDLK_KP_MINUS:
		if (system_frameskip_key > 1)
		    system_frameskip_key--;
		break;
	    case SDLK_EQUALS:
	    case SDLK_KP_EQUALS:
	    case SDLK_PLUS:
	    case SDLK_KP_PLUS:
		if (system_frameskip_key < 7)
		    system_frameskip_key++;
		break;
	    case SDLK_1:
		graphics_mag_req = 1;
		break;
	    case SDLK_2:
		graphics_mag_req = 2;
		break;
	    case SDLK_3:
		graphics_mag_req = 3;
		break;
	    case SDLK_b:
		graphics_mag_smooth = graphics_mag_smooth ? 0 : 1;
		break;
	    case SDLK_f:
		system_graphics_fullscreen_toggle();
		break;
	    case SDLK_i:
		ram[0x6F82] |= INPUT_MASK_UP;
		break;
	    case SDLK_k:
		ram[0x6F82] |= INPUT_MASK_DOWN;
		break;
	    case SDLK_j:
		ram[0x6F82] |= INPUT_MASK_LEFT;
		break;
	    case SDLK_l:
		ram[0x6F82] |= INPUT_MASK_RIGHT;
		break;
	    case SDLK_m:
		mute = mute ? 0: 1;
		break;
	    case SDLK_LSHIFT:
	    case SDLK_RSHIFT:
	    case SDLK_a:
		ram[0x6F82] |= INPUT_MASK_BUTTON_A;
		break;
	    case SDLK_LCTRL:
	    case SDLK_RCTRL:
	    case SDLK_s:
		ram[0x6F82] |= INPUT_MASK_BUTTON_B;
		break;
	    case SDLK_TAB:
	    case SDLK_d:
		ram[0x6F82] |= INPUT_MASK_OPTION;
		break;
	    case SDLK_ESCAPE:
		do_exit = 1;
		break;
	    default:
		/* ignore */
		break;
	    }
	    break;
	case SDL_KEYUP:
	    switch(evt.key.keysym.sym) {
	    case SDLK_i:
		ram[0x6F82] &= ~INPUT_MASK_UP;
		break;
	    case SDLK_k:
		ram[0x6F82] &= ~INPUT_MASK_DOWN;
		break;
	    case SDLK_j:
		ram[0x6F82] &= ~INPUT_MASK_LEFT;
		break;
	    case SDLK_l:
		ram[0x6F82] &= ~INPUT_MASK_RIGHT;
		break;
	    case SDLK_LSHIFT:
	    case SDLK_RSHIFT:
	    case SDLK_a:
		ram[0x6F82] &= ~INPUT_MASK_BUTTON_A;
		break;
	    case SDLK_LCTRL:
	    case SDLK_RCTRL:
	    case SDLK_s:
		ram[0x6F82] &= ~INPUT_MASK_BUTTON_B;
		break;
	    case SDLK_TAB:
	    case SDLK_d:
		ram[0x6F82] &= ~INPUT_MASK_OPTION;
		break;
	    default:
		/* ignore */
		break;
	    }
	    break;
	case SDL_QUIT:
	    do_exit = 1;
	    break;
	default:
	    /* ignore */
	    break;
	}

    }
}
