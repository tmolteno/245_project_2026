# New Version — PHSI245 Game Boy v2

This document describes the software changes for the next hardware revision (2026), which adds SD card storage and reorganizes the pin assignments.

## Summary of Changes

### Hardware Pin Reassignment

| Pin | v1 Function | v2 Function | Notes |
|---|---|---|---|
| PA0 | Button UP | Button UP | Unchanged (TK0) |
| PA1 | Button LEFT | Button LEFT | Unchanged (TK1) |
| PA2 | Button DOWN | Button DOWN | Unchanged (TK2) |
| PA3 | Button RIGHT | Button RIGHT | Unchanged (TK3) |
| PA4 | Button B | IO_D0 | Freed from SPI (was SD CS) |
| PA5 | Button A | *(free)* | Freed from SPI (was SD SCK) |
| PA6 | *(unused)* | *(free)* | Freed from SPI (was SD MISO) |
| PA7 | *(unused)* | *(free)* | Freed from SPI (was SD MOSI) |
| PA9 | *(speaker)* | **SD MISO** | SPI1 PartialRemap2 |
| PA10 | *(I2C SCL)* | **SD MOSI** | SPI1 PartialRemap2 |
| PA11 | *(I2C SDA)* | **SD SCK** | SPI1 PartialRemap2 |
| PA12 | *(unused)* | **SD CS** | SPI1 PartialRemap2 |
| PA13 | *(unused)* | **I2C SCL** | I2C1 PartialRemap1 (display) |
| PA14 | *(unused)* | **I2C SDA** | I2C1 PartialRemap1 (display) |
| PA15 | IO_D0 | **Speaker** | Moved from PA9 |
| PA21 | *(NRST)* | **NRST / Reset** | External reset button to GND |
| PB0 | *(unused)* | **Button A** | Touch key channel TK8 |
| PB1 | *(unused)* | **Button B** | Touch key channel TK9 |

Buttons A and B move from PA5/PA4 to PB0/PB1 to free the SPI1 peripheral pins for the SD card. The CH32X035 touch key controller supports channels TK8 (PB0) and TK9 (PB1) natively.

The SD card uses **SPI1 PartialRemap2**, which maps SPI1 to PA9–PA12. The display uses **I2C1 PartialRemap1**, which maps I2C1 to PA13–PA14. Both remaps are configured via AFIO at init time. The speaker moves from PA9 to PA15 to avoid conflict with the SPI MISO pin.

### External Reset Button

PA21 is the hardware NRST pin. A momentary push-button from PA21 to GND
provides a full hardware reset that works even if firmware hangs:

```
PA21 (LQFP48 pin 7) ──┬── button ── GND
                      │
                      └── 10kΩ (optional) ── VDD
```

No firmware changes required — the CH32X035 configures PA21 as active-low
reset by default. The internal pull-up (~40kΩ) is usually sufficient;
the external 10kΩ pull-up improves noise immunity in harsh environments.

### HAL Changes (`lib/PHSI245_HAL/`)

**`HAL.h`** — Updated button pin definitions:

```cpp
#define PIN_BTN_A PB0   // was PA5
#define PIN_BTN_B PB1   // was PA4
```

Added SPI SD card pin definitions (SPI1 PartialRemap2):

```cpp
#define SD_SPI_PORT  SPI1
#define SD_CS_PIN    PA12
#define SD_SCK_PIN   PA11
#define SD_MISO_PIN  PA9
#define SD_MOSI_PIN  PA10
```

Speaker moved from PA9 to PA15:

```cpp
#define PIN_BEEP PA15  // was PA9
```

Added SPI function declarations:

```cpp
void spi_init(void);
uint8_t spi_transfer(uint8_t data);
void spi_cs_low(void);
void spi_cs_high(void);
```

**`HAL.cpp`** — Updated touch button initialization:
- Added `pin_to_touch_adc()` helper to map pin names to ADC touch-key channels (TK0–TK9)
- `initTouchButtons()` now configures PA0–PA3 on GPIOA and PB0–PB1 on GPIOB
- `IsTouched()` uses the ADC channel mapping internally; the external API (passing pin names) is unchanged

**`SPI.cpp`** *(new)* — Hardware SPI1 driver with PartialRemap2:
- AFIO remap enabled at init time (`GPIO_PinRemapConfig(GPIO_PartialRemap2_SPI1, ENABLE)`)
- Mode 0 (CPOL=0, CPHA=0)
- Initial speed ~187 kHz (prescaler 256) for SD card init
- Manual CS control via PA12
- `spi_transfer(data)`, `spi_cs_low()`, `spi_cs_high()`

**`I2C.cpp`** — Updated for I2C1 PartialRemap1 on v2:
- AFIO remap enabled at init time (`GPIO_PinRemapConfig(GPIO_PartialRemap1_I2C1, ENABLE)`)
- v2 pins: SCL=PA13, SDA=PA14
- v1 pins unchanged: SCL=PA10, SDA=PA11

### New Libraries

#### `lib/phsi245_gfx/` — Standalone Graphics Library

Decoupled from the Arduboy2 game engine. Provides:

