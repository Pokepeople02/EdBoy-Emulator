#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <SDL.h>

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
	uint8_t *currKeyStates; //Stores current states of keyboard keys
	bool doFrameStep = true; //Toggles whether emulator is run in real time or frame step mode.
	bool isPressedFrameStep[8]; //Stores toggles for whether a given button on the Game Boy is to be press during a frame of frame-step mode
	bool justPressedFrameStep[8]; //Used for debouncing input toggles for emulator key presses during frame-step mode
	bool fsJustPressed = false; //Used for debouncing frame-step toggle
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
		eprintf( "Memory for emulated Game Boy system unable to be fully allocated.\n" );

		GB_Deinit( &gb );
		DeinitEmuWindows( windows );
		SDL_Quit();
		return 1;
	}//end if

	//Load Boot ROM
	GB_Load_BootROM( &gb, bootromPath );

	//Load Game


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

		//Perform frame as appropriate
		if ( doFrameStep )
			DoFrameStepFrame( currKeyStates, isPressedFrameStep, justPressedFrameStep );
		else
			DoFullSpeedFrame( currKeyStates );

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

/*	Does frame-stepping mode logic. 
*	Handles toggling frame-stepped emulator input for the next frame.
*	Runs emulated Game Boy system for one frame upon pressing the frame-advance key.
*/
void DoFrameStepFrame( uint8_t *keyStates, bool *isPressed, bool *justPressed ) {

	//Update frame-step control toggles
	for ( int i = GB_UP; i < GB_SELECT; ++i ) {
		if ( keyStates[CTRL_SCANCODES[i]] ) {

			if ( !justPressed[i] ) {
				isPressed[i] = !isPressed[i];
				justPressed[i] = true;

				dprintf( "Toggled button %d for next frame to %s\n", i, isPressed[i] ? "On" : "Off" );
			}//end if

		}//end if
		else justPressed[i] = false;
	}//end for

	//Update JOYP register for input

	//Do frame

	return;
}//end function DoFrameStepFrame

/*	Does full-speed mode logic.
*	Handles setting emulator input for the next frame.
*	Runs emulated Game Boy system for one frame, then delays execution to ensure proper emulation speed.
*/
void DoFullSpeedFrame( uint8_t *keyStates ) {
	bool isPressed[8]; //Stores whether a given key is pressed corresponding to a given button on the emulated Game Boy for the next frame

	//Get input for next frame
	for ( int i = GB_UP; i < GB_SELECT; ++i )
		isPressed[i] = keyStates[CTRL_SCANCODES[i]];

	//Update JOYP register for input

	//Do frame

	return;
}//end function DoFullSpeedFrame