#include <stdlib.h>
#include <string.h>
#include <SDL_endian.h>

#include "Edboy.h"

/*	Initializes the emulated Game Boy system and allocates space for its memory.
*	ROM Banks and external RAM for inserted cartridge is allocated in GB_Load_Game()
*	Boot ROM memory is allocated in GB_Load_BootROM()
*/
void GB_Init( GameBoy *gb ) {
	//Allocate and configure WRAM
	gb->wram = malloc( 0x2000 );
	gb->isWRAMBlocked = false;
	dprintf( "WRAM allocated.\n" );

	//Allocate and configure VRAM
	gb->vram = malloc( 0x2000 );
	gb->isVRAMBlocked = false;
	dprintf( "VRAM allocated.\n" );

	//Allocate and configure OAM
	gb->cpu.ppu.oam = malloc( 0xA0 );
	gb->cpu.ppu.isOAMBlocked = false;
	dprintf( "OAM allocated.\n" );

	//Allocate and configure I/O registers
	memset( gb->io, 0, 0x80 * sizeof( uint8_t * ) );
	for ( int i = 0x00; i < 0x03; ++i ) 
		gb->io[i] = malloc( 1 ); //0xFF00 - 0xFF02
	for ( int i = 0x04; i < 0x08; ++i ) 
		gb->io[i] = malloc( 1 ); //0xFF04 - 0xFF07
	for ( int i = 0x10; i < 0x15; ++i ) 
		gb->io[i] = malloc( 1 ); //0xFF10 - 0xFF14
	for ( int i = 0x16; i < 0x27; ++i ) 
		gb->io[i] = malloc( 1 ); //0xFF16 - 0xFF26
	for ( int i = 0x30; i < 0x4C; ++i )
		gb->io[i] = malloc( 1 ); //0xFF30 - 0xFF4B
	gb->io[0x50] = malloc( 1 );	//0xFF50
	dprintf( "I/O Registers allocated.\n" );


	//Allocate and configure LCD
	for ( int i = 0; i < GB_LCD_HEIGHT; ++i )
		gb->lcd[i] = malloc( GB_LCD_WIDTH );
	gb->lcdBlankThisFrame = true;
	dprintf( "LCD buffer allocated.\n" );

	//Allocate HRAM
	gb->cpu.hram = malloc( 0x80 );
	dprintf( "HRAM allocated.\n" );

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

	//Configure PPU
	gb->cpu.ppu.fetcherX = 0;
	gb->cpu.ppu.fetcherY = 0;
	gb->cpu.ppu.bgFIFOTail = 0;
	gb->cpu.ppu.bgFIFOTail = 0;
	dprintf( "PPU fetcher and FIFO indices initialized.\n" );

	return;
}//end function GB_Init