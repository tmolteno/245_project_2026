#include "beep.h"
#include <Arduino.h>
#include <HAL.h>

namespace beep {

void init()
{
    pinMode(PIN_BEEP, OUTPUT);
    digitalWrite(PIN_BEEP, PIN_BEEP_OFF);
}

void beep_ms(uint16_t duration_ms)
{
    // Toggle at ~2 kHz for audible beep
    uint32_t end = millis() + duration_ms;
    while (millis() < end) {
        digitalWrite(PIN_BEEP, LOW);
        delayMicroseconds(250);
        digitalWrite(PIN_BEEP, PIN_BEEP_OFF);
        delayMicroseconds(250);
    }
}

void beep_error()
{
    for (uint8_t i = 0; i < 3; i++) {
        beep_ms(100);
        delay(100);
    }
}

void beep_startup()
{
    beep_ms(50);
}

} // namespace beep
