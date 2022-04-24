#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <SDL_endian.h>

#include "../Edboy.h"

/*	Initializes the emulated Game Boy system and allocates space for its memory.
*	ROM Banks and external RAM for inserted cartridge is allocated in GB_Load_Game()
*	Boot ROM memory is allocated in GB_Load_BootROM()
*	Returns 0 if all memory allocation successful. Else, returns 1 if unable.
*/
int GB_Init( GameBoy *gb ) {
	bool ableToAllocateIO = true; //Whether all I/O registers were able to be allocated.
	bool ableToAllocateLCD = true; //Whether all LCD scanlines were able to be allocated.

	//Allocate and configure WRAM
	gb->wram = malloc( 0x2000 );
	gb->isWRAMBlocked = false;

	if( gb->wram ) dprintf( "WRAM allocated.\n" );
	else {
		eprintf( "Unable to allocate WRAM.\n" );
		return 1;
	}//end if-else

	//Allocate and configure VRAM
	gb->vram = malloc( 0x2000 );
	gb->isVRAMBlocked = false;
	if ( gb->vram ) dprintf( "VRAM allocated.\n" );
	else {
		eprintf( "Unable to allocate VRAM.\n" );
		return 1;
	}//end if-else

	//Allocate and configure OAM
	gb->cpu.ppu.oam = malloc( 0xA0 );
	gb->cpu.ppu.isOAMBlocked = false;
	if ( gb->cpu.ppu.oam ) dprintf( "OAM allocated.\n" );
	else {
		eprintf( "Unable to allocate OAM.\n" );
		return 1;
	}//end if-else

	//Configure cartridge ROM/RAM blocks
	gb->cart.isROM0Blocked = false;
	gb->cart.isROM1Blocked = false;
	gb->cart.isExtRAMBlocked = false;

	//Allocate and configure I/O registers
	memset( gb->io, 0, 0x80 * sizeof( uint8_t * ) );

	for ( int i = 0x00; i < 0x03; ++i ) {
		gb->io[i] = malloc( 1 ); //0xFF00 - 0xFF02
		if ( !( gb->io[i] ) ) {
			ableToAllocateIO = false;
			break;
		}//end if
	}//end for

	for ( int i = 0x04; i < 0x08 && ableToAllocateIO; ++i ) {
		gb->io[i] = malloc( 1 ); //0xFF04 - 0xFF07
		if ( !( gb->io[i] ) ) {
			ableToAllocateIO = false;
			break;
		}//end if
	}//end for

	for ( int i = 0x0F; i < 0x15 && ableToAllocateIO; ++i ) {
		gb->io[i] = malloc( 1 ); //0xFF10 - 0xFF14
		if ( !( gb->io[i] ) ) {
			ableToAllocateIO = false;
			break;
		}//end if
	}//end for

	for ( int i = 0x16; i < 0x27 && ableToAllocateIO; ++i ) {
		gb->io[i] = malloc( 1 ); //0xFF16 - 0xFF26
		if ( !( gb->io[i] ) ) {
			ableToAllocateIO = false;
			break;
		}//end if
	}//end for

	for ( int i = 0x30; i < 0x4C && ableToAllocateIO; ++i ) {
		gb->io[i] = malloc( 1 ); //0xFF30 - 0xFF4B
		if ( !( gb->io[i] ) ) {
			ableToAllocateIO = false;
			break;
		}//end if
	}//end for

	gb->io[0x50] = malloc( 1 );	//0xFF50
	if ( !( gb->io[0x50] ) ) ableToAllocateIO = false;

	if( ableToAllocateIO ) dprintf( "I/O Registers allocated.\n" );
	else {
		eprintf( "Unable to allocate I/O Registers.\n" );
		return 1;
	}//end if-else

	//Allocate and configure LCD
	memset( gb->lcd, 0, GB_LCD_HEIGHT * sizeof( uint8_t * ) );
	for ( int i = 0; i < GB_LCD_HEIGHT; ++i ) {
		gb->lcd[i] = malloc( GB_LCD_WIDTH );
		if ( !( gb->lcd[i] ) ) {
			ableToAllocateLCD = false;
			break;
		}//end if
	}//end for
	gb->lcdBlankThisFrame = true;

	if( ableToAllocateLCD ) dprintf( "LCD buffer allocated.\n" );
	else {
		eprintf( "Unable to allocate LCD scanlines.\n" );
		return 1;
	}//end if-else

	//Allocate HRAM
	gb->cpu.hram = malloc( 0x80 );
	if ( gb->cpu.hram ) dprintf( "HRAM allocated.\n" );
	else {
		eprintf( "Unable to allocate HRAM.\n" );
		return 1;
	}//end if-else

	//Configure CPU registers
	gb->cpu.af = (uint16_t *)&( gb->cpu.regs[0] );
	gb->cpu.bc = (uint16_t *)&( gb->cpu.regs[2] );
	gb->cpu.de = (uint16_t *)&( gb->cpu.regs[4] );
	gb->cpu.hl = (uint16_t *)&( gb->cpu.regs[6] );
	dprintf( "CPU 16b register pair references set.\n" );

	if ( SDL_BYTEORDER == SDL_BIG_ENDIAN ) {
		gb->cpu.a = &( gb->cpu.regs[0] );
		gb->cpu.f = &( gb->cpu.regs[1] );
		gb->cpu.b = &( gb->cpu.regs[2] );
		gb->cpu.c = &( gb->cpu.regs[3] );
		gb->cpu.d = &( gb->cpu.regs[4] );
		gb->cpu.e = &( gb->cpu.regs[5] );
		gb->cpu.h = &( gb->cpu.regs[6] );
		gb->cpu.l = &( gb->cpu.regs[7] );
		dprintf( "CPU 8b register references set for Big Endian.\n" );
	}//end if
	else {
		gb->cpu.a = &( gb->cpu.regs[1] );
		gb->cpu.f = &( gb->cpu.regs[0] );
		gb->cpu.b = &( gb->cpu.regs[3] );
		gb->cpu.c = &( gb->cpu.regs[2] );
		gb->cpu.d = &( gb->cpu.regs[5] );
		gb->cpu.e = &( gb->cpu.regs[4] );
		gb->cpu.h = &( gb->cpu.regs[7] );
		gb->cpu.l = &( gb->cpu.regs[6] );
		dprintf( "CPU 8b register references set for Little Endian.\n" );
	}//end if-else

	//Configure PPU fetcher
	gb->cpu.ppu.fetcherX = 0;
	gb->cpu.ppu.fetcherY = 0;
	gb->cpu.ppu.bgFIFOTail = 0;
	gb->cpu.ppu.bgFIFOTail = 0;
	dprintf( "PPU fetcher and FIFO indices initialized.\n" );

	//Configure PPU OAM scan results buffer
	memset( gb->cpu.ppu.oamScanResults, 0, 10 * sizeof( uint8_t * ) );
	dprintf( "PPU OAM Scan results buffer initialized.\n" );

	//Configure I/O Registers
	*( gb->io[0x04] ) = 0x00; //DIV
	*( gb->io[0x05] ) = 0x00; //TIMA
	*( gb->io[0x07] ) = 0x00; //TAC
	gb->cpu.hram[0x7F] = 0x00; //IE

	//Configure I/O Register helper values
	gb->cyclesNextDIV = 255;
	gb->cyclesNextTIMA = 0;

	//Set unloaded cartridge ROM/RAM banks to NULL
	gb->cart.rom0 = NULL;
	gb->cart.rom1 = NULL;
	gb->cart.extram = NULL;

	return 0;
}//end function GB_Init

