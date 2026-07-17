#pragma once

#include <stdint.h>
#include <stdbool.h>

namespace storage {

// Initialize the save area (call once during setup).
// Uses internal flash option bytes (26 bytes available on CH32X035).
void initSave();

// Save a blob identified by a 4-byte tag.
// Returns true on success, false if the blob is too large.
// Max total storage across all tags is ~20 bytes.
bool saveGame(const char tag[4], const void *data, uint8_t len);

// Load a blob identified by tag. Fills `buf` with up to `maxLen` bytes.
// Returns the number of bytes loaded, or 0 if nothing was saved under this tag.
uint8_t loadGame(const char tag[4], void *buf, uint8_t maxLen);

// Erase saved data for a tag.
void eraseGame(const char tag[4]);

// High score helpers (uses the tag "HISC" automatically).
uint16_t highScoreLoad();
void     highScoreSave(uint16_t score);

} // namespace storage
