---
name: hw-version-pinout
description: Support multiple hardware revisions in a single PlatformIO codebase via a HW_VERSION build flag — conditional pin definitions, guarded source files, separate build environments, and application-level feature adaptation
source: auto-skill
extracted_at: '2026-07-16T12:00:00.000Z'
---

# Multi-Hardware-Version Pinout via Build Flag

When a project has two (or more) PCB revisions with different pin assignments —
e.g. v1 has buttons on PA4/PA5 with no SD card, v2 moves buttons to PB0/PB1 and
adds an SD card on SPI1 — use a single `HW_VERSION` preprocessor flag instead of
maintaining separate branches or board definitions.

## The build flag

Define `HW_VERSION` in the PlatformIO build flags as an integer (1, 2, …):

```ini
[env:CH32X035]
build_flags = -D HW_VERSION=2   # default to latest revision

[env:v1]
extends = env:CH32X035
build_flags = ${env:CH32X035.build_flags} -D HW_VERSION=1

[env:v2]
extends = env:CH32X035
build_flags = ${env:CH32X035.build_flags} -D HW_VERSION=2
```

In a Makefile, expose version-specific targets with a default:

```makefile
ENV ?= v2
build:
    pio run -e $(ENV)
build-v1:
    pio run -e v1
upload-v1: build-v1
    pio run -e v1 -t upload
```

## HAL header: conditional pin defines

In the top-level HAL header, provide a default and a compile-time error for unknown
versions:

```cpp
#pragma once
#include <stdint.h>

#ifndef HW_VERSION
#define HW_VERSION 2          // default
#endif

// Pins that are the same across all revisions
#define PIN_BTN_UP    PA0
#define PIN_BTN_LEFT  PA1
#define PIN_BTN_DOWN  PA2
#define PIN_BTN_RIGHT PA3

// Pins that move between revisions
#if HW_VERSION == 1
    #define PIN_BTN_A PA5    // v1: on GPIOA
    #define PIN_BTN_B PA4
#elif HW_VERSION == 2
    #define PIN_BTN_A PB0    // v2: moved to GPIOB
    #define PIN_BTN_B PB1
#else
    #error "HW_VERSION must be 1 or 2"
#endif

// Revision-specific hardware features
#if HW_VERSION == 2
#define SD_SPI_PORT   SPI1
#define SD_CS_PIN     PA4
#define SD_SCK_PIN    PA5
#define SD_MISO_PIN   PA6
#define SD_MOSI_PIN   PA7
#endif
```

### Guard function declarations too

If a hardware peripheral only exists on certain revisions, guard its function
declarations:

```cpp
#if HW_VERSION == 2
void spi_init(void);
uint8_t spi_transfer(uint8_t data);
void spi_cs_low(void);
void spi_cs_high(void);
#endif
```

## HAL source: conditional init sequences

Touch button initialization differs between revisions. Use `#if` to adjust which
GPIO pins are configured and how many:

```cpp
void initTouchButtons() {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

#if HW_VERSION == 1
    // v1: all 6 buttons on GPIOA
    const unsigned char pa_pins[6] = {PIN_UP, PIN_LEFT, PIN_DOWN, PIN_RIGHT,
                                       PIN_A, PIN_B};
    const uint8_t pa_count = 6;
#else
    // v2: only 4 D-pad buttons on GPIOA; A/B moved to GPIOB
    const unsigned char pa_pins[4] = {PIN_UP, PIN_LEFT, PIN_DOWN, PIN_RIGHT};
    const uint8_t pa_count = 4;
#endif
    for (uint8_t i = 0; i < pa_count; i++) { /* configure PA pins */ }

#if HW_VERSION == 2
    const uint16_t pb_pins[2] = {GPIO_Pin_0, GPIO_Pin_1};
    for (int i = 0; i < 2; i++) { /* configure PB pins */ }
#endif

    // ADC and TouchKey init is identical for both versions
    // ...
}
```

The ADC channel mapping must also be conditional:

```cpp
static uint8_t pin_to_touch_adc(uint8_t pin) {
    if (pin <= PA7) return pin;  // PA0–PA7 → TK0–TK7 (both versions)
#if HW_VERSION == 2
    if (pin == PB0) return 8;    // PB0 → TK8 (v2 only)
    if (pin == PB1) return 9;    // PB1 → TK9 (v2 only)
#endif
    return 0;
}
```

## Guarding whole source files

For a peripheral driver that only exists on one revision (e.g. SPI SD card on v2),
wrap the entire file body:

```cpp
// sd_spi.cpp
#include "storage.h"
#include <HAL.h>
#include <Arduino.h>

#if HW_VERSION == 2

namespace sd {
    // ... all implementation ...
    const BlockDevice DEVICE = { init, readBlock, writeBlock, blockCount };
} // namespace sd

#endif // HW_VERSION == 2
```

## Application adaptation

The application uses `#if HW_VERSION` to guard includes, skip features, and show
revision-appropriate UI:

```cpp
#include <Arduino.h>
#include <HAL.h>
#include "gfx.h"
#if HW_VERSION == 2
#include "storage.h"
#endif

void setup() {
    // Init common to both versions
    gfx::init();
    input::init();

#if HW_VERSION == 1
    state = STATE_V1_INFO;   // skip SD, show info screen
#endif
}
```

In the state machine, guard feature-specific states:

```cpp
switch (state) {
#if HW_VERSION == 1
case STATE_V1_INFO:
    // Show "Hardware v1 — no SD card" screen
    break;
#endif

#if HW_VERSION == 2
case STATE_BROWSE:
    // File browser (only exists on v2)
    break;
#endif
}
```

And include headers conditionally to avoid missing-symbol errors:

```cpp
#if HW_VERSION == 2
static void collectFiles() {
    fat::openDir("/");
    while (fat::readDir(...)) { ... }
}
#endif
```

## Why this approach vs. separate board definitions

- **Single codebase**: one set of source files, one git history
- **Compile-time safety**: `#error` catches unknown versions; wrong pin assignments
  fail at build time, not runtime
- **Dead code elimination**: with LTO enabled (`board_build.use_lto = yes`),
  functions guarded by `#if` that evaluate to false are stripped from the binary
- **No runtime overhead**: zero runtime checks — the `#if` is resolved by the
  preprocessor
- **Convenient Makefile targets**: `make build-v1` / `make upload-v1` are
  explicit and discoverable

## Anti-patterns to avoid

- **Don't use `#ifdef HW_VERSION_V1` / `#ifdef HW_VERSION_V2`**: you'll forget to set
  one and silently build with the wrong config. Use a single integer `HW_VERSION`
  with `#if ==` checks and a final `#else #error` for unknown values.
- **Don't put pinout in a separate config header**: it looks cleaner but doubles
  the include burden. The HAL header is the single source of truth for pin
  definitions; keep it there.
- **Don't try to make every function work on every version**: wrapping `sd_spi.cpp`
  with `#if HW_VERSION == 2` is simpler than guarding every SPI register access
  individually. The binary is smaller too since LTO strips the empty TU.
