#include <stdbool.h>
#include <stdint.h>

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
void GB_Increment_Cycles_This_Frame( GameBoy *gb, unsigned cyclesIncrement ) {

	//For every T-State to progress by:
	for ( unsigned i = 0; i < cyclesIncrement; ++i ) {

		//Increment LY register every 456 cycles
		if ( gb->cycles / GB_DOTS_PER_SCANLINE > *( gb->io[0x44] ) ) {
			*( gb->io[0x44] ) = (*( gb->io[0x44] ) + 1) % GB_SCANLINES_PER_FRAME;
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
			case 00 : //In 1024 cycles
				gb->cyclesNextTIMA += 1024;
				break;
			case 01 : //In 16 cycles
				gb->cyclesNextTIMA += 16;
				break;
			case 02 : //In 64 cycles
				gb->cyclesNextTIMA += 64;
				break;
			case 03 : //In 256 cycles
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
	}//end if

	return;
}//end function GB_Increment_Cycles_This_Frame

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

/*	Performs read operation and returns the byte at the current program counter, then iterates the program counter.	*/
uint8_t GB_Get_Next_Byte( GameBoy *gb ) {
	uint8_t nextByte; //The byte at the current program counter

	nextByte = GB_Read( gb, gb->cpu.pc );
	gb->cpu.pc += 1;

	return nextByte;
}//end function GB_Get_Next_Byte

/*	Performs read operation on byte at the specified 16-bit address from the corresponding place in the emulated Game Boy's memory.
*	Iterates cycle count for current frame by 4 T-States for the read op.
*/
uint8_t GB_Read( GameBoy *gb, uint16_t addr ) {
	uint8_t byte; //The byte read by this operation

	//Boot ROM
	if ( addr < 0x100 && *( gb->io[0x50] ) == 0x00 ) {
		byte = gb->cpu.boot[addr];
		dprintf( "Read 0x%02X from boot ROM @ 0x%04X\n", byte, addr );
	}//end if

	//Lower ROM bank
	else if ( addr < 0x4000 ) {
		if ( gb->cart.rom0 && !gb->cart.isROM0Blocked ) byte = gb->cart.rom0[addr];
		else byte = 0xFF;
		dprintf( "Read 0x%02X from lower ROM bank @ 0x%04X\n", byte, addr );
	}//end else-if

	//Upper ROM bank
	else if ( addr < 0x8000 ) {
		if ( gb->cart.rom1 && !gb->cart.isROM1Blocked ) byte = gb->cart.rom1[addr - 0x4000];
		else byte = 0xFF;
		dprintf( "Read 0x%02X from upper ROM bank @ 0x%04X\n", byte, addr );
	}//end else-if

	//VRAM
	else if ( addr < 0xA000 ) {
		if ( !gb->isVRAMBlocked ) byte = gb->vram[addr - 0x8000];
		else byte = 0xFF;
		dprintf( "Read 0x%02X from VRAM @ 0x%04X\n", byte, addr );
	}//end else-if

	//External RAM
	else if ( addr < 0xC000 ) {
		if ( gb->cart.extram && !gb->cart.isExtRAMBlocked ) byte = gb->cart.extram[addr - 0xA000];
		else byte = 0xFF;
		dprintf( "Read 0x%02X from external RAM bank @ 0x%04X\n", byte, addr );
	}//end else-if

	//WRAM
	else if ( addr < 0xE000 ) {
		if ( !gb->isWRAMBlocked ) byte = gb->wram[addr - 0xC000];
		else byte = 0xFF;
		dprintf( "Read 0x%02X from WRAM @ 0x%04X\n", byte, addr );
	}//end else-if

	//Echo WRAM
	else if ( addr < 0xFE00 ) {
		if ( !gb->isWRAMBlocked ) byte = gb->wram[addr - 0xE000];
		else byte = 0xFF;
		dprintf( "Read 0x%02X from Echo WRAM @ 0x%04X\n", byte, addr );
	}//end else-if

	//OAM
	else if ( addr < 0xFEA0 ) {
		if ( !gb->cpu.ppu.isOAMBlocked ) byte = gb->cpu.ppu.oam[addr - 0xFE00];
		else byte = 0xFF;
		dprintf( "Read 0x%02X from OAM @ 0x%04X\n", byte, addr );

		//If read during Mode 2, trigger OAM Corruption Bug

	}//end else-if

	//Unusable Area
	else if ( addr < 0xFF00 ) {
		if ( !gb->cpu.ppu.isOAMBlocked ) byte = 0x00;
		else byte = 0xFF;
		eprintf( "Read from Unusable Area @ 0x%04X\n", addr );
		dprintf( "Read 0x%02X from Unusable Area @ 0x%04X\n", byte, addr );

		//If read during Mode 2, trigger OAM Corruption Bug

	}//end else-if

	//I/O registers
	else if ( addr < 0xFF80 ) {
		if ( gb->io[addr - 0xFF00] ) byte = *( gb->io[addr - 0xFF00] );
		else {
			byte = 0xFF;
			eprintf( "Read from unused I/O register @ 0x%04X\n", addr );
		}//end else
		dprintf( "Read 0x%02X from I/O register @ 0x%04X\n", byte, addr );
	}//end else-if

	//HRAM and IE register
	else {
		byte = gb->cpu.hram[addr - 0xFF80];
		dprintf( "Read 0x%02X from HRAM @ 0x%04X\n", byte, addr );
	}//end if-else

	//Increment cycles for read
	GB_Increment_Cycles_This_Frame(gb, 4 );

	return byte;
}//end function GB_Read