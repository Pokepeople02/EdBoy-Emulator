#include "EdBoy.h"

/* Runs the emulated Game Boy system for one frame. */
void GB_Run_Frame( GameBoy *gb, bool *isPressed ) {
	unsigned cycles; //The number of cycles that have been run this frame

	//Initialize LY for this frame
	*( gb->io[0x44] ) = 0;
	dprintf( "Initializing LY to %X\n", *( gb->io[0x44] ) );

	//Enter main frame cycle
	for ( cycles = 0; cycles < 70224; ) {

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