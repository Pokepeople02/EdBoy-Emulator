#include <stdio.h>
#include <SDL.h>

#include "EdBoy.h"

int main( int argc, char *argv[] ) {
	SDL_Window *windows[2]; //Stores ptrs to SDL window structures. [0] = main emulator, [1] = VRAM BG Tiles

	//Initialize SDL
	if ( SDL_Init(SDL_INIT_VIDEO) ) {
		eprintf( "Unable to initialize SDL Video Subsystem: %s\n", SDL_GetError() );
		return 1;
	}//end if

	//Initialize windows
	Init_Emu_Windows( windows );



	//Deinitialize and quit SDL
	Deinit_Emu_Windows( windows );
	SDL_Quit();

	return 0;
}//end function main