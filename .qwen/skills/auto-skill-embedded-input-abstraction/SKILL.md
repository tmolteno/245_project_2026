---
name: embedded-input-abstraction
description: Build a poll-based input library with edge detection (justPressed/justReleased) on top of raw hardware button reads — for embedded MCU projects with capacitive touch, GPIO, or matrix keypads
source: auto-skill
extracted_at: '2026-07-16T11:00:00.000Z'
---

# Embedded Input Abstraction Library

Build a lightweight input library that wraps raw hardware button reads (GPIO, capacitive
touch, matrix scan, etc.) and provides edge detection: `pressed()`, `justPressed()`,
`justReleased()`. This is critical for game consoles, menu systems, and any UI where you
need to distinguish "button went down this frame" from "button is still held."

## Core pattern: single `update()` per frame

The library does NOT use interrupts or callbacks. Instead the application calls
`input::update()` once per main loop iteration. This keeps it deterministic and avoids
re-entrancy issues on bare-metal MCUs.

```cpp
namespace input {

void init();
void update();                       // call once per frame

bool pressed(uint8_t pin);           // currently held
bool justPressed(uint8_t pin);       // newly pressed this frame (rising edge)
bool justReleased(uint8_t pin);      // newly released this frame (falling edge)

} // namespace input
```

## Implementation: bitmask state tracking

Use three `uint8_t` or `uint16_t` static variables (depending on button count):

```cpp
static uint8_t prevState   = 0;  // state from last update()
static uint8_t edgePress   = 0;  // bits that transitioned 0→1 this frame
static uint8_t edgeRelease = 0;  // bits that transitioned 1→0 this frame
```

The edge detection logic (the key 2 lines):

```cpp
void update() {
    uint8_t cur = 0;
    for (uint8_t i = 0; i < btnCount; i++) {
        if (read_raw_button(btnPins[i]))
            cur |= (1 << i);
    }
    edgePress   = (cur ^ prevState) & cur;
    edgeRelease = (cur ^ prevState) & prevState;
    prevState   = cur;
}
```

- `(cur ^ prevState)` — which bits changed since last frame
- `& cur` — keep only the ones that changed TO 1 (rising edge)
- `& prevState` — keep only the ones that changed TO 0 (falling edge)

## Button-to-bit mapping

Since the state bitmask uses compact bits (0–N), not raw pin numbers, you need a mapping
function. For simple cases where pin values happen to be 0–N (like Arduino pin numbers or
enum values), the mapping can be a direct lookup:

```cpp
static const uint8_t btnPins[] = {
    PIN_UP, PIN_DOWN, PIN_LEFT, PIN_RIGHT,
    PIN_A,   PIN_B,
};

static uint8_t btnMask(uint8_t pin) {
    for (uint8_t i = 0; i < btnCount; i++) {
        if (btnPins[i] == pin) return (1 << i);
    }
    return 0;
}

bool justPressed(uint8_t pin) {
    return (edgePress & btnMask(pin)) != 0;
}
```

This decouples the internal bitmask (compact, fixed-size) from the external pin identifiers
(which may be scattered across GPIO ports with non-contiguous values).

## Why this approach vs. per-button callbacks

- **No dynamic allocation**: static bitmasks, no heap
- **No interrupt re-entrancy**: all state changes happen in `update()`, called from the
  main loop
- **Deterministic timing**: the state snapshot is taken at a known point each frame, so
  all button queries within that frame see a consistent snapshot
- **RAM**: 3 bytes of state for up to 8 buttons, 6 bytes for up to 16

## Integration with hardware debouncing

The raw `read_raw_button()` call should already do hardware-level debouncing (e.g.
touch-key threshold + hysteresis, or GPIO with RC filter). The input library only adds
frame-level edge detection — it does NOT do software debouncing. This keeps the
responsibilities clean:

- **HAL layer**: handles the electrical reality (debounce, threshold, noise filtering)
- **Input library**: handles the logical abstraction (frames, edges, held state)
- **Application**: only sees clean, logical button state

## Anti-patterns to avoid

- **Don't read buttons at arbitrary points**: if the application calls `IsTouched()` in
  multiple places within one frame, you can get inconsistent results (button state changes
  between reads). Always snapshot once via `update()` then query the snapshot.
- **Don't track edges per-button with separate variables**: leads to code explosion when
  button count grows. The bitmask pattern scales to N buttons with O(1) edge computation.
- **Don't put the mapping table in the header**: the pin list is an implementation detail.
  The API should accept the same pin constants the HAL uses, so the caller doesn't need to
  know about internal bit assignments.
