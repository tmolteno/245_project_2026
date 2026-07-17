---
name: monochrome-sprite-library
description: Add a sprite system with multiple draw modes (overwrite, masked, self-masked, erase, plus-mask) to a 1-bit monochrome graphics library, including data-first multi-frame sprite sheets
source: auto-skill
extracted_at: '2026-07-16T10:30:00.000Z'
---

# Monochrome Sprite Library

Add masked sprite drawing to a 1-bit (monochrome) graphics library that uses
vertical-byte-packed buffers (8 vertical pixels per byte, LSB at the top). The
sprite data itself uses the conventional horizontal-byte-packed format (MSB on the
left), matching what bitmap editors and tools like `drawBitmap` expect.

## Draw modes

Define five modes as preprocessor constants rather than enums (to match the
existing code style of monochrome embedded graphics libraries):

```cpp
#define GFX_SPRITE_UNMASKED    0   // overwrite: image copied directly
#define GFX_SPRITE_MASKED      1   // external mask: separate image+mask arrays
#define GFX_SPRITE_PLUS_MASK   2   // interleaved image+mask byte pairs
#define GFX_SPRITE_SELF_MASKED 3   // self-masked: 1=draw, 0=transparent
#define GFX_SPRITE_ERASE       4   // erase: 1=clear, 0=transparent
```

Each mode modifies how the buffer is written at the per-pixel level:

| Mode | Bit meaning | Buffer operation |
|------|-------------|------------------|
| UNMASKED | Image bit | `buf = (buf & ~bit) \| (img & bit)` — direct overwrite |
| SELF_MASKED | Image bit=1 → draw | `if (img) buf \|= bit` — set only, never clear |
| ERASE | Image bit=1 → clear | `if (img) buf &= ~bit` — clear only, never set |
| MASKED | Mask bit=1 → write image | `if (mask) { if (img) buf \|= bit; else buf &= ~bit; }` |
| PLUS_MASK | Same as MASKED | Interleaved (img,mask) byte pairs in the sprite data |

## Two API layers: raw and data-first

### Raw format (explicit width/height)

Mirror the pattern of the existing `drawBitmap` function. The caller provides
width and height directly; the bitmap data is pure pixel data with no header:

```cpp
void drawSprite(int16_t x, int16_t y, const uint8_t *bitmap,
                int16_t w, int16_t h, uint8_t mode = GFX_SPRITE_UNMASKED);

void drawSpriteMasked(int16_t x, int16_t y, const uint8_t *bitmap,
                      const uint8_t *mask, int16_t w, int16_t h);
```

### Data-first format (width/height in first 2 bytes)

For multi-frame sprite sheets and compatibility with Arduboy2 tooling. The sprite
data begins with 2 header bytes (width, height), followed by frame data. Each
frame is `ceil(width/8) * height` bytes of pixel data. Frames are packed
sequentially — frame N starts at `sheet + 2 + N * frameStride`:

```cpp
void drawSpriteSheet(int16_t x, int16_t y, const uint8_t *sheet, uint8_t frame,
                     uint8_t mode = GFX_SPRITE_UNMASKED);

void drawSpriteSheetMasked(int16_t x, int16_t y, const uint8_t *sheet,
                           const uint8_t *maskSheet, uint8_t frame,
                           uint8_t maskFrame = 0);

void drawPlusMask(int16_t x, int16_t y, const uint8_t *sheet, uint8_t frame);
```

## Critical: avoid C++ overload ambiguity

The raw `drawSprite(x, y, bitmap, w, h, mode)` takes 6 parameters ending with
`int16_t, int16_t, uint8_t`. The data-first `drawSprite(x, y, sheet, frame, mode)`
takes 5 parameters ending with `uint8_t, uint8_t`. These are distinct enough
(6 vs 5 args) that they don't collide.

**But** the masked variants both take 6 parameters — raw ends with `int16_t, int16_t`
and data-first ends with `uint8_t, uint8_t`. A call like `drawSpriteMasked(x, y,
sheet, maskSheet, 0, 0)` is ambiguous because the `int` literal `0` matches both
`int16_t` and `uint8_t`.

**Solution**: use different function names for data-first variants:
- Raw: `drawSprite`, `drawSpriteMasked`
- Data-first: `drawSpriteSheet`, `drawSpriteSheetMasked`, `drawPlusMask`

This avoids any overload ambiguity while keeping the naming self-documenting.

## Implementation: per-pixel with direct buffer access

Use per-pixel iteration with direct buffer manipulation (not calling `drawPixel`)
for reasonable speed on microcontrollers:

```cpp
void drawSprite(int16_t x, int16_t y, const uint8_t *bitmap,
                int16_t w, int16_t h, uint8_t mode) {
    int16_t byteWidth = (w + 7) / 8;

    for (int16_t j = 0; j < h; j++) {
        int16_t py = y + j;
        if (py < 0 || py >= GFX_HEIGHT) continue;
        uint16_t bufRowOff = (py >> 3) * GFX_WIDTH;  // (y/8) * screen_width
        uint8_t  bufBit    = 1 << (py & 7);           // bit within the column byte

        for (int16_t i = 0; i < w; i++) {
            int16_t px = x + i;
            if (px < 0 || px >= GFX_WIDTH) continue;

            // Read sprite bit: MSB-first horizontal packing
            uint8_t spriteByte = pgm_read_byte(bitmap + (j * byteWidth) + (i >> 3));
            uint8_t spriteBit  = spriteByte & (0x80 >> (i & 7));

            if (!spriteBit) continue;  // nothing to draw for non-masked modes

            uint16_t bufIdx = bufRowOff + px;
            switch (mode) {
            case GFX_SPRITE_UNMASKED:
                if (spriteBit) buffer[bufIdx] |= bufBit;
                else           buffer[bufIdx] &= ~bufBit;
                break;
            case GFX_SPRITE_SELF_MASKED:
                if (spriteBit) buffer[bufIdx] |= bufBit;
                break;
            case GFX_SPRITE_ERASE:
                if (spriteBit) buffer[bufIdx] &= ~bufBit;
                break;
            }
        }
    }
}
```

### Buffer addressing formula (vertical byte packing)

```
byte_index = ((y >> 3) * SCREEN_WIDTH) + x
bit        = 1 << (y & 7)              // LSB = top row
```

### Sprite data addressing (horizontal byte packing)

```
byte_index = (row * byteWidth) + (col >> 3)
bit        = 0x80 >> (col & 7)         // MSB = leftmost pixel
```

## Plus-mask special case

Plus-mask sprites have interleaved image and mask bytes. The row stride is
`2 * byteWidth` instead of `byteWidth`. At column `i` in row `j`, the image byte
is at `frameData + (j * 2 * byteWidth) + ((i>>3) * 2)` and the mask byte is at
the next address. This mode has its own dedicated function `drawPlusMask` rather
than being handled through the generic `drawSprite` — because the data layout
is fundamentally different from single-array sprites.

## Edge cases

- **Zero-size sprites**: return immediately if `w <= 0 || h <= 0`
- **Partial off-screen**: per-pixel bounds check `if (px < 0 || px >= WIDTH) continue`
  handles sprites that straddle the display edge without requiring pre-clipping
- **PROGMEM access**: always use `pgm_read_byte()` for sprite data; sprite sheets
  live in flash, not RAM
- **Transparent pixels in unmasked mode**: an unmasked 0 bit explicitly clears
  the buffer pixel (sets it to black). This differs from self-masked mode where
  a 0 bit leaves the buffer unchanged
