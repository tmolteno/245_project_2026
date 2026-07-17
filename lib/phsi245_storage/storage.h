#pragma once

#include <stdint.h>
#include <stdbool.h>

// --- Block Device Interface ---

struct BlockDevice {
    bool     (*init)();
    bool     (*readBlock)(uint32_t block, uint8_t *buf);
    bool     (*writeBlock)(uint32_t block, const uint8_t *buf);
    uint32_t (*blockCount)();
};

// --- SD Card (SPI mode) ---

namespace sd {

extern const BlockDevice DEVICE;

bool init();
bool readBlock(uint32_t block, uint8_t *buf);
bool writeBlock(uint32_t block, const uint8_t *buf);
uint32_t blockCount();

// Raw byte read (bypasses block buffer, for internal use)
uint8_t spiRecv();

} // namespace sd

// --- Minimal FAT Filesystem ---

namespace fat {

// Result codes
enum Result {
    OK = 0,
    DISK_ERR,
    NOT_READY,
    NO_FILE,
    NOT_OPENED,
    NOT_ENABLED,
    NO_FILESYSTEM,
};

Result mount(const BlockDevice *dev);
Result open(const char *path);
Result read(void *buf, uint16_t btr, uint16_t *br);
Result close();

// Directory listing: iterates entries in the current directory.
// Call openDir() first, then readDir() repeatedly until it returns false.
// Each call fills `name` (max 13 bytes: 8.3 + null) and `isDir`, `fileSize`.
Result openDir(const char *path);
bool   readDir(char *name, bool *isDir, uint32_t *fileSize);

// Write support: create/overwrite a file and write data
Result create(const char *path);
Result write(const void *buf, uint16_t btw, uint16_t *bw);

// Format the SD card with a fresh FAT32 filesystem.
// All existing data is lost. Returns OK on success.
Result format();

} // namespace fat

// EEPROM save/load API (high scores, game state)
#include "eeprom_save.h"

// SD card detection — call once during boot, returns true if a card is present.
// Handles v1 (no SD hardware, always false) and v2 (mounts and checks).
namespace storage {
bool sdAvailable();
}
