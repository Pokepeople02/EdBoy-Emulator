#include <stdbool.h>
#include <stdint.h>

#include "../EdBoy.h"

/*	Decodes the next instruction at the current PC and calls the appropriate instruction function.
*	Passes Game Boy joypad buttons pressed this frame to children functions that could write to I/O registers for potential JOYP register update.
*	Notifies user via console and temporarily pauses execution upon encountering unknown or unimplemented opcode.
*	Returns true if unknown opcode encountered and user quits mid-pause by closing emulator window. Otherwise, returns false.
*/
bool GB_Decode_Execute( GameBoy *gb, bool *isPressed ) {
	bool didQuitMidPause = false; //Whether user requested to quit mid-pause upon pausing execution for unknown opcode.
	uint8_t opcode; //The opcode of the encoded instruction

	opcode = GB_Get_Next_Byte( gb );

	//If first byte not 0xCB, decode opcode as normal
	if ( opcode != 0xCB ) {
		switch ( opcode ) {
		default:
			eprintf( "Unknown or unimplemented opcode 0x%02X\n", opcode );
			didQuitMidPause = Pause_On_Unknown_Opcode();
		}//end switch
	}//end if

	//If first byte 0xCB, decode as 0xCB-prefixed opcode
	else {
		opcode = GB_Get_Next_Byte( gb );

		switch ( opcode ) {
		default:
			eprintf( "Unknown or unimplemented opcode 0xCB%02X\n", opcode );
			didQuitMidPause = Pause_On_Unknown_Opcode();
		}//end switch
	}//end if-else

	return didQuitMidPause;
}//end function GB_Decode_Execute