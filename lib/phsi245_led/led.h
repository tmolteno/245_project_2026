#pragma once

#include <stdint.h>

namespace led {

void init();
void on(uint8_t pin);
void off(uint8_t pin);
void toggle(uint8_t pin);

} // namespace led