| Function | Description |
|---|---|
| `gfx::init()` | Initialize SSD1306 OLED (128×64) over I2C |
| `gfx::clear(color)` | Clear the frame buffer |
| `gfx::display()` | Flush frame buffer to display |
| `gfx::display(clear)` | Flush and optionally clear buffer |
| `gfx::drawPixel(x, y, color)` | Set a single pixel |
| `gfx::getPixel(x, y)` | Read a pixel |
| `gfx::drawLine(x0, y0, x1, y1, color)` | Bresenham line |
| `gfx::drawFastVLine(x, y, h, color)` | Vertical line |
| `gfx::drawFastHLine(x, y, w, color)` | Horizontal line |
| `gfx::drawRect(x, y, w, h, color)` | Rectangle outline |
| `gfx::fillRect(x, y, w, h, color)` | Filled rectangle |
| `gfx::drawCircle(x0, y0, r, color)` | Circle outline |
| `gfx::fillCircle(x0, y0, r, color)` | Filled circle |
| `gfx::drawBitmap(x, y, data, w, h, color)` | PROGMEM bitmap |
| `gfx::setCursor(x, y)` | Text cursor position |
| `gfx::setTextSize(s)` | Text scale factor |
| `gfx::setTextColor(c)` | Text color (BLACK/WHITE) |
| `gfx::print(str)` | Print C string with 5×7 font |
| `gfx::print(n)` | Print signed/unsigned 16-bit integer |

Memory: 1024-byte frame buffer (RAM) + 480-byte font (flash).

#### `lib/phsi245_storage/` — Block Storage Library

**Block Device Interface** (`storage.h`):

```cpp
struct BlockDevice {
    bool     (*init)();
    bool     (*readBlock)(uint32_t block, uint8_t *buf);
    bool     (*writeBlock)(uint32_t block, const uint8_t *buf);
    uint32_t (*blockCount)();
};
```

**SD Card Driver** (`sd_spi.cpp`, namespace `sd`):
- SPI mode initialization (CMD0 → CMD8 → ACMD41 → CMD58)
- SDHC/SDXC detection (HCS bit in OCR)
- Automatic block addressing (byte-addressed for SDSC, sector-addressed for SDHC)
- Capacity via CSD register parsing (v1.0 and v2.0)
- Block read (CMD17) and write (CMD24)
- Exports `sd::DEVICE` singleton

**FAT Filesystem** (`fat.cpp`, namespace `fat`):

Read support:
- `fat::mount(&dev)` — Mount a block device, parse MBR/BPB, detect FAT16/FAT32
- `fat::open("path/to/file")` — Open a file for reading (supports subdirectories)
- `fat::read(buf, len, &bytesRead)` — Read bytes from open file (streaming)
- `fat::close()` — Close the current file
- `fat::openDir("path")` — Open a directory for listing
- `fat::readDir(name, &isDir, &size)` — Iterate directory entries

Write support:
- `fat::create("path")` — Create a new file or truncate an existing one
- `fat::write(buf, len, &bytesWritten)` — Append data to an open file

Limitations:
- 8.3 filenames only (no LFN)
- Single-sector buffer (512 bytes)
- Single open file at a time
- No deletion or rename
- No date/time stamps on created files

Memory: 512-byte sector buffer (RAM, shared between FAT and directory operations).

### Demo Application

`src_games/OSDemo/OSDemo.ino` — Demonstrates both libraries:
1. Initializes graphics and shows system info
2. Attempts to mount SD card
3. Menu with A (list files) and B (read TEST.TXT)

### Build Verification

All code compiles cleanly (zero warnings) against the CH32V Arduino core:

```
RAM:   1,944 / 20,480 bytes (9.5%)
Flash: 10,860 / 63,488 bytes (17.1%)
```

Existing games (e.g., HelloNOISE) also compile successfully with the updated HAL.

### Architecture

```
┌──────────────────────────────────────┐
│           Game / Application         │
├──────────────────────────────────────┤
│  phsi245_gfx         │ phsi245_storage │
│  ┌─────────────┐    │ ┌──────────────┐│
│  │ gfx::buffer │    │ │ fat::mount   ││
│  │ drawPixel   │    │ │ fat::open    ││
│  │ drawCircle  │    │ │ fat::read    ││
│  │ print       │    │ │ fat::write   ││
│  │ display     │    │ │ sd::DEVICE   ││
│  └─────────────┘    │ └──────────────┘│
├──────────────────────────────────────┤
│  PHSI245_HAL (I2C, SPI, Touch, GPIO) │
└──────────────────────────────────────┘
```

### Migration Guide

To adopt the v2 libraries in a game:

```cpp
#include <HAL.h>
#include "gfx.h"
#include "storage.h"

void setup() {
    gfx::init();
    gfx::setTextColor(GFX_WHITE);

    if (fat::mount(&sd::DEVICE) == fat::OK) {
        // SD card ready
    }
}

void loop() {
    gfx::clear();

    // Drawing using gfx:: namespace
    gfx::drawPixel(10, 10, GFX_WHITE);
    gfx::fillRect(20, 20, 30, 30, GFX_WHITE);
    gfx::setCursor(0, 0);
    gfx::print("Hello v2!");

    // Read a file
    if (fat::open("DATA.BIN") == fat::OK) {
        uint8_t buf[64];
        uint16_t bytesRead;
        fat::read(buf, sizeof(buf), &bytesRead);
        fat::close();
    }

    gfx::display();

    // Buttons still work the same way
    if (IsTouched(PIN_BTN_A)) { /* ... */ }
}
```

### To Build

```bash
# Edit platformio.ini: set src_dir to the target
pio run
```

The existing Arduboy2-based games remain compatible. The `PHIS245_HAL` changes are backward-compatible at the API level — `IsTouched(PIN_BTN_A)` and `IsTouched(PIN_BTN_B)` work identically, they just read from different physical pins.
