#include "EdBoy.h"

/* Runs the emulated Game Boy system for one frame. */
void GB_Run_Frame( GameBoy *gb, bool *isPressed ) {
	unsigned cycles = 0; //The number of cycles that have been run this frame

	//Enter main frame cycle
	while ( cycles < GB_CYCLES_PER_FRAME ) {

		//Handle next unhandled interrupt, if one exists

		//Decode and run the next instruction
		GB_Decode_Execute( gb, &cycles, isPressed );

	}//end for

	return;
}//end function GB_Run_Frame

/*	Does frame-stepping mode logic.
*	Handles toggling frame-stepped emulator input for the next frame.
*	Runs emulated Game Boy system for one frame upon pressing the frame-advance key.
*/
void DoFrameStepFrame( GameBoy *gb, const uint8_t *keyStates, bool *isPressed, bool *justPressed, bool *faJustPressed ) {

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
			GB_Run_Frame( gb, isPressed );
			*faJustPressed = true;
		}//end if
	}//end if
	else *faJustPressed = false;

	return;
}//end function DoFrameStepFrame

/*	Does full-speed mode logic.
*	Handles setting emulator input for the next frame.
*	Runs emulated Game Boy system for one frame, then delays execution to ensure proper emulation speed.
*/
void DoFullSpeedFrame( GameBoy *gb, const uint8_t *keyStates ) {
	bool isPressed[8]; //Stores whether a given key is pressed corresponding to a given button on the emulated Game Boy for the next frame

	//Get input for next frame
	for ( int i = GB_UP; i < GB_SELECT; ++i )
		isPressed[i] = keyStates[CTRL_SCANCODES[i]];

	//Do frame
	dprintf( "Doing full-speed frame.\n" );
	GB_Run_Frame( gb, isPressed );

	return;
}//end function DoFullSpeedFrame