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


// Patch EEPROM library version
// Reduires you to compile with NO_GLOBAL_EEPROM defined
#ifdef NO_GLOBAL_EEPROM
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
