#include <stdio.h>
#include <stdlib.h>

#include "../Edboy.h"

/*	TEMPORARY IMPLEMENTATION: Lacks MBC, external RAM support of any kind
*	Attempts to allocate memory for cartridge ROM/RAM banks and load contents from file at the supplied path.
*	If successful, loads the first 0x4000 bytes into lower ROM bank, then the next 0x4000 bytes into upper ROM bank.
*	If unsuccessful, leaves uninitialized data in ROM banks to emulate unconnected cartridge pins.
*	Leaves uninitialized data for external RAM to emulate lack of external RAM.
*	Returns 0 if all allocations successful. Else, returns 1 if unable.
*/
int GB_Load_Game( GameBoy *gb, char *path ) {
	FILE *romFile = NULL; //Points to ROM file, if successfully loaded.

	//Open file
	fopen_s( &romFile, path, "rb" );

	if ( romFile ) {
		dprintf( "Successfully opened cartridge ROM file.\n" );
	}//end if

	//If able to open file and allocate, load ROM contents
	if ( romFile ) {

		//Allocate ROM banks
		gb->cart.rom0 = malloc( 0x4000 );
		if ( !( gb->cart.rom0 ) ) {
			eprintf( "Unable to allocate memory for ROM bank 0.\n" );

			fclose( romFile );
			return 1;
		}//end if
		else dprintf( "ROM bank 0 allocated.\n" );

		gb->cart.rom1 = malloc( 0x4000 );
		if ( !( gb->cart.rom1 ) ) {
			eprintf( "Unable to allocate memory for ROM bank 1.\n" );

			fclose( romFile );
			return 1;
		}//end if
		else dprintf( "ROM bank 1 allocated.\n" );

		//Attempt to load ROM Bank 0
		fread( gb->cart.rom0, 1, 0x4000, romFile );
		dprintf( "ROM Bank 0 loaded. First byte test: 0x%02X\n", gb->cart.rom0[0] );

		//Attempt to load ROM Bank 1
		if ( fread( gb->cart.rom1, 1, 0x4000, romFile ) > 0 ) {
			dprintf( "ROM Bank 1 loaded. First byte test: 0x%02X\n", gb->cart.rom1[0] );
		}//end if

		//Disable external RAM
		gb->cart.extram = NULL;
	}//end if
	//Else, unable to load ROM. Emulate empty cartridge slot.
	else {
		eprintf( "Unable to load any cartridge ROM. Emulating empty cartridge slot instead.\n" );

		//Disable ROM banks and external RAM
		gb->cart.rom0 = NULL;
		gb->cart.rom1 = NULL;
		gb->cart.extram = NULL;
	}//end if-else

	if ( romFile )
		fclose( romFile );

	return 0;
}//end function GB_Load_Game

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

		//Initialize CPU registers
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
		dprintf( "Initialized CPU registers to post-boot ROM state.\n" );

		//Initialize other I/O registers
		*( gb->io[0x00] ) = 0xCF; //P1
		*( gb->io[0x01] ) = 0x00; //SB
		*( gb->io[0x02] ) = 0x7E; //SC
		*( gb->io[0x04] ) = 0xAB; //DIV
		*( gb->io[0x05] ) = 0x00; //TIMA
		*( gb->io[0x06] ) = 0x00; //TMA
		*( gb->io[0x07] ) = 0xF8; //TAC
		*( gb->io[0x0F] ) = 0xE1; //IF
		//Skipping sound registers
		*( gb->io[0x40] ) = 0x91; //LCDC
		*( gb->io[0x41] ) = 0x85; //STAT
		*( gb->io[0x42] ) = 0x00; //SCY
		*( gb->io[0x43] ) = 0x00; //SCX
		*( gb->io[0x44] ) = 0x00; //LY
		*( gb->io[0x45] ) = 0x00; //LYC
		*( gb->io[0x46] ) = 0xFF; //DMA
		*( gb->io[0x47] ) = 0xFC; //BGP
		//OBP0 and OBP1 left uninitialized
		*( gb->io[0x4A] ) = 0x00; //WY
		*( gb->io[0x4B] ) = 0xFC; //WX
		*( gb->io[0x50] ) = 0x01; //BANK
		gb->cpu.hram[0x7F] = 0x00; //IE
		dprintf( "Initialized I/O registers to post-boot ROM state.\n" );
	}//end else

	//Close file
	if ( bootromFile ) {
		fclose( bootromFile );
		dprintf( "Closed boot ROM file.\n" );
	}//end if

	return;
}//end function GB_Load_BootROM