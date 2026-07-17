#pragma once
#include <stdint.h>

// This file contains some extra patches which I have found necessary to run the Arduboy games I have tested

#define _delay_ms delay
#define _delay_us delayMicroseconds


#define max(x,y) ((x > y)? (x) : (y))
#define min(x,y) ((x > y)? (y) : (x))

#define bitToggle(value, bit) ((value) ^= (1UL << (bit)))

// Some common functions which need rewriting from Danial C's games
extern void DanCSound(uint8_t freq,uint8_t dur);


// EEPROM shim for Arduboy2-compatible games.
// Only included when the game/app explicitly needs it (includes <EEPROM.h> first).
#ifdef NO_GLOBAL_EEPROM
#if __has_include(<EEPROM.h>)
#include <EEPROM.h>

class EEPROMShim : public EEPROMClass
{
    public:
    void update(int const idx, uint8_t const val);
    void begin (int a);
    void begin ();
};
extern EEPROMShim EEPROM;
#endif
#endif
