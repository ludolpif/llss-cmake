#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
/*
 * This compilation unit isn't empty because including SDL_main.h include implementation code
 *  and define the correct program entrypoint, usually main() but can be WinMain() or so.
 *
 * SDL main callbacks implementations are in sdl-app-init.c, sdl-app-event.c, sdl-app-iterate.c
 */
