#include <Arduino.h>
#include <HAL.h>

void setup()
{
    pinMode(PIN_LED_0, OUTPUT);
    digitalWrite(PIN_LED_0, HIGH);
}

void loop()
{
    delay(500);
    digitalToggle(PIN_LED_0);
}
