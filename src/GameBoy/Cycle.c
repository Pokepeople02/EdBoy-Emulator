#include <stdbool.h>

#include "../EdBoy.h"

/* Runs the emulated Game Boy system for one frame. Returns true if user quit application prematurely via mid-frame pause on unknown opcode. */
bool GB_Run_Frame( GameBoy *gb, bool *isPressed ) {

	gb->isFrameOver = false;

	//Enter main ~70224 T-State cycle
	while ( !gb->isFrameOver ) {

		//Handle next unhandled interrupt, if one exists

		//Decode and run the next instruction, and quit prematurely if user requested quit during unknown-opcode-pause.
		if ( GB_Decode_Execute( gb, isPressed ) ) return true;

	}//end for

	return false;
}//end function GB_Run_Frame

/*	Increments the count of T-State cycles performed this frame and performs behaviors that occur every X T-State(s), such as:
*		Incrementing the LY register,
*		Comparing the LY and LYC registers,
*		Incrementing the DIV register,
*		Incrementing the TIMA register,
*		Progressing on an in-progress DMA Transfer
*		Initiating the PPU to tick for 1 dot every 1 T-State.
*/
void GB_Cycle_T_States( GameBoy *gb, unsigned cyclesIncrement ) {

	//For every T-State to progress by:
	for ( unsigned i = 0; i < cyclesIncrement; ++i ) {

		//Increment LY register every 456 cycles
		if ( gb->cycles / GB_DOTS_PER_SCANLINE > *( gb->io[0x44] ) ) {
			*( gb->io[0x44] ) = ( *( gb->io[0x44] ) + 1 ) % GB_SCANLINES_PER_FRAME;
			dprintf( "LY register now %d\n", *( gb->io[0x44] ) );
		}//end if

		//Compare LY and LYC registers. If equal and enabled, request LCD STAT interrupt
		if ( *( gb->io[0x44] ) == *( gb->io[0x45] ) ) {
			dprintf( "LY and LYC equal at %d. Requesting LCD STAT interrupt\n", *( gb->io[0x44] ) );
			//TODO Request LCD STAT interrupt
		}//end if

		//Increment DIV register every 256 cycles
		if ( gb->cycles == gb->cyclesNextDIV ) {
			*( gb->io[0x04] ) += 1;
			gb->cyclesNextDIV = ( gb->cycles + 256 ) % GB_CYCLES_PER_FRAME;
			dprintf( "DIV register now %d\n", *( gb->io[0x04] ) );
		}//end if

		//Increment TIMA register according to TAC register
		if ( ( *( gb->io[0x07] ) & 0x04 ) && ( gb->cycles == gb->cyclesNextTIMA ) ) {
			*( gb->io[0x05] ) += 1;
			dprintf( "TIMA register now %d\n", *( gb->io[0x05] ) );

			//If TIMA overflow, reset to TMA and request Timer interrupt
			if ( *( gb->io[0x05] ) == 0 ) {
				dprintf( "TIMA overflow. Requesting Timer interrupt\n" );
				//TODO Request Timer interrupt

				//Reset TIMA to TMA value
				*( gb->io[0x05] ) = *( gb->io[0x06] );
				dprintf( "TIMA reset to %d\n", *( gb->io[0x05] ) );
			}//end if

			//Update cyclesNextTIMA to know when to increment next
			gb->cyclesNextTIMA = gb->cycles;
			switch ( *( gb->io[0x07] ) & 0x03 ) {
			case 00: //In 1024 cycles
				gb->cyclesNextTIMA += 1024;
				break;
			case 01: //In 16 cycles
				gb->cyclesNextTIMA += 16;
				break;
			case 02: //In 64 cycles
				gb->cyclesNextTIMA += 64;
				break;
			case 03: //In 256 cycles
				gb->cyclesNextTIMA += 256;
				break;
			}//end switch
			gb->cyclesNextTIMA %= GB_CYCLES_PER_FRAME;

		}//end if

		//TODO Tick PPU


		//Increment cycles this frame
		gb->cycles += 1;
	}//end for

	//Check for whether next instruction is part of this frame
	if ( gb->cycles >= GB_CYCLES_PER_FRAME ) {
		gb->isFrameOver = true;
		gb->cycles %= GB_CYCLES_PER_FRAME;

		//TODO Clear OAM Search results on end of frame
	}//end if

	return;
}//end function GB_Increment_Cycles_This_Frame