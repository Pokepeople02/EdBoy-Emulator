#pragma once

#include <SDL.h>;

/*	Game Boy Constants	*/
#define GB_LCD_WIDTH 160 //Game Boy LCD screen pixel width
#define GB_LCD_HEIGHT 144 //Game Boy LCD screen pixel height

/*	Emulator Constants	*/
#define ED_WINDOW_SCALE 3

/*	Debug	*/
#define DEBUG 1 //Debug logging signifier constant
#ifdef DEBUG
#define dprintf(...) printf(__VA_ARGS__) //Debug print function
#else
#define dprintf(...) 
#endif

/*	Shorthand	*/
#define eprintf(...) fprintf( stderr, __VA_ARGS__ ) //stderr print function shorthand

/*	Function Prototypes	*/
int Init_Emu_Windows( SDL_Window **windows ); //Render.c
void Deinit_Emu_Windows( SDL_Window **windows ); //Render.c