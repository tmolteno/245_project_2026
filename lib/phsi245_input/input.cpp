#include "input.h"
#include <HAL.h>

namespace input {

static uint8_t prevState = 0;
static uint8_t edgePress = 0;
static uint8_t edgeRelease = 0;

// Buttons to poll (must match 8-bit mask)
static const uint8_t btnPins[] = {
    PIN_BTN_UP,    // bit 0
    PIN_BTN_DOWN,  // bit 1
    PIN_BTN_LEFT,  // bit 2
    PIN_BTN_RIGHT, // bit 3
    PIN_BTN_A,     // bit 4
    PIN_BTN_B,     // bit 5
};
static const uint8_t btnCount = 6;

void init()
{
    initTouchButtons();
    prevState = 0;
    edgePress = 0;
    edgeRelease = 0;
}

void update()
{
    uint8_t cur = 0;

    for (uint8_t i = 0; i < btnCount; i++) {
        if (IsTouched(btnPins[i]))
            cur |= (1 << i);
    }

    edgePress  = (cur ^ prevState) & cur;
    edgeRelease = (cur ^ prevState) & prevState;
    prevState = cur;
}

static uint8_t btnMask(uint8_t pin)
{
    for (uint8_t i = 0; i < btnCount; i++) {
        if (btnPins[i] == pin) return (1 << i);
    }
    return 0;
}

bool pressed(uint8_t pin)
{
    return (prevState & btnMask(pin)) != 0;
}

bool justPressed(uint8_t pin)
{
    return (edgePress & btnMask(pin)) != 0;
}

bool justReleased(uint8_t pin)
{
    return (edgeRelease & btnMask(pin)) != 0;
}

} // namespace input
