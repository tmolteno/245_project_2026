#pragma once

#include <Arduboy2.h>
#include "avr/pgmspace.h"

// Based on HP35 emulator http://home.citycable.ch/pierrefleur/Jacques-Laporte/HP35_Arduino.htm check license in rom.h
#include "rom.h"
#include "registers.h"

#include "keyboard.h"
#include "bitmaps.h"

extern Arduboy2 arduboy;
extern Sprites sprites;

#define SSIZE 12
#define WSIZE 14

extern int i, tm;
extern int io_count;
extern boolean display_enable;
extern boolean update_display;

extern boolean enable_bug ;
extern void lcd_init(bool powered);