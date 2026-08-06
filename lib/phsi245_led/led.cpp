#include "led.h"
#include <Arduino.h>
#include <HAL.h>

namespace led {

void init()
{
    pinMode(PIN_LED_0, OUTPUT);
    pinMode(PIN_LED_1, OUTPUT);
    off(PIN_LED_0);
    off(PIN_LED_1);
}

void on(uint8_t pin)
{
    digitalWrite(pin, HIGH);
}

void off(uint8_t pin)
{
    digitalWrite(pin, LOW);
}

void toggle(uint8_t pin)
{
    digitalToggle(pin);
}

} // namespace led
