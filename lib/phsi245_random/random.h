#pragma once

#include <stdint.h>

namespace rng {

// Seed the PRNG from touch-key ADC noise.
// Call once during setup().
void init();

// Return a random integer in [0, max-1].
// Uses a 32-bit xorshift generator — fast, small, no division.
uint16_t next(uint16_t max);

// Return a random integer in [min, max] inclusive.
uint16_t next(uint16_t min, uint16_t max);

} // namespace rng
