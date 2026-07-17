#include "eeprom_save.h"
#include <string.h>
#include <EEPROM.h>

namespace storage {

// Magic bytes to validate the save area has been initialized
static const char MAGIC[4] = {'P', 'H', '2', '5'};

// Total EEPROM size on CH32X035 (option bytes area)
static const uint8_t EEPROM_SIZE = 26;

// We use bytes 4..25 for user data (22 bytes available)
static const uint8_t DATA_OFFSET = 4;

static EEPROMClass eeprom;

void initSave()
{
    eeprom.begin();

    // Check if save area has been initialized
    bool initialized = true;
    for (uint8_t i = 0; i < 4; i++) {
        if (eeprom.read(i) != MAGIC[i]) {
            initialized = false;
            break;
        }
    }

    if (!initialized) {
        // Write magic and clear the data area
        for (uint8_t i = 0; i < 4; i++)
            eeprom.write(i, MAGIC[i]);
        for (uint8_t i = DATA_OFFSET; i < EEPROM_SIZE; i++)
            eeprom.write(i, 0);
        eeprom.commit();
    }
}

static int8_t findTag(const char tag[4])
{
    // Scan the data area for a matching tag
    uint8_t pos = DATA_OFFSET;
    while (pos + 5 <= EEPROM_SIZE) {
        uint8_t len = eeprom.read(pos + 4);
        if (len == 0) break;  // end of used slots

        bool match = true;
        for (uint8_t i = 0; i < 4; i++) {
            if (eeprom.read(pos + i) != (uint8_t)tag[i]) {
                match = false;
                break;
            }
        }

        if (match) return pos;

        pos += 5 + len;  // tag(4) + len(1) + data(len)
    }
    return -1;  // not found
}

bool saveGame(const char tag[4], const void *data, uint8_t len)
{
    if (len == 0 || len > (EEPROM_SIZE - DATA_OFFSET - 5))
        return false;

    // Find existing entry or the end of used data
    int8_t existing = findTag(tag);
    uint8_t pos;

    if (existing >= 0) {
        // Overwrite existing entry if it fits
        uint8_t oldLen = eeprom.read(existing + 4);
        if (len <= oldLen) {
            pos = existing;
        } else {
            // New data is larger — erase and re-create at end
            eraseGame(tag);
            pos = DATA_OFFSET;
            while (pos + 5 <= EEPROM_SIZE) {
                if (eeprom.read(pos + 4) == 0) break;
                pos += 5 + eeprom.read(pos + 4);
            }
        }
    } else {
        // Find end of data
        pos = DATA_OFFSET;
        while (pos + 5 <= EEPROM_SIZE) {
            if (eeprom.read(pos + 4) == 0) break;
            pos += 5 + eeprom.read(pos + 4);
        }
    }

    // Check if it fits
    if (pos + 5 + len > EEPROM_SIZE)
        return false;

    // Write tag
    for (uint8_t i = 0; i < 4; i++)
        eeprom.write(pos + i, tag[i]);

    // Write length
    eeprom.write(pos + 4, len);

    // Write data
    for (uint8_t i = 0; i < len; i++)
        eeprom.write(pos + 5 + i, ((const uint8_t *)data)[i]);

    return eeprom.commit();
}

uint8_t loadGame(const char tag[4], void *buf, uint8_t maxLen)
{
    int8_t pos = findTag(tag);
    if (pos < 0) return 0;

    uint8_t len = eeprom.read(pos + 4);
    if (len > maxLen) len = maxLen;

    for (uint8_t i = 0; i < len; i++)
        ((uint8_t *)buf)[i] = eeprom.read(pos + 5 + i);

    return len;
}

void eraseGame(const char tag[4])
{
    int8_t pos = findTag(tag);
    if (pos < 0) return;

    uint8_t len = eeprom.read(pos + 4);

    // Shift remaining data down to fill the gap
    uint8_t gap   = 5 + len;
    uint8_t start = pos;
    uint8_t nextPos = pos + gap;
    uint8_t end = DATA_OFFSET;
    while (end + 5 <= EEPROM_SIZE) {
        if (eeprom.read(end + 4) == 0) break;
        end += 5 + eeprom.read(end + 4);
    }

    while (nextPos < end) {
        eeprom.write(start, eeprom.read(nextPos));
        start++;
        nextPos++;
    }

    // Zero out the freed space at the end
    while (start < end) {
        eeprom.write(start, 0);
        start++;
    }

    eeprom.commit();
}

uint16_t highScoreLoad()
{
    uint16_t score = 0;
    loadGame("HISC", &score, sizeof(score));
    return score;
}

void highScoreSave(uint16_t score)
{
    saveGame("HISC", &score, sizeof(score));
}

} // namespace storage
