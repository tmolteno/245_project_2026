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

// --- Diagnostics (for "SD not detected" debugging) ---
// Filled in by the most recent init() attempt:
//   r1Cmd0        R1 from CMD0 (0x01 = idle OK)
//   r1Cmd8        R1 from CMD8; 0x02 = R7 voltage-pattern mismatch
//   r1Acmd41      FINAL R1 from ACMD41 loop (0x00 = card ready)
//   r1Acmd41First FIRST R1 from ACMD41 (0xFF = never responded at all)
//   busyCount     number of ACMD41 replies that were 0x01 (card was
//                 initializing) before the response vanished
//   stage         last init stage reached (SD_STAGE_*)
//   sdhc          OCR CCS bit (1 = SDHC/SDXC)
//   blocksHi      blocks >> 16 (0 if CSD parse failed)
struct SdDiag {
    uint8_t r1Cmd0;
    uint8_t r1Cmd8;
    uint8_t r1Acmd41;
    uint8_t r1Acmd41First;
    uint8_t busyCount;
    uint8_t stage;
    uint8_t sdhc;
    uint16_t blocksHi;
};

// Init-stage markers (match sd_spi.cpp)
enum {
    SD_STAGE_NONE   = 0,
    SD_STAGE_CMD0   = 1,
    SD_STAGE_CMD8   = 2,
    SD_STAGE_ACMD41 = 3,
    SD_STAGE_CSD    = 4,
    SD_STAGE_READY  = 5,
};

const SdDiag& getDiag();

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
