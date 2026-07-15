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
