#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL_endian.h>

#include "Edboy.h"

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

	for ( int i = 0x10; i < 0x15 && ableToAllocateIO; ++i ) {
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

	//Configure PPU
	gb->cpu.ppu.fetcherX = 0;
	gb->cpu.ppu.fetcherY = 0;
	gb->cpu.ppu.bgFIFOTail = 0;
	gb->cpu.ppu.bgFIFOTail = 0;
	dprintf( "PPU fetcher and FIFO indices initialized.\n" );

	//Set unloaded cartridge ROM/RAM banks to NULL
	gb->cart.rom0 = NULL;
	gb->cart.rom1 = NULL;
	gb->cart.extram = NULL;

	return 0;
}//end function GB_Init

/*	Attempts to allocate memory and load boot ROM contents from file at the supplied path.
*	If successful:
*		Loads memory for boot ROM from file.
*		Sets the Boot ROM Disable register (0xFF50) to 0.
*		Sets the Program Counter to 0x0.
*	If unsuccessful:
*		Sets the Boot ROM Disable register to 1.
*		Initializes register values to their post-Boot ROM values.
*/
void GB_Load_BootROM( GameBoy *gb, char *path ) {
	FILE *bootromFile = NULL; //Points to bootrom file, if successfully loaded
	long fileSize = 0; //Size of loaded boot ROM file in bytes

	//Pre-allocate boot ROM
	gb->cpu.boot = malloc( 0x100 );
	if ( !( gb->cpu.boot ) ) eprintf( "Unable to allocate memory for boot ROM.\n" );
	else dprintf( "Boot ROM allocated.\n" );

	fopen_s( &bootromFile, path, "rb" );

	//Get file size
	if ( bootromFile ) {
		fseek( bootromFile, 0, SEEK_END );
		fileSize = ftell( bootromFile );
		rewind( bootromFile );

		dprintf( "Successfully opened boot ROM file (0x%lX bytes).\n", fileSize );
	}//end if

	//If able to open, load bootrom and prepare for execution
	if ( gb->cpu.boot && bootromFile && fileSize == 0x100 ) {

		//Load boot ROM
		fread( gb->cpu.boot, 1, 0x100, bootromFile );
		dprintf( "Boot ROM loaded. First byte test: 0x%X\n", gb->cpu.boot[0] );

		//Set BANK register
		*( gb->io[0x50] ) = 0;
		dprintf( "BANK register set to %X\n", *( gb->io[0x50] ) );

		//Initialize PC register
		gb->cpu.pc = 0x0;
		dprintf( "PC set to %04X\n", gb->cpu.pc );

	}//end if
	//Unable to load bootrom, prepare for post-bootrom execution
	else {
		gb->cpu.boot = NULL;
		eprintf( "Unable to load boot ROM. Loading alternative setup.\n" );

		//Set BANK register
		*( gb->io[0x50] ) = 1;
		dprintf( "BANK register set to %X\n", *( gb->io[0x50] ) );

		//Initialize registers
		*( gb->cpu.a ) = 0x01;
		*( gb->cpu.f ) = 0x00;
		*( gb->cpu.b ) = 0x00;
		*( gb->cpu.c ) = 0x13;
		*( gb->cpu.d ) = 0x00;
		*( gb->cpu.e ) = 0xD8;
		*( gb->cpu.h ) = 0x01;
		*( gb->cpu.l ) = 0x4D;
		gb->cpu.pc = 0x0100;
		gb->cpu.sp = 0xFFFE;
		dprintf( "Initialized registers to post-boot ROM state.\n" );
		dprintf( "AF: %04X, BC: %04X, DE: %04X, HL: %04X\n", *( gb->cpu.af ), *( gb->cpu.bc ), *( gb->cpu.de ), *( gb->cpu.hl ) );
		dprintf( "PC: %04X, SP: %04X\n", gb->cpu.pc, gb->cpu.sp );
	}//end else

	//Close file
	if ( bootromFile ) {
		fclose( bootromFile );
		dprintf( "Closed boot ROM file.\n" );
	}//end if

	return;
}//end function GB_Load_BootROM

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

/*	TEMPORARY IMPLEMENTATION: Lacks MBC, external RAM support of any kind
*	Attempts to allocate memory for cartridge ROM/RAM banks and load contents from file at the supplied path.
*	If successful, loads the first 0x4000 bytes into lower ROM bank, then the next 0x4000 bytes into upper ROM bank.
*	If unsuccessful, leaves uninitialized data in ROM banks to emulate unconnected cartridge pins.
*	Leaves uninitialized data for external RAM to emulate lack of external RAM.
*	Returns 0 if all allocations successful. Else, returns 1 if unable.
*/
int GB_Load_Game( GameBoy *gb, char *path ) {
	FILE *romFile = NULL; //Points to ROM file, if successfully loaded.
	long fileSize = 0; //Size of loaded ROM file in bytes

	//Allocate ROM banks
	gb->cart.rom0 = malloc( 0x4000 );
	if ( !( gb->cart.rom0 ) ) {
		eprintf( "Unable to allocate memory for ROM bank 0.\n" );
		return 1;
	}//end if
	else dprintf( "ROM bank 0 allocated.\n" );

	gb->cart.rom1 = malloc( 0x4000 );
	if ( !( gb->cart.rom1 ) ) {
		eprintf( "Unable to allocate memory for ROM bank 1.\n" );
		return 1;
	}//end if
	else dprintf( "ROM bank 1 allocated.\n" );

	//Allocate external RAM bank
	gb->cart.extram = malloc( 0x2000 );
	if ( !( gb->cart.extram ) ) {
		eprintf( "Unable to allocate memory for external RAM bank.\n" );
		return 1;
	}//end if
	else dprintf( "External RAM allocated.\n" );
	gb->cart.hasExtRam = false;

	//Open file and get file size
	fopen_s( &romFile, path, "rb" );

	if ( romFile ) {
		fseek( romFile, 0, SEEK_END );
		fileSize = ftell( romFile );
		rewind( romFile );

		dprintf( "Successfully opened cartridge ROM file (0x%lX bytes).\n", fileSize );
	}//end if

	//If able to allocate, open file, and file meets min size req, load ROM contents
	if ( romFile && fileSize >= 0x8000 ) {

		//If did not read 0x4000 bytes, error during read
		if ( fread( gb->cart.rom0, 1, 0x4000, romFile ) != 0x4000 ) {
			eprintf( "Read error during read for ROM bank 0.\n" );
			fclose( romFile );
			return 1;
		}//end if
		dprintf( "ROM Bank 0 loaded. First byte test: 0x%02X\n", gb->cart.rom0[0] );

		if ( fread( gb->cart.rom1, 1, 0x4000, romFile ) != 0x4000 ) {
			eprintf( "Read error during read for ROM bank 1.\n" );
			fclose( romFile );
			return 1;
		}//end if
		dprintf( "ROM Bank 1 loaded. First byte test: 0x%02X\n", gb->cart.rom1[0] );

	}//end if
	//Else, unable to load ROM. Emulate empty cartridge slot.
	else {
		eprintf( "Unable to load cartridge ROM. Emulating empty cartridge slot instead.\n" );
	}//end if-else

	if( romFile )
		fclose( romFile );

	return 0;
}//end 