#pragma once

#include <stdint.h>

// --- Hardware version selection ---
// Build with -D HW_VERSION=1 for v1 (original) pinout
// Build with -D HW_VERSION=2 for v2 (SD card) pinout
#ifndef HW_VERSION
#define HW_VERSION 2
#endif

#define PIN_BTN_UP PA0
#define PIN_BTN_LEFT PA1
#define PIN_BTN_DOWN PA2
#define PIN_BTN_RIGHT PA3

#if HW_VERSION == 1
    // v1: buttons A/B on PA5/PA4, no SD card
    #define PIN_BTN_A PA5
    #define PIN_BTN_B PA4
#elif HW_VERSION == 2
    // v2: buttons A/B on PB0/PB1, SD card on SPI1
    #define PIN_BTN_A PB0
    #define PIN_BTN_B PB1
#else
    #error "HW_VERSION must be 1 or 2"
#endif

#define PIN_LED_0 PB12
#define PIN_LED_1 PB13

#define PIN_BEEP PA15
#define PIN_BEEP_OFF HIGH // The speaker is not drawing current when the pin is high.

#define IO_D0 PA4
#define IO_D1 PA16
#define IO_D2 PA17
#define IO_D3 PA18
#define PIN_PHOTO       PA5     // PT331C phototransistor (LCSC C53413645), emitter-follower to ADC
#define PIN_THERM       PA6     // NTC 10kΩ B=3950K (LCSC C338623), voltage divider to ADC

// SD card interface (v2 only) — SPI1 PartialRemap2
#if HW_VERSION == 2
#define SD_SPI_PORT     SPI1
#define SD_CS_PIN       PA12
#define SD_SCK_PIN      PA11
#define SD_MISO_PIN     PA9
#define SD_MOSI_PIN     PA10
#define PIN_RESET       PA21    // hardware NRST — active-low, no GPIO config needed

// Enable hardware NRST on PA21 (v2 only — one-time option byte config)
void initNRST();
#endif

// Button Management
    // Default touch thresholds (used until calibration runs)
    #define TOUCH_THRESHOLD_DEFAULT 0x0E20
    #define TOUCH_DEBOUNCE_DEFAULT  0x01A0
    #define TOUCH_MAX_CHANNELS 10

    // Detect if a certain pin is currently touched
    extern int IsTouched(unsigned long int channel);
    // Start up button adc
    extern void initTouchButtons();

    // Run touch calibration (measures baseline, stores in EEPROM on v2).
    // Call once on first boot — subsequent calls are no-ops if already calibrated.
    extern void touchCalibrate();
    // Returns true if calibration has been run (v2 only; always false on v1).
    extern bool touchIsCalibrated();

    // Run calibration unconditionally (ignores stored state), saves to flash.
    // `buildHash` is a one-byte identifier that changes on firmware reflash
    // (e.g. XOR of __DATE__ __TIME__ from a recompiled source file).
    extern void touchCalibrate(uint8_t buildHash);

    // Force recalibration regardless of stored state.
    extern void touchRecalibrate(uint8_t buildHash);

    // Useful for random numbers
    uint16_t Touch_Key_Adc(uint8_t ch);

    // Map pin to ADC touch-key channel
    uint8_t pin_to_touch_adc(uint8_t pin);

/// I2C connection to display
    void i2c_init(void);
    void i2c_start(void);
    void i2c_send_byte(unsigned char data);
    void i2c_stop(void);

#if HW_VERSION == 2
/// SPI connection (for SD card)
    void spi_init(void);
    uint8_t spi_transfer(uint8_t data);
    void spi_cs_low(void);
    void spi_cs_high(void);
#endif

// Sensor readings (v2 analog inputs)
uint16_t photoRead(void);        // PT331C phototransistor on PA5 — raw ADC (0–4095)
uint16_t thermRead(void);        // NTC thermistor on PA6 — raw ADC (0–4095)
int16_t  thermReadCelsius(void); // NTC thermistor on PA6 — temperature in °C × 10

#include "extras.h"