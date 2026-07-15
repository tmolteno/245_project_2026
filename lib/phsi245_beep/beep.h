#pragma once

#include <stdint.h>

namespace beep {

void init();
void beep_ms(uint16_t duration_ms);
void beep_error();   // 3 short beeps
void beep_startup(); // 1 short beep

} // namespace beep
