#include "EdBoy.h"

//Defines SDL keyscan codes for keyboard keys linked to Game Boy buttons
const int CTRL_SCANCODES[] = {
	SDL_SCANCODE_W, //GB_UP
	SDL_SCANCODE_S, //GB_DOWN
	SDL_SCANCODE_A, //GB_LEFT
	SDL_SCANCODE_D, //GB_RIGHT
	SDL_SCANCODE_P, //GB_A
	SDL_SCANCODE_O, //GB_B
	SDL_SCANCODE_RETURN, //GB_START
	SDL_SCANCODE_LSHIFT //GB_SELECT
};

int main( int argc, char *argv[] ) {
	GameBoy gb; //Contains the total state of the emulated Game Boy system
	char *romPath; //File system path to the Game Boy ROM to be loaded
	char *bootromPath; //File system path to the Game Boy Boot ROM to be used
	SDL_Window *windows[2]; //Stores ptrs to SDL window structures. [0] = main emulator, [1] = VRAM BG Tiles
	SDL_Surface *surfaces[2]; //Stores ptrs to final SDL window surfaces.
	SDL_Event event; //SDL Event handler for closing windows and keyboard input
	const uint8_t *currKeyStates; //Stores current states of keyboard keys
	bool doFrameStep = true; //Toggles whether emulator is run in real time or frame step mode.
	bool isPressedFrameStep[8]; //Stores toggles for whether a given button on the Game Boy is to be press during a frame of frame-step mode
	bool justPressedFrameStep[8]; //Used for debouncing input toggles for emulator key presses during frame-step mode
	bool fsJustPressed = false; //Used for debouncing frame-step toggle
	bool faJustPressed = false; //Used for debouncing frame-advance button
	bool didQuit = false; //Stores whether user wishes to close the emulator

	//Get ROM and Boot ROM paths
	//TODO: Hardcoded for now
	romPath = "./roms/tetris.gb";
	bootromPath = "./roms/dmg_boot.bin";

	//Initialize SDL
	if ( SDL_Init( SDL_INIT_VIDEO ) ) {
		eprintf( "Unable to initialize SDL Video Subsystem: %s\n", SDL_GetError() );
		return 1;
	}//end if

	//Initialize windows
	if ( InitEmuWindows( windows ) ) { 
		eprintf( "Unable to initialize emulator windows: %s\n", SDL_GetError() );

		SDL_Quit();
		return 1;
	}//end if

	//Get surfaces
	surfaces[0] = SDL_GetWindowSurface( windows[0] );
	surfaces[1] = SDL_GetWindowSurface( windows[1] );

	//Initialize Game Boy system
	if ( GB_Init( &gb ) ) {
		eprintf( "An error occurred during emulated Game Boy system initialization.\n" );

		GB_Deinit( &gb );
		DeinitEmuWindows( windows );
		SDL_Quit();
		return 1;
	}//end if

	//Load Boot ROM or no-boot ROM alternative setup
	GB_Load_BootROM( &gb, bootromPath );

	//Load Game
	if ( GB_Load_Game( &gb, romPath ) ) {
		eprintf( "An error occurred while loading the game ROM file.\n" );

		GB_Deinit( &gb );
		DeinitEmuWindows( windows );
		SDL_Quit();
		return 1;
	}//end if

	//Initialize isPressed for frame skip on first frame
	for ( int i = 0; i < 8; ++i ) {
		isPressedFrameStep[i] = false;
		justPressedFrameStep[i] = false;
	}//end for

	//Main loop
	while ( !didQuit ) {

		//Poll for quit events until event queue is empty
		while ( SDL_PollEvent( &event ) && !didQuit )
			if ( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE  )
				didQuit = true;

		currKeyStates = SDL_GetKeyboardState( NULL );

		//Toggle frame-step mode, if needed
		if ( currKeyStates[CTRL_FRAMESTEP_TOGGLE] ) {
			if ( !fsJustPressed ) {
				doFrameStep = !doFrameStep;

				dprintf( "Frame-Step Mode is now %s\n", doFrameStep ? "On" : "Off" );

				for ( int i = 0; i < 8; ++i ) { 
					isPressedFrameStep[i] = false;
					justPressedFrameStep[i] = false;
				}//end for

				fsJustPressed = true;
			}//end if
		}//end if
		else fsJustPressed = false;

		//Perform frame as appropriate, check for mid-frame request for quit
		if ( doFrameStep ) {
			if ( DoFrameStepFrame( &gb, currKeyStates, isPressedFrameStep, justPressedFrameStep, &faJustPressed ) ) didQuit = true;
		}//end if
		else {
			if ( DoFullSpeedFrame( &gb, currKeyStates ) ) didQuit = true;
		}//end if-else

		//Update emulator surface contents

		//Update VRAM surface contents

		//Blit surfaces

		//Update windows

	}//end while

	//Deinitialize Game Boy system and loaded game
	GB_Deinit( &gb );

	//Deinit windows and quit SDL
	DeinitEmuWindows( windows );
	SDL_Quit();

	return 0;
}//end function main