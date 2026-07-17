#pragma once

#include <stdint.h>

namespace beep {

void init();
void beep_ms(uint16_t duration_ms);
void beep_error();   // 3 short beeps
void beep_startup(); // 1 short beep

// Play a square-wave tone at `freqHz` for `durationMs` (blocks).
// freqHz = 0 silences the speaker for the duration.
void tone(uint16_t freqHz, uint16_t durationMs);

// Play a PROGMEM note sequence (non-blocking — call once per frame).
// Each note is 4 bytes: freqHi, freqLo, durHi, durLo (16-bit freq, 16-bit ms).
// Terminated by {0,0,0,0}. freq=0 is a rest.
// Returns true while the melody is still playing.
bool playMelody(const uint8_t *notes);

} // namespace beep

