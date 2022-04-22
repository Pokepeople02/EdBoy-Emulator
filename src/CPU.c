
#include "EdBoy.h"

/*	Decodes the next instruction at the current PC and calls the appropriate instruction function.
*	Passes number of T-States executed this frame to children function that would consume cycles to execute.
*	Passes Game Boy joypad buttons pressed this frame to children functions that could write to I/O registers for potential JOYP register update.
*	Notifies user via console and temporarily pauses execution upon encountering unknown or unimplemented opcode.
*	Returns true if unknown opcode encountered and user quits mid-pause by closing emulator window. Otherwise, returns false.
*/
bool GB_Decode_Execute( GameBoy *gb, unsigned *cycles, bool *isPressed ) {
	bool didQuitMidPause = false; //Whether user requested to quit mid-pause upon pausing execution for unknown opcode.
	uint8_t opcode; //The opcode of the encoded instruction

	opcode = GB_Get_Next_Byte( gb, cycles );

	//If first byte not 0xCB, decode opcode as normal
	if ( opcode != 0xCB ) {
		switch ( opcode ) {
		default: 
			eprintf( "Unknown or unimplemented opcode 0x%02X\n", opcode );
			didQuitMidPause = Temporary_Pause();
		}//end switch
	}//end if

	//If first byte 0xCB, decode as 0xCB-prefixed opcode
	else {
		opcode = GB_Get_Next_Byte( gb, cycles );

		switch ( opcode ) {
		default:
			eprintf( "Unknown or unimplemented opcode 0xCB%02X\n", opcode );
			didQuitMidPause = Temporary_Pause();
		}//end switch
	}//end if-else

	return didQuitMidPause;
}//end function GB_Decode_Execute

/*	Reads and returns the byte at the current program counter, then iterates the program counter.
*	Iterates cycles as appropriate for the read.
*/
uint8_t GB_Get_Next_Byte( GameBoy *gb, unsigned *cycles ) {
	uint8_t nextByte; //The byte at the current program counter

	nextByte = GB_Read( gb, cycles, gb->cpu.pc );
	gb->cpu.pc += 1;

	return nextByte;
}//end function GB_Get_Next_Byte

/*	Reads the byte at the specified 16-bit address from the corresponding place in the emulated Game Boy's memory.
*	Iterates cycles by 4 T-States for the read.
*/
uint8_t GB_Read( GameBoy *gb, unsigned *cycles, uint16_t addr ) {
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
	GB_Increment_Cycles_This_Frame(gb, cycles, 4 );

	return 0x0;
}//end function GB_Read

/*	Increments the count of T-State cycles performed this frame
*	Initiates behaviors that occur every T-State or every set number of T-State cycles, such as:
*		Incrementing the LY register,
*		Comparing the LY and LYC registers,
*		Incrementing the DIV register,
*		Incrementing the TIMA register, 
*		Progressing on an in-progress DMA Transfer
*	Also initiates the PPU to tick for one dot every T-State.
*/
void GB_Increment_Cycles_This_Frame( GameBoy *gb, unsigned *cyclesSoFar, unsigned incCycles ) {

	//For every T-State to progress by:
	for ( unsigned i = 0; i < incCycles; ++i ) {

	}//end for

	//Increment cycles
	*( cyclesSoFar ) += incCycles;

	return;
}//end function GB_Increment_Cycles_This_Frame