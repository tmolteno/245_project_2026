# OS Bootloader

The bootloader (`src/main.cpp`) is the main firmware for the PHSI245 Game Boy.
It provides a loading screen, SD card file browser (v2), and a main menu with a
pong game, touch calibration, and restart capability.

## Screens and State Machine

```
                     ┌──────────────┐
                     │  Blink Test  │  3 LED flashes
                     │  + Beep      │  1 startup beep
                     └──────┬───────┘
                            │
                     ┌──────▼───────┐
                     │   Loading    │  Progress bar animation
                     │              │  SD card mount (v2)
                     └──┬───────┬───┘
                        │       │
              SD OK ┌───┘       └───┐ No SD / v1
                    │               │
           ┌────────▼────────┐  ┌───▼──────────┐
           │    Browse (v2)  │  │  No SD Card   │  Error beep (3x)
           │  File list      │  │  Info screen  │
           │  A:Open B:Menu  │  │  B:Menu       │
           └──┬───────┬──────┘  └───┬───────────┘
              │       │             │
    A:Open ┌──┘       └── B:Menu ──┤
           │                       │
   ┌───────▼────────┐              │
   │   File Info    │              │
   │   A:Exec       │              │
   │   B:Back       │              │
   └───────┬────────┘              │
           │                       │
   ┌───────▼────────┐              │
   │   Executing    │              │
   │   Header hex   │              │
   │   B:Back       │              │
   └────────────────┘              │
                                   │
                    ┌──────────────▼──────────────┐
                    │         Main Menu           │
                    │  1. Play Pong               │
                    │  2. Calibrate Touch         │
                    │  3. Restart                 │
                    └──┬──────────┬──────────┬────┘
                       │          │          │
              ┌────────▼──┐  ┌────▼──────┐  ┌▼─────────┐
              │   Pong    │  │ Calibrate │  │  Reset   │
              │  B:Quit   │  │  B:Back   │  │  MCU     │
              └───────────┘  └───────────┘  └──────────┘
```

### Blink Test

On startup, the LED (PB12) blinks 3 times and the buzzer beeps once. This
confirms the MCU is alive before display initialization begins.

### Loading Screen

An animated progress bar fills from 0–100%. During the 30–60% phase, the
bootloader attempts to mount an SD card (v2 hardware only):

- **v2 with SD card**: proceeds to the file browser
- **v2 without SD card**: shows the "No SD Card" error screen with 3 error beeps
- **v1 hardware**: skips SD entirely, shows a hardware info screen

### No SD Card / v1 Info

Shows a message explaining the situation. Press **B** to open the main menu.

### File Browser (v2 only)

Lists files and directories from the SD card root:

| Button | Action |
|--------|--------|
| UP / DOWN | Navigate file list |
| A | Open file (shows file info) |
| B | Open main menu |

### File Info

Shows the selected file's name, type, and size:

| Button | Action |
|--------|--------|
| A | Execute (shows file header as hex) |
| B | Back to file browser |

### Main Menu

Three options available on both v1 and v2:

| Option | Description |
|--------|-------------|
| **Play Pong** | Single-player pong — UP/DOWN moves paddle, score counter, B quits |
| **Calibrate Touch** | Live display of raw ADC values for all 6 touch buttons |
| **Restart** | Full MCU reset via `NVIC_SystemReset()` |

### Hardware Reset (v2)

v2 hardware includes an external reset button on **PA21 (NRST, pin 7)**.
Momentarily pulling NRST to GND triggers a full hardware reset that works
regardless of firmware state. No firmware configuration is needed — the
CH32X035 defaults PA21 to active-low reset input.

The "Restart" menu option provides a software-only reset for v1 hardware
or when the physical button is not accessible.

## Building

```bash
# v2 hardware (default, with SD card)
make build
make upload

# v1 hardware (original, no SD card)
make build-v1
make upload-v1

# Build both
pio run -e v1 -e v2
```

The hardware version is controlled by the `HW_VERSION` build flag (1 or 2,
default 2). See `platformio.ini` for build environments and `lib/PHSI245_HAL/HAL.h`
for the pin mapping details.

