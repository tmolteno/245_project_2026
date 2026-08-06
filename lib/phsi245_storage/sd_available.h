#pragma once

#include <stdbool.h>

namespace storage {

// Returns true if an SD card is present and mounted.
// On v1 hardware: always returns false (no SD slot).
// On v2 hardware: mounts the card on first call, returns result.
bool sdAvailable();

// Returns the last FAT mount result code (for diagnostics):
// -1 = never attempted, otherwise a value from `fat::Result` in storage.h.
int lastMountError();

} // namespace storage
