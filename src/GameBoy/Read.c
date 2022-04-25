#include <stdbool.h>
#include <stdint.h>

#include "../EdBoy.h"

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
	GB_Cycle_T_States( gb, 4 );

	return byte;
}//end function GB_Read

/*	Performs read operation and returns the byte at the current program counter, then iterates the program counter.	*/
uint8_t GB_Get_Next_Byte( GameBoy *gb ) {
	uint8_t nextByte; //The byte at the current program counter

	nextByte = GB_Read( gb, gb->cpu.pc );
	gb->cpu.pc += 1;

	return nextByte;
}//end function GB_Get_Next_Byte