/* Frees memory allocated for the emulated Game Boy system and the loaded game. */
void GB_Deinit( GameBoy *gb ) {
	//Free WRAM
	free( gb->wram );
	dprintf( "Freed WRAM.\n" );

	//Free VRAM
	free( gb->vram );
	dprintf( "Freed VRAM.\n" );

	//Free OAM
	free( gb->cpu.ppu.oam );
	dprintf( "Freed OAM.\n" );

	//Free IO Registers
	for ( int i = 0; i < 0x80; ++i )
		if ( gb->io[i] ) free( gb->io[i] );
	dprintf( "Freed IO Registers, if allocated.\n" );

	//Free LCD
	for ( int i = 0; i < GB_LCD_HEIGHT; ++i )
		if ( gb->lcd[i] ) free( gb->lcd[i] );
	dprintf( "Freed LCD scanlines, if allocated.\n" );

	//Free HRAM
	free( gb->cpu.hram );
	dprintf( "Freed HRAM.\n" );

	//Free Boot ROM
	if ( gb->cpu.boot ) free( gb->cpu.boot );
	dprintf( "Freed Boot ROM, if allocated.\n" );

	//Free ROM banks
	if ( gb->cart.rom0 ) free( gb->cart.rom0 );
	dprintf( "Freed lower ROM bank, if allocated.\n" );

	if ( gb->cart.rom1 ) free( gb->cart.rom1 );
	dprintf( "Freed upper ROM bank, if allocated.\n" );

	//Free external RAM
	if ( gb->cart.extram ) free( gb->cart.extram );
	dprintf( "Freed external RAM, if allocated.\n" );

	return;
}//end function GB_Deinit