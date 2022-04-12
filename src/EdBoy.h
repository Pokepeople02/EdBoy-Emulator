#pragma once

#include <stdbool.h>
#include <SDL.h>;

/*	Game Boy Constants	*/
#define GB_LCD_HEIGHT 144 //Game Boy LCD screen pixel height
#define GB_LCD_WIDTH 160 //Game Boy LCD screen pixel width

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
//Defines button IDs used for Game Boy buttons. Used as indices into isPressed, CTRL_SCANCODES, etc.
enum gameBoyButtonID {
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
int InitEmuWindows( SDL_Window **windows ); //Render.c
void DeinitEmuWindows( SDL_Window **windows ); //Render.c
void DoFrameStepFrame( uint8_t *keyStates, bool *isPressed, bool *justPressed ); //EdBoy.c
void DoFullSpeedFrame( uint8_t *keyStates ); //EdBoy.c