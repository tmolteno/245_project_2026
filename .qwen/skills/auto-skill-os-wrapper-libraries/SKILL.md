---
name: os-wrapper-libraries
description: Create thin OS wrapper libraries (led, timer, etc.) that provide namespace-scoped APIs over Arduino/HAL primitives — including critical header naming collision avoidance with framework headers
source: auto-skill
extracted_at: '2026-07-16T11:30:00.000Z'
---

# OS Wrapper Libraries for Embedded Projects

Create thin, namespace-scoped wrapper libraries that present Arduino or HAL primitives
through a clean, consistent OS API. The pattern is: one header, one source file, one
namespace, minimal dependencies.

## When to use this pattern

When you want to replace scattered `pinMode()`/`digitalWrite()`/`delay()` calls in
application code with `led::on()`/`ostime::delay_ms()` calls that:
- Are discoverable via Doxygen
- Can be mocked or replaced for testing
- Signal intent more clearly than raw Arduino calls
- Keep the application decoupled from which specific framework provides `millis()` or
  GPIO access

## Library structure

```
lib/phsi245_<name>/
├── <name>.h       # public API: namespace, function declarations
└── <name>.cpp     # implementation: wraps Arduino/HAL functions
```

Example for an LED library:

**led.h**:
```cpp
#pragma once
#include <stdint.h>

namespace led {
    void init();
    void on(uint8_t pin);
    void off(uint8_t pin);
    void toggle(uint8_t pin);
}
```

**led.cpp**:
```cpp
#include "led.h"
#include <Arduino.h>
#include <HAL.h>   // for PIN_LED_0, PIN_LED_1

namespace led {
    void init() {
        pinMode(PIN_LED_0, OUTPUT);
        pinMode(PIN_LED_1, OUTPUT);
        off(PIN_LED_0);
        off(PIN_LED_1);
    }
    void on(uint8_t pin)  { digitalWrite(pin, HIGH); }
    void off(uint8_t pin) { digitalWrite(pin, LOW); }
    void toggle(uint8_t pin) { digitalToggle(pin); }
}
```

## Include order for dependencies on HAL pin defines

If the library's `.cpp` references HAL pin defines (like `PIN_LED_0`, `PIN_BTN_A`), include
`<HAL.h>` **before** `<Arduino.h>` — the project's HAL may provide SPL/chip headers that
the Arduino framework's own headers need. The safe order is:

```cpp
#include "own_header.h"
#include <HAL.h>     // chip SPL types, pin defines
#include <Arduino.h> // millis(), digitalWrite(), etc.
```

## Critical pitfall: header naming collisions with the Arduino framework

The Arduino framework for CH32V (and likely other platforms) has its own internal headers
in its cores directory. When you create a library, PlatformIO adds `lib/<your_lib>/` to
the include path. If your header name matches a framework header, **your header shadows
the framework header** and causes compilation errors deep in framework code.

### The `timer.h` collision

Creating a file named `timer.h` in your library causes `HardwareTimer.h` (included via
`Arduino.h → wiring.h → board.h → analog.h → HardwareTimer.h`) to find YOUR `timer.h`
instead of the framework's `cores/arduino/timer.h`. This produces unrelated-looking errors
like:

```
HardwareTimer.h:164: error: 'TIM_HandleTypeDef' has not been declared
```

**Fix**: rename the file and namespace to something unambiguous. `ostime.h` / `namespace ostime`
works; `systick.h` / `namespace systick` also works. Never use generic names like `timer`,
`time`, `clock`, `gpio`, `spi`, `i2c`, `uart`, `pwm`, `adc`, `dac`, `dma`, `rtc`, `wdt`,
`sleep`, `power`, `reset`, `interrupt`, `flash`, `eeprom`, `usb`, `can`, `eth` as file names —
the Arduino cores directory has headers with many of these names.

### How to check for collisions

```bash
# Find framework headers that match your proposed name
find ~/.platformio/packages/framework-*/cores -name "timer.h" 2>/dev/null
```

If the command returns anything, pick a different name.

## Pattern for timer/tick libraries

The simplest tick library wraps `millis()`:

```cpp
namespace ostime {
    void     init();          // no-op; millis() is already running
    uint32_t ticks();         // returns millis()
    void     delay_ms(uint32_t ms);  // blocking delay using ticks()
}
```

Implementation of `delay_ms` should use `ticks()` internally rather than calling
Arduino `delay()` directly — this keeps the abstraction self-consistent:

```cpp
void delay_ms(uint32_t ms) {
    uint32_t start = millis();
    while ((millis() - start) < ms) {
        // busy-wait
    }
}
```

## Application integration

In the main firmware, replace raw Arduino calls:

```cpp
// Before
#include <Arduino.h>
pinMode(PIN_LED_0, OUTPUT);
digitalWrite(PIN_LED_0, HIGH);
delay(150);

// After
#include "led.h"
#include "ostime.h"
led::init();
led::on(PIN_LED_0);
ostime::delay_ms(150);
```

The pin constants (`PIN_LED_0`, `PIN_BTN_A`, etc.) still come from `HAL.h` — the wrapper
libraries accept them directly so the application doesn't need an additional mapping layer.
