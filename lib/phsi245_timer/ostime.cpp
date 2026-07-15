#include "ostime.h"
#include <HAL.h>
#include <Arduino.h>

namespace ostime {

void init()
{
    // millis() is already initialised by the Arduino framework
}

uint32_t ticks()
{
    return millis();
}

void delay_ms(uint32_t ms)
{
    uint32_t start = millis();
    while ((millis() - start) < ms) {
        // busy-wait — yields to any background framework work
    }
}

} // namespace ostime
