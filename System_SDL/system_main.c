#include <SDL.h>
#include "neopop.h"

void
system_message(char *vaMessage, ...)
{
    va_list vl;

    va_start(vl, vaMessage);
    vprintf(vaMessage, vl);
    va_end(vl);
}
         
void
system_VBL(void)
{
    /* update screen */
    return;
}

int
main(int argc, char *argv[])
{

    /* initialize SDL and create standard-sized surface */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
       fprintf(stderr, "cannot initialize SDL: %s\n", SDL_GetError());
       exit(1);
    }

    system_graphics_init();

    sleep(4);
    atexit(SDL_Quit);
}
