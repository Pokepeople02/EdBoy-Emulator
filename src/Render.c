#include "EdBoy.h"

/*	Initializes the SDL windows used by the emulator.
*	Window 0: EdBoy Emulator window. Renders Game Boy LCD contents.
*	Window 1: VRAM Tiles window. Visually renders tiled contents of Game Boy VRAM.
*	Returns 0 on successful full initialization. Otherwise, returns 1 on error.
*/
int InitEmuWindows( SDL_Window **windows ) {
	windows[0] = NULL;
	windows[1] = NULL;

	windows[0] = SDL_CreateWindow( 
		"EdBoy Emulator",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		GB_LCD_WIDTH,
		GB_LCD_HEIGHT,
		SDL_WINDOW_SHOWN
	);

	if ( windows[0] == NULL ) {
		eprintf( "Window 0 could not be created.\n" );
		return 1;
	}//end if

	dprintf( "Window 0 created.\n" );

	windows[1] = SDL_CreateWindow(
		"EdBoy VRAM Tiles",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		VRAM_WINDOW_WIDTH,
		VRAM_WINDOW_HEIGHT,
		SDL_WINDOW_SHOWN
	);

	if ( windows[0] == NULL ) {
		eprintf( "Window 1 could not be created.\n"  );
		return 1;
	}//end if

	dprintf( "Window 1 created.\n" );

	return 0;
}//end function InitEmuWindows

/*	Deinitializes the SDL windows used by the emulator.	*/
void DeinitEmuWindows( SDL_Window **windows ) {
	if ( windows[0] != NULL ) {
		SDL_DestroyWindow( windows[0] );
		dprintf( "Window 0 destroyed.\n" );
	}//end if
	
	if ( windows[1] != NULL ) {
		SDL_DestroyWindow( windows[1] );
		dprintf( "Window 1 destroyed.\n" );
	}//end if

	return;
}//end function DeinitEmuWindows