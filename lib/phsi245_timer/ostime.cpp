#include "ostime.h"
#include <HAL.h>
#include <Arduino.h>

namespace ostime {

static uint32_t framePeriodMs  = 33;  // default ~30 fps
static uint32_t nextFrameTime  = 0;
static uint32_t frameCounter   = 0;

void init()
{
    // millis() is already initialised by the Arduino framework
    nextFrameTime = millis();
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

void setFrameRate(uint8_t fps)
{
    if (fps == 0) fps = 30;
    framePeriodMs = 1000 / fps;
}

bool nextFrame()
{
    uint32_t now = millis();
    int32_t  remaining = (int32_t)(nextFrameTime - now);

    if (remaining > 0) {
        // Wait for the next frame boundary
        delay_ms((uint32_t)remaining);
    }

    nextFrameTime = millis() + framePeriodMs;
    frameCounter++;
    return true;
}

uint32_t frameCount()
{
    return frameCounter;
}

} // namespace ostime
