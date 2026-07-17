#include "input.h"
#include <HAL.h>

namespace input {

static uint8_t prevState = 0;
static uint8_t edgePress = 0;
static uint8_t edgeRelease = 0;

// Hold tracking: press timestamp + last repeat timestamp per button
static uint32_t holdStart[6];
static uint32_t lastRepeat[6];

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
    for (uint8_t i = 0; i < btnCount; i++)
        holdStart[i] = 0;
}

void update()
{
    uint8_t cur = 0;
    uint32_t now = millis();

    for (uint8_t i = 0; i < btnCount; i++) {
        if (IsTouched(btnPins[i]))
            cur |= (1 << i);
    }

    edgePress  = (cur ^ prevState) & cur;
    edgeRelease = (cur ^ prevState) & prevState;
    prevState = cur;

    // Track hold start timestamps
    for (uint8_t i = 0; i < btnCount; i++) {
        if (edgePress & (1 << i))
            holdStart[i] = now;
    }
}

static uint8_t btnMask(uint8_t pin)
{
    for (uint8_t i = 0; i < btnCount; i++) {
        if (btnPins[i] == pin) return (1 << i);
    }
    return 0;
}

static int8_t btnIndex(uint8_t pin)
{
    for (uint8_t i = 0; i < btnCount; i++) {
        if (btnPins[i] == pin) return i;
    }
    return -1;
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

bool heldFor(uint8_t pin, uint16_t delayMs, uint16_t repeatMs)
{
    int8_t idx = btnIndex(pin);
    if (idx < 0) return false;

    uint8_t mask = 1 << idx;
    if (!(prevState & mask)) {
        holdStart[idx] = 0;
        lastRepeat[idx] = 0;
        return false;
    }

    uint32_t now = millis();
    uint32_t elapsed = now - holdStart[idx];

    if (elapsed < delayMs) return false;

    // If repeatMs is 0, fire once after delay (no repeat)
    if (repeatMs == 0) {
        if (lastRepeat[idx] == 0) {
            lastRepeat[idx] = now;
            return true;
        }
        return false;
    }

    // Fire on initial delay expiry, then every repeatMs thereafter
    if (lastRepeat[idx] == 0 || (now - lastRepeat[idx]) >= repeatMs) {
        lastRepeat[idx] = now;
        return true;
    }
    return false;
}

bool allPressed(uint8_t count, const uint8_t pins[])
{
    for (uint8_t i = 0; i < count; i++) {
        if (!pressed(pins[i])) return false;
    }
    return true;
}

bool anyPressed(uint8_t count, const uint8_t pins[])
{
    for (uint8_t i = 0; i < count; i++) {
        if (pressed(pins[i])) return true;
    }
    return false;
}

} // namespace input
