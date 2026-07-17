#pragma once

#include <stdint.h>

namespace input {

void init();
void update();

bool pressed(uint8_t pin);
bool justPressed(uint8_t pin);
bool justReleased(uint8_t pin);

// Hold-to-repeat: returns true on initial press, then repeats every `repeatMs`
// after an initial `delayMs`. Use for menu scrolling, movement, etc.
//   if (input::heldFor(PIN_BTN_UP, 400, 100)) paddleY -= 2;
bool heldFor(uint8_t pin, uint16_t delayMs, uint16_t repeatMs = 0);

// Combo helpers: check if a set of pins are all/any pressed this frame.
// Pins are passed as a flat list + count for simplicity.
bool allPressed(uint8_t count, const uint8_t pins[]);
bool anyPressed(uint8_t count, const uint8_t pins[]);

} // namespace input
