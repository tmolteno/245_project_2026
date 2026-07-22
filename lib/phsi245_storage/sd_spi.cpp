#include "storage.h"
#include <HAL.h>
#include <Arduino.h>
#include "gfx.h"

#if HW_VERSION == 2

// SD card commands (SPI mode)
#define CMD0    0x00  // GO_IDLE_STATE
#define CMD1    0x01  // SEND_OP_COND
#define CMD8    0x08  // SEND_IF_COND
#define CMD9    0x09  // SEND_CSD
#define CMD10   0x0A  // SEND_CID
#define CMD12   0x0C  // STOP_TRANSMISSION
#define CMD16   0x10  // SET_BLOCKLEN
#define CMD17   0x11  // READ_SINGLE_BLOCK
#define CMD24   0x18  // WRITE_BLOCK
#define CMD55   0x37  // APP_CMD
#define CMD58   0x3A  // READ_OCR
#define ACMD41  0x29  // SD_SEND_OP_COND

namespace sd {

static bool sdhc = false;    // true if SDHC/SDXC card
static uint32_t blocks = 0;  // total block count

// Send a command and get R1 response
static uint8_t sdCommand(uint8_t cmd, uint32_t arg)
{
    uint8_t buf[6];
    buf[0] = cmd | 0x40;
    buf[1] = arg >> 24;
    buf[2] = arg >> 16;
    buf[3] = arg >> 8;
    buf[4] = arg;
    buf[5] = (cmd == CMD0) ? 0x95 :
             (cmd == CMD8) ? 0x87 : 0x01; // CRC (only matters for CMD0 and CMD8)

    for (int i = 0; i < 6; i++)
        spi_transfer(buf[i]);

    // Wait for non-0xFF response (up to 8 bytes)
    uint8_t r1;
    for (int i = 0; i < 8; i++) {
        r1 = spi_transfer(0xFF);
        if (!(r1 & 0x80)) break;
    }
    return r1;
}

uint8_t spiRecv()
{
    return spi_transfer(0xFF);
}

bool init()
{
    spi_init();

    // --- Debug: show init start ---
    gfx::clear();
    gfx::setCursor(0, 0);
    gfx::print("SD Init...");
    gfx::display();

    // --- Debug: verify CS pin toggles correctly ---
    spi_cs_high();
    delay(1);
    int csHigh = (GPIOA->INDR & GPIO_Pin_12) ? 1 : 0;  // read actual pin state

    spi_cs_low();
    delay(1);
    int csLow  = (GPIOA->INDR & GPIO_Pin_12) ? 1 : 0;

    spi_cs_high();  // leave CS high for clock pulses

    gfx::setCursor(0, 10);
    gfx::print("CS H:");
    gfx::print((int16_t)csHigh);
    gfx::print(" L:");
    gfx::print((int16_t)csLow);
    gfx::display();

    if (csHigh != 1 || csLow != 0) {
        gfx::setCursor(0, 20);
        gfx::print("CS PIN BAD!");
        gfx::display();
        delay(5000);
        return false;
    }

    // Send >= 74 clock pulses with CS high
    spi_cs_high();
    for (int i = 0; i < 10; i++)
        spi_transfer(0xFF);

    // CMD0: go idle
    spi_cs_low();
    uint8_t r1 = sdCommand(CMD0, 0);
    spi_cs_high();
    spi_transfer(0xFF);

    gfx::setCursor(0, 30);
    gfx::print("CMD0: ");
    gfx::print((int16_t)r1);
    gfx::display();

    if (r1 != 0x01) {
        gfx::setCursor(0, 40);
        gfx::print("FAIL (no card?)");
        gfx::display();
        delay(3000);
        return false;
    }

    // CMD8: check voltage range (2.7-3.6V)
    spi_cs_low();
    r1 = sdCommand(CMD8, 0x000001AA);
    if (r1 == 0x01) {
        // R7 response: 4 more bytes
        uint8_t r7[4];
        for (int i = 0; i < 4; i++) r7[i] = spi_transfer(0xFF);
        spi_cs_high();
        spi_transfer(0xFF);

        gfx::setCursor(0, 20);
        gfx::print("CMD8 r7: ");
        gfx::print((int16_t)r7[2]);
        gfx::print(" ");
        gfx::print((int16_t)r7[3]);
        gfx::display();

        if (r7[2] != 0x01 || r7[3] != 0xAA) {
            gfx::setCursor(0, 30);
            gfx::print("FAIL (bad pattern)");
            gfx::display();
            delay(3000);
            return false;
        }
    } else if (r1 & 0x04) {
        // Illegal command — v1 card, fine
        spi_cs_high();
        spi_transfer(0xFF);
        gfx::setCursor(0, 20);
        gfx::print("v1 card (no CMD8)");
        gfx::display();
    } else {
        spi_cs_high();
        gfx::setCursor(0, 20);
        gfx::print("CMD8: ");
        gfx::print((int16_t)r1);
        gfx::display();
        gfx::setCursor(0, 30);
        gfx::print("FAIL");
        gfx::display();
        delay(3000);
        return false;
    }

    // ACMD41: initialize card
    gfx::setCursor(0, 30);
    gfx::print("ACMD41...");
    gfx::display();

    uint32_t timeout = 0;
    do {
        spi_cs_low();
        sdCommand(CMD55, 0);
        r1 = sdCommand(ACMD41, 0x40000000); // HCS bit for SDHC
        spi_cs_high();
        spi_transfer(0xFF);
        if (++timeout > 1000) {
            gfx::setCursor(0, 40);
            gfx::print("ACMD41 timeout");
            gfx::display();
            delay(3000);
            return false;
        }
    } while (r1 != 0x00);

    // CMD58: read OCR (check CCS bit for SDHC)
    spi_cs_low();
    r1 = sdCommand(CMD58, 0);
    if (r1 == 0x00) {
        uint8_t ocr[4];
        for (int i = 0; i < 4; i++) ocr[i] = spi_transfer(0xFF);
        sdhc = (ocr[0] & 0x40) != 0;
    }
    spi_cs_high();
    spi_transfer(0xFF);

    gfx::setCursor(0, 40);
    gfx::print("SDHC: ");
    gfx::print(sdhc ? "yes" : "no");
    gfx::display();

    // CMD16: set block length to 512
    spi_cs_low();
    sdCommand(CMD16, 512);
    spi_cs_high();
    spi_transfer(0xFF);

    // Read CSD to get block count
    uint8_t csd[16];
    spi_cs_low();
    r1 = sdCommand(CMD9, 0);
    if (r1 == 0x00) {
        timeout = 0;
        while (spi_transfer(0xFF) != 0xFE && ++timeout < 100);
        if (timeout < 100) {
            for (int i = 0; i < 16; i++)
                csd[i] = spi_transfer(0xFF);
            spi_transfer(0xFF); spi_transfer(0xFF);

            uint8_t csdVer = (csd[0] >> 6) & 0x03;
            if (csdVer == 0) {
                uint32_t csize = ((csd[6] & 0x03) << 10) | (csd[7] << 2) | ((csd[8] >> 6) & 0x03);
                uint8_t cmult = ((csd[9] & 0x03) << 1) | ((csd[10] >> 7) & 0x01);
                uint32_t readBlLen = csd[5] & 0x0F;
                blocks = (csize + 1) * (1UL << (cmult + 2)) * (1UL << (readBlLen - 9));
            } else if (csdVer == 1) {
                uint32_t csize = ((csd[7] & 0x3F) << 16) | (csd[8] << 8) | csd[9];
                blocks = (csize + 1) * 1024;
            }
        }
    }
    spi_cs_high();

    gfx::setCursor(0, 50);
    gfx::print("Blocks: ");
    gfx::print((int16_t)(blocks >> 16));
    gfx::print("K");
    gfx::display();

    if (blocks == 0) {
        gfx::setCursor(0, 56);
        gfx::print("CSD read FAIL");
        gfx::display();
        delay(3000);
    } else {
        gfx::setCursor(0, 56);
        gfx::print("SD OK!");
        gfx::display();
        delay(1000);
    }

    return blocks > 0;
}

bool readBlock(uint32_t block, uint8_t *buf)
{
    if (!sdhc)
        block <<= 9; // byte addressing for SDSC

    spi_cs_low();
    uint8_t r1 = sdCommand(CMD17, block);
    if (r1 != 0x00) {
        spi_cs_high();
        return false;
    }

    // Wait for start token 0xFE
    uint16_t timeout = 0;
    while (spi_transfer(0xFF) != 0xFE && ++timeout < 500);
    if (timeout >= 500) {
        spi_cs_high();
        return false;
    }

    // Read 512 bytes
    for (int i = 0; i < 512; i++)
        buf[i] = spi_transfer(0xFF);

    // Read and discard 2 CRC bytes
    spi_transfer(0xFF);
    spi_transfer(0xFF);

    spi_cs_high();
    // Send extra clock cycles
    spi_transfer(0xFF);
    return true;
}

bool writeBlock(uint32_t block, const uint8_t *buf)
{
    if (!sdhc)
        block <<= 9;

    spi_cs_low();
    uint8_t r1 = sdCommand(CMD24, block);
    if (r1 != 0x00) {
        spi_cs_high();
        return false;
    }

    // Send start token
    spi_transfer(0xFE);

    // Write 512 bytes
    for (int i = 0; i < 512; i++)
        spi_transfer(buf[i]);

    // Write dummy CRC (2 bytes)
    spi_transfer(0xFF);
    spi_transfer(0xFF);

    // Check data response token
    uint8_t resp = spi_transfer(0xFF);
    if ((resp & 0x1F) != 0x05) {
        spi_cs_high();
        return false;
    }

    // Wait for write to complete (card returns non-0xFF when busy)
    uint16_t timeout = 0;
    while (spi_transfer(0xFF) == 0x00 && ++timeout < 10000);
    spi_cs_high();
    spi_transfer(0xFF);

    return timeout < 10000;
}

uint32_t blockCount()
{
    return blocks;
}

const BlockDevice DEVICE = {
    init,
    readBlock,
    writeBlock,
    blockCount,
};

} // namespace sd

#endif // HW_VERSION == 2
