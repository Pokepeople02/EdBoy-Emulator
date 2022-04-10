#include <stdio.h>
#include <SDL.h>

#include "EdBoy.h"

/*	Initializes the SDL windows used by the emulator.
*	Window 0: EdBoy Emulator window. Renders Game Boy LCD contents.
*	Window 1: VRAM BG Tiles window. Renders contents of Game Boy VRAM used for BG tiles.
*	Returns 0 on successful full initialization. Otherwise, returns 1 on error.
*/
int Init_Emu_Windows( SDL_Window **windows ) {
	windows[0] = NULL;
	windows[1] = NULL;

	windows[0] = SDL_CreateWindow( 
		"EdBoy Emulator",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		GB_LCD_WIDTH * ED_WINDOW_SCALE,
		GB_LCD_HEIGHT * ED_WINDOW_SCALE,
		SDL_WINDOW_SHOWN
	);

	if ( windows[0] == NULL ) {
		eprintf( "Window 0 could not be created: %s\n", SDL_GetError() );
		return 1;
	}//end if

	dprintf( "Window 0 created.\n" );

	windows[1] = SDL_CreateWindow(
		"EdBoy VRAM BG Tiles",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		GB_LCD_WIDTH * ED_WINDOW_SCALE,
		GB_LCD_HEIGHT * ED_WINDOW_SCALE,
		SDL_WINDOW_SHOWN
	);

	if ( windows[0] == NULL ) {
		eprintf( "Window 1 could not be created: %s\n", SDL_GetError() );
		return 1;
	}//end if

	dprintf( "Window 1 created.\n" );

	return 0;
}//end function Init_Emu_Windows

/*	Deinitializes the SDL windows used by the emulator.	*/
void Deinit_Emu_Windows( SDL_Window **windows ) {
	SDL_DestroyWindow( windows[0] );
	dprintf( "Window 0 destroyed.\n" );

	SDL_DestroyWindow( windows[1] );
	dprintf( "Window 1 destroyed.\n" );

	return;
}//end function Deinit_Emu_Windows