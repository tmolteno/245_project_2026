#pragma once

#include <stdint.h>

namespace input {

void init();
void update();

bool pressed(uint8_t pin);
bool justPressed(uint8_t pin);
bool justReleased(uint8_t pin);

} // namespace input
