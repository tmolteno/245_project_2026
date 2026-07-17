---
name: remap-touch-buttons-across-ports
description: Remap CH32X035 capacitive touch buttons from GPIOA to GPIOB while preserving the IsTouched() API — for freeing SPI pins during hardware revision
source: auto-skill
extracted_at: '2026-07-16T01:00:00.000Z'
---

# Remap Touch Buttons Across GPIO Ports

When a hardware revision moves capacitive touch buttons from GPIOA to GPIOB (e.g. to free
PA4–PA7 for an SPI SD card), the touch-key HAL needs three coordinated changes. The
CH32X035 touch-key controller maps TK0–TK7 to PA0–PA7 and TK8–TK9 to PB0–PB1.

## Problem

The naive approach — just changing the `#define` for the button pin and keeping the same
`initTouchButtons()` loop — fails for two reasons:

1. **`(1 << pin_value)` overflows for GPIOB.** The `PinName` enum assigns PB0 = 24, so
   `(1 << 24)` is 0x1000000, which truncates to 0 in the GPIO peripheral's 16-bit
   `GPIO_Pin` field. You must use the `GPIO_Pin_X` constants (e.g. `GPIO_Pin_0`,
   `GPIO_Pin_1`) for GPIOB pins instead of computing a shift from the pin number.

2. **ADC channel ≠ pin number for GPIOB.** The `Touch_Key_Adc()` function expects an ADC
   channel (0–9), not a raw `PinName`. PA0–PA7 happen to equal their ADC channels (0–7),
   but PB0 = 24 maps to ADC channel 8, and PB1 = 25 maps to ADC channel 9.

## Solution: three coordinated changes

### Step 1 — Update pin defines in `HAL.h`

```cpp
#define PIN_BTN_A PB0   // was PA5
#define PIN_BTN_B PB1   // was PA4
// PA0–PA3 remain unchanged
```

This is the only user-facing change. All downstream code (`IsTouched(PIN_BTN_A)`, etc.)
continues to compile and work because the internal mapping absorbs the difference.

### Step 2 — Add an ADC channel mapping function

```cpp
static uint8_t pin_to_touch_adc(uint8_t pin) {
    if (pin <= PA7) return pin;  // PA0–PA7 → ADC channels 0–7
    if (pin == PB0) return 8;    // PB0 → TK8
    if (pin == PB1) return 9;    // PB1 → TK9
    return 0;
}
```

This exploits the fact that `PinName` values for PA0–PA7 are 0–7, which match the ADC
channel numbers. Only the GPIOB pins need explicit mapping.

### Step 3 — Split `initTouchButtons()` across ports

The original loop initialized all 6 pins on `GPIOA` with a single `(1 << cap_pins[i])` shift.
Replace it with two port-specific loops:

```cpp
void initTouchButtons() {
    // Enable both GPIO clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  // NEW
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // GPIOA: pin values 0–3 work with (1 << pin) shift
    const unsigned char pa_pins[4] = {PIN_BTN_UP, PIN_BTN_LEFT,
                                       PIN_BTN_DOWN, PIN_BTN_RIGHT};
    for (int i = 0; i < 4; i++) {
        GPIO_InitStructure.GPIO_Pin = (1 << pa_pins[i]);
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }

    // GPIOB: must use GPIO_Pin_X constants, not (1 << pin_value)
    const uint16_t pb_pins[2] = {GPIO_Pin_0, GPIO_Pin_1};
    for (int i = 0; i < 2; i++) {
        GPIO_InitStructure.GPIO_Pin = pb_pins[i];
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    // ... ADC/TouchKey init unchanged ...
}
```

## Why this is the key insight

`(1 << PB0) ` where PB0 = 24 silently truncates to 0 because the GPIO peripheral's pin
mask register is 16 bits wide. On a 16-bit shift, `24 & 0x0F = 8`, so you'd actually get
`(1 << 8) = GPIO_Pin_8`, which is wrong — it targets PA8, not PB0. Using `GPIO_Pin_0` and
`GPIO_Pin_1` directly avoids the bit-shift computation entirely and is correct for any port.

## The `IsTouched()` API stays unchanged

Callers continue to pass pin names. The internal `pin_to_touch_adc()` conversion is
transparent:

```cpp
int IsTouched(unsigned long int channel) {
    uint8_t adc_ch = pin_to_touch_adc(channel);  // pin → ADC channel
    uint16_t val = Touch_Key_Adc(adc_ch);
    // TouchState bitmask uses adc_ch (0–9), not raw pin (0–25)
    if (!(TouchState & (1 << adc_ch)) && val < TOUCH_THRESHOLD) {
        TouchState |= (1 << adc_ch);
        return 1;
    }
    // ...
}
```

The `TouchState` bitmask must also switch from pin-indexed to ADC-channel-indexed,
otherwise `(1 << 24)` overflows the `uint16_t` state variable.

## Validation

After the change, build both the new application AND an existing game (e.g. HelloNOISE)
to confirm the HAL change doesn't break downstream consumers. Both should compile cleanly.
