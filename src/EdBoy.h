#pragma once

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*	Game Boy Constants	*/
#define GB_LCD_HEIGHT 144 //Game Boy LCD screen pixel height
#define GB_LCD_WIDTH 160 //Game Boy LCD screen pixel width
#define GB_DOTS_PER_SCANLINE 456 //Number of PPU dots per Game Boy LCD scanline
#define GB_CYCLES_PER_FRAME 70224 //Number of CPU cycles and PPU dots in one DMG Game Boy frame
#define GB_SCANLINES_PER_FRAME 154 //Number of scanlines in one frame, including non-rendering VBlank scanlines

/*	Emulator Constants	*/
#define VRAM_WINDOW_HEIGHT 128 //Unscaled VRAM display window pixel width (24 tiles wide * 8 px per tile)
#define VRAM_WINDOW_WIDTH 192 //Unscaled VRAM display window pixel hight (16 tiles high * 8 px per tile)

/* Emulator Controls */
#define CTRL_FRAMESTEP_TOGGLE SDL_SCANCODE_K //Toggles frame-step/full-speed modes
#define CTRL_FRAMESTEP_ADVANCE SDL_SCANCODE_SPACE //Advances one frame in frame-step mode

/*	Debug	*/
#define DEBUG 1 //Debug logging signifier constant
#ifdef DEBUG
#define dprintf(...) printf(__VA_ARGS__) //Debug print function
#else
#define dprintf(...) 
#endif

/*	Shorthand	*/
#define eprintf(...) fprintf( stderr, __VA_ARGS__ ) //stderr print function shorthand

/*	Definitions	*/
//Defines the attributes of a pixel currently in one of the PPU's Pixel FIFOs
struct GB_FIFOPixel {
	uint8_t colorIndex; //Pixel color as an index into the appropriate Palette register
	uint8_t palette; //Value of Palette Number bit. For sprites only.
	uint8_t priority; //Value of OBJ-to-BG Priority bit. For sprites only.
};

//Defines the state of the emulated Game Boy's PPU (Picture Processing Unit)
struct GB_PictureProcessor {
	uint8_t *oam; //160 B Object Attribute Memory
	bool isOAMBlocked; //Whether OAM access is currently blocked

	uint8_t fetcherX; //X-Coordinate of PPU Pixel Fetcher
	uint8_t fetcherY; //Y-Coordinate of PPU Pixel Fetcher

	struct GB_FIFOPixel bgFIFO[16]; //Contents of BG Pixel FIFO
	struct GB_FIFOPixel oamFIFO[16]; //Contents of sprite/OAM Pixel FIFO

	uint8_t bgFIFOTail; //First free index of BG Pixel FIFO
	uint8_t oamFIFOTail; //First free index of OAM Pixel FIFO

	uint8_t *oamScanResults[10]; //Pointers to sprites in OAM found during Mode 2 for the current scanline
};

//Defines the state of the emulated Game Boy's CPU/System on a Chip
struct GB_Processor {
	uint8_t regs[8]; //Stores raw 8-bit register pairs

	uint8_t *a; //8-bit accumulator register A
	uint8_t *f; //8-bit flag register F
	uint8_t *b; //8-bit register B
	uint8_t *c; //8-bit register C
	uint8_t *d; //8-bit register D
	uint8_t *e; //8-bit register E
	uint8_t *h; //8-bit register H
	uint8_t *l; //8-bit register L

	uint16_t *af; //16-bit register pair AF
	uint16_t *bc; //16-bit register pair BC
	uint16_t *de; //16-bit register pair DE
	uint16_t *hl; //16-bit register pair HL

	uint16_t sp; //16-bit stack pointer register SP
	uint16_t pc; //16-bit program counter register PC

	struct GB_PictureProcessor ppu; //Picture Processing Unit

	uint8_t *boot; //256 B Boot ROM

	uint8_t *hram; //128 B High RAM

	uint8_t ime; //Interrupt Master Enable Flag IME
};

//Defines the state of the contents of the emulated Game Boy's cartridge slot
struct GB_GamePak {
	uint8_t *rom0; //16 KB lower addressable ROM Bank (Bank 00)
	bool isROM0Blocked; //Whether lower ROM bank is currently blocked

	uint8_t *rom1; //16 KB upper addressable ROM Bank (Bank 00 ~ NN)
	bool isROM1Blocked; //Whether upper ROM bank is currently blocked

	uint8_t *extram; //8 KB addressable external RAM Bank
	bool isExtRAMBlocked; //Whether external RAM bank is currently blocked
};

//Defines the total state of the emulated Game Boy system
typedef struct GB_System {
	struct GB_Processor cpu; //Game Boy SoC ("DMG-CPU")

	uint8_t *vram; //8 KB Video RAM
	bool isVRAMBlocked; //Whether VRAM access is currently blocked

	uint8_t *wram; //8 KB Work RAM
	bool isWRAMBlocked; //Whether WRAM access is currently blocked

	struct GB_GamePak cart; //Game Boy cartridge slot contents

	uint8_t *io[0x80]; //Game Boy memory-mapped I/O registers

	uint8_t *lcd[GB_LCD_HEIGHT]; //160 x 144 LCD screen. Contains pointers to scanlines.
	bool lcdBlankThisFrame; //Whether LCD should not render drawn pixels during this frame

	unsigned cyclesNextDIV; //Cycle count upon next increment of DIV register
	unsigned cyclesNextTIMA; //Cycle count upon next increment of TIMA register
} GameBoy;

//Defines button IDs used for Game Boy buttons. Used as indices into isPressed, CTRL_SCANCODES, etc.
enum GameBoyButtonID {
	GB_UP, //D-Pad Up
	GB_DOWN, //D-Pad Down
	GB_LEFT, //D-Pad Left
	GB_RIGHT, //D-Pad Right
	GB_A, //A Button
	GB_B, //B Button
	GB_START, //Start Button
	GB_SELECT //Select Button
};

/*	Externs	*/
extern const int CTRL_SCANCODES[]; //EdBoy.c

/*	Function Prototypes	*/
int Init_Emulator_Windows( SDL_Window **windows ); //Window.c
void Deinit_Emulator_Windows( SDL_Window **windows ); //Window.c

bool Do_FrameStep_Frame( GameBoy *gb, const uint8_t *keyStates, bool *isPressed, bool *justPressed, bool *faJustPressed ); //Run.c
bool Do_FullSpeed_Frame( GameBoy *gb, const uint8_t *keyStates ); //Run.c
bool Pause_On_Unknown_Opcode(); //Run.c

int GB_Init( GameBoy *gb ); //GameBoy/Init.c
void GB_Deinit( GameBoy *gb ); //GameBoy/Init.c

void GB_Load_BootROM( GameBoy *gb, char *path ); //GameBoy/Load.c
int GB_Load_Game( GameBoy *gb, char *path ); //GameBoy/Load.c

bool GB_Run_Frame( GameBoy *gb, bool *isPressed ); //GameBoy/CPU.c
bool GB_Decode_Execute( GameBoy *gb, unsigned *cycles, bool *isPressed ); //GameBoy/CPU.c
void GB_Increment_Cycles_This_Frame( GameBoy *gb, unsigned *cyclesSoFar, unsigned incCycles ); //GameBoy/CPU.c
uint8_t GB_Get_Next_Byte( GameBoy *gb, unsigned *cycles ); //GameBoy/CPU.c
uint8_t GB_Read( GameBoy *gb, unsigned *cycles, uint16_t addr ); //GameBoy/CPU.c