## OS Libraries Used

The bootloader uses the full OS library stack:

| Library | Namespace | Purpose |
|---------|-----------|---------|
| `phsi245_gfx` | `gfx` | 128×64 OLED display: pixels, shapes, sprites, text |
| `phsi245_input` | `input` | Button polling with edge detection |
| `phsi245_led` | `led` | LED on/off/toggle control |
| `phsi245_timer` | `ostime` | Millisecond tick counter and delay |
| `phsi245_beep` | `beep` | Buzzer audio feedback |
| `phsi245_storage` | `fat`, `sd` | FAT filesystem and SD card (v2 only) |
| `PHSI245_HAL` | *(global)* | Hardware abstraction: GPIO, I2C, SPI, touch keys |

## Audio Feedback

| Pattern | Meaning |
|---------|---------|
| 1 beep before display init | MCU alive, starting display init |
| 1 beep after display init | Display init completed successfully |
| 3 rapid beeps | SD card error (no card, mount failure) |

No beep at all means the MCU isn't running (power or clock issue). One beep
followed by silence means the display init is hanging.

## Program Loading

### Overview

The bootloader's file browser allows the user to select a file from the SD
card and "execute" it. In the current implementation, execution is simulated:
the bootloader reads and displays the file's header bytes rather than actually
loading the binary into flash or RAM. This provides a foundation for future
flash-programming or relocatable-loader support.

### Flow

```
Browse ──A──► File Info ──A──► Executing
  ▲              │                │
  │              │                │
  └─────B────────┘       B────────┘
```

1. **Browse** — User navigates the file list with UP/DOWN, presses **A** to
   inspect the selected file.
2. **File Info** — Shows the file's name, type (file or directory), and size.
   Press **A** to "execute" (directories are ignored).
3. **Executing** — The bootloader opens the file via the FAT filesystem
   library, reads the first 16 bytes (the header), and displays them as a hex
   dump on screen. A progress bar fills to 100%. Press **B** to return to the
   file browser.

### Header Reader

The current execution path in `src/main.cpp` (`STATE_EXECUTING`):

```cpp
fat::Result r = fat::open(path);      // open file by name
uint8_t header[16];
uint16_t br;
r = fat::read(header, 16, &br);       // read first 16 bytes
fat::close();

// display each byte as hex on screen
```

Files are opened from the SD card root (`/filename`). The FAT library handles
both FAT16 and FAT32 volumes, including 8.3 filename lookup and subdirectory
traversal (though the browser currently only scans the root directory).

### Memory Layout (Future)

When real program loading is implemented, the target memory regions are:

| Region | Start | Size | Notes |
|--------|-------|------|-------|
| Flash (CodeFlash) | `0x08000000` | 62 KB | Application + bootloader share this space |
| RAM (SRAM) | `0x20000000` | 20 KB | Stack, heap, and .data/.bss |

The bootloader occupies the beginning of flash (`0x08000000`). A future
program loader would need to:

1. Reserve a fixed bootloader region (e.g. 16 KB at `0x08000000–0x08003FFF`)
2. Load applications to `0x08004000` and above
3. Read the binary from SD card via `fat::read()` in 512-byte blocks
4. Program flash pages using the on-chip Flash controller
5. Verify with a checksum
6. Jump to the application entry point (reset vector at offset `+4`)

The CH32X035 flash controller requires:
- Unlocking via `FLASH_Unlock()`
- Erasing pages (1 KB each) before writing
- Programming in 32-bit words
- Locking afterwards with `FLASH_Lock()`

### Binary Format (Planned)

Executable files should follow a simple header format so the bootloader can
validate them before programming:

```
Offset  Size  Field
------  ----  -----
0x00    4     Magic number (e.g. 0x50 0x48 0x32 0x35 = "PH25")
0x04    4     Entry point address
0x08    4     Load size (bytes)
0x0C    4     Checksum (CRC32 over payload)
0x10    N     Raw binary payload
```

The magic number prevents accidental execution of non-program files. The
entry point, size, and checksum allow the bootloader to verify integrity
before flashing.
