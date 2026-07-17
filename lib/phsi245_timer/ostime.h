#pragma once

#include <stdint.h>

namespace ostime {

void     init();
uint32_t ticks();
void     delay_ms(uint32_t ms);

// Frame-rate control — drop-in replacement for arduboy.setFrameRate()/nextFrame()
void     setFrameRate(uint8_t fps);
bool     nextFrame();           // returns true when it's time to render a new frame
uint32_t frameCount();          // number of frames rendered since boot

} // namespace ostime
