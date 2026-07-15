#pragma once

#include <stdint.h>

namespace ostime {

void     init();
uint32_t ticks();
void     delay_ms(uint32_t ms);

} // namespace ostime
