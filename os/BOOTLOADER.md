# OS Bootloader

The bootloader (`src/main.cpp`) is the main firmware for the PHSI245 Game Boy.
It provides a loading screen, SD card file browser, and a main menu with
sensor readouts, touch calibration, SD card formatting, and restart.

## Screens and State Machine

```
                     ┌──────────────┐
                     │  LED Blink   │  3 LED flashes
                     │  Test        │
                     └──────┬───────┘
                            │
                     ┌──────▼───────┐
                     │   Loading    │  Progress bar animation
                     │              │  SD card mount
                     └──┬───────┬───┘
                        │       │
              SD OK ┌───┘       └───┐ No SD
                    │               │
           ┌────────▼────────┐  ┌───▼──────────┐
           │    Browse       │  │  No SD Card   │
           │  File list      │  │  Info screen  │
           │  A:Open B:Menu  │  │  A:Menu       │
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
                    │  1. Sensors                 │
                    │  2. Calibrate Touch         │
                    │  3. Format SD Card          │
                    │  4. Restart                 │
                    └─────────────────────────────┘
```

### LED Blink Test

On startup, the LED (PB12) blinks 3 times. This confirms the MCU is alive
before display initialization begins. (The buzzer is currently disabled for
debugging.)

### Loading Screen

An animated progress bar fills from 0–100%. During the 30–60% phase, the
bootloader attempts to mount the SD card:

- **SD card present**: proceeds to the file browser
- **No SD card**: shows the "No SD Card" info screen

Before the animation, a one-second SD diagnostic readout is shown (SD status,
CMD0/CMD8/ACMD41 response bytes, and mount result).

### No SD Card

Shows a message explaining the situation. Press **A** to open the main menu.

### File Browser

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

| Option | Description |
|--------|-------------|
| **Sensors** | Live phototransistor (PA5) and NTC thermistor (PA6) readings |
| **Calibrate Touch** | Re-run touch-button calibration |
| **Format SD Card** | Erase and re-create the FAT filesystem (only shown when an SD card is present) |
| **Restart** | Full MCU reset via `NVIC_SystemReset()` |

### Hardware Reset

This board has **no reset button** (PA21/NRST is not connected), so the only
way to reset is the "Restart" menu option (`NVIC_SystemReset()`) or by
power-cycling the board.

## Building

```bash
make build
make upload
```

Builds for the v2 hardware (default `ENV=v2` in the Makefile). See
`platformio.ini` for the build environment and `lib/PHSI245_HAL/HAL.h`
for the pin mapping.

## OS Libraries Used

The bootloader uses the full OS library stack:

| Library | Namespace | Purpose |
|---------|-----------|---------|
| `phsi245_gfx` | `gfx` | 128×64 OLED display: pixels, shapes, sprites, text |
| `phsi245_input` | `input` | Button polling with edge detection |
| `phsi245_led` | `led` | LED on/off/toggle control |
| `phsi245_timer` | `ostime` | Millisecond tick counter and delay |
| `phsi245_beep` | `beep` | Buzzer audio feedback |
| `phsi245_storage` | `fat`, `sd` | FAT filesystem and SD card |
| `PHSI245_HAL` | *(global)* | Hardware abstraction: GPIO, I2C, SPI, touch keys |

## Audio Feedback

| Pattern | Meaning |
|---------|---------|
| 1 beep before display init | MCU alive, starting display init |
| 1 beep after display init | Display init completed successfully |
| 3 rapid beeps | SD card error (no card, mount failure) |

**Note:** the buzzer is currently disabled for debugging (all `beep_*` calls
are commented out in `src/main.cpp`), so no sound is produced. No LED blink at
all means the MCU isn't running (power or clock issue).

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
