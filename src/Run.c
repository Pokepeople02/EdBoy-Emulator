#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

#include "EdBoy.h"

/*	Does frame-stepping mode logic.
*	Handles toggling frame-stepped emulator input for the next frame.
*	Runs emulated Game Boy system for one frame upon pressing the frame-advance key.
*	Returns true if termination requested prematurely mid-frame. Otherwise, returns false.
*/
bool Do_FrameStep_Frame( GameBoy *gb, const uint8_t *keyStates, bool *isPressed, bool *justPressed, bool *faJustPressed ) {

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

	//If frame-advance button pressed, do frame
	if ( keyStates[CTRL_FRAMESTEP_ADVANCE] ) {
		if ( !*faJustPressed ) {
			dprintf( "\nDoing frame-stepped frame:\n" );
			if ( GB_Run_Frame( gb, isPressed ) ) return true;
			*faJustPressed = true;
		}//end if
	}//end if
	else *faJustPressed = false;

	return false;
}//end function Do_FrameStep_Frame

/*	Does full-speed mode logic.
*	Handles setting emulator input for the next frame.
*	Runs emulated Game Boy system for one frame, then delays execution to ensure proper emulation speed.
*	Returns true if termination requested prematurely mid-frame. Otherwise, returns false.
*/
bool Do_FullSpeed_Frame( GameBoy *gb, const uint8_t *keyStates ) {
	bool isPressed[8]; //Stores whether a given key is pressed corresponding to a given button on the emulated Game Boy for the next frame

	//Get input for next frame
	for ( int i = GB_UP; i < GB_SELECT; ++i )
		isPressed[i] = keyStates[CTRL_SCANCODES[i]];

	//Do frame
	dprintf( "Doing full-speed frame.\n" );
	if( GB_Run_Frame( gb, isPressed ) ) return true;

	return false;
}//end function Do_FullSpeed_Frame

/*	Pauses mid-frame and waits for the frame-advance key to be pressed.
*	Does not accept other input, including closing the application windows.
*	Meant for temporary testing only.
*	Returns true if request was made to abort program via closing window mid-pause. Otherwise, returns false.
*/
bool Pause_On_Unknown_Opcode() {
	SDL_Event tempEvent; //Temporary pause event handler
	const uint8_t *currKeyStates; //Current keyboard key states during pause
	bool didContinue = false; //Set to true when user wants to continue executing frame
	bool alreadyPressing = false; //Whether the frame-advance key is already being pressed. Prevents premature advancing

	//Check status of frame-advance key
	currKeyStates = SDL_GetKeyboardState( NULL );
	if ( currKeyStates[CTRL_FRAMESTEP_ADVANCE] ) alreadyPressing = true;

	//Loop with no logging until told to break
	while ( !didContinue ) {

		//Poll for quit events until event queue is empty or told to quit mid-pause
		while ( SDL_PollEvent( &tempEvent ) )
			if ( tempEvent.type == SDL_WINDOWEVENT && tempEvent.window.event == SDL_WINDOWEVENT_CLOSE )
				return true;

		//Get state of keyboard
		currKeyStates = SDL_GetKeyboardState( NULL );

		//Update alreadyPressing if no longer pressing frame-advance key, and otherwise check if is now pressing frame-advance key
		if ( currKeyStates[CTRL_FRAMESTEP_ADVANCE] && !alreadyPressing ) didContinue = true;
		else if ( !currKeyStates[CTRL_FRAMESTEP_ADVANCE] && alreadyPressing ) alreadyPressing = false;
	}//end while

	return false;
}//end function Pause_On_Unknown_Opcode