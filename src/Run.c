#include "EdBoy.h"

/* Runs the emulated Game Boy system for one frame. */
void GB_Run_Frame( GameBoy *gb, bool *isPressed ) {
	unsigned cycles; //The number of cycles that have been run this frame

	//Initialize LY for this frame
	*( gb->io[0x44] ) = 0;
	dprintf( "Initializing LY to %X\n", *( gb->io[0x44] ) );

	//Enter main frame cycle
	for ( cycles = 0; cycles < GB_CYCLES_PER_FRAME; ) {

		//Check whether LYC matches LY. If so, request LCD STAT interrupt

		//Run the next instruction


		//Update LY register when the next scanline is reached
		if ( cycles / GB_DOTS_PER_SCANLINE > *( gb->io[0x44] ) ) {
			*( gb->io[0x44] ) += 1;
			dprintf( "Incremented LY. Now on scaline %d\n", *( gb->io[0x44] ) );
		}//end if

		cycles++;
	}//end for

	return;
}//end function GB_Run_Frame

/*	Does frame-stepping mode logic.
*	Handles toggling frame-stepped emulator input for the next frame.
*	Runs emulated Game Boy system for one frame upon pressing the frame-advance key.
*/
void DoFrameStepFrame( GameBoy *gb, int *cycleOverflow, uint8_t *keyStates, bool *isPressed, bool *justPressed, bool *faJustPressed ) {

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
			GB_Run_Frame( gb, cycleOverflow, isPressed );
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
void DoFullSpeedFrame( GameBoy *gb, int *cycleOverflow, uint8_t *keyStates ) {
	bool isPressed[8]; //Stores whether a given key is pressed corresponding to a given button on the emulated Game Boy for the next frame

	//Get input for next frame
	for ( int i = GB_UP; i < GB_SELECT; ++i )
		isPressed[i] = keyStates[CTRL_SCANCODES[i]];

	//Do frame
	dprintf( "Doing full-speed frame.\n" );
	GB_Run_Frame( gb, cycleOverflow, isPressed );

	return;
}//end function DoFullSpeedFrame