#include <Arduino.h>
#include "os_libs.h"

// --- Constants ---

#define MAX_FILES   64
#define LIST_Y      12
#define ITEM_H      8
#define MAX_VISIBLE 6

// --- File entry ---

#if HW_VERSION == 2
struct FileEntry {
    char name[13];
    bool isDir;
    uint32_t size;
};

static FileEntry files[MAX_FILES];
static uint8_t  fileCount = 0;
#endif

// --- UI State ---

enum State {
    STATE_LOADING,
    STATE_BROWSE,
    STATE_FILE_INFO,
    STATE_EXECUTING,
    STATE_ERROR,
    STATE_V1_INFO,
    STATE_MENU,
    STATE_PONG,
    STATE_CALIBRATE,
    STATE_FORMAT,
};

static State   state      = STATE_LOADING;
static uint8_t cursor     = 0;
#if HW_VERSION == 2
static uint8_t scrollOff  = 0;
#endif
static uint8_t loadingPct = 0;

// --- Helpers ---

static void drawHeader(const char *title)
{
    gfx::drawFastHLine(0, 10, GFX_WIDTH, GFX_WHITE);
    gfx::setCursor(1, 1);
    gfx::print(title);
}

static void drawProgressBar(uint8_t pct)
{
    int16_t barW = 100;
    int16_t barX = (GFX_WIDTH - barW) / 2;
    int16_t barY = 40;
    int16_t barH = 8;

    gfx::drawRect(barX - 1, barY - 1, barW + 2, barH + 2, GFX_WHITE);
    int16_t fill = (barW * pct) / 100;
    if (fill > 0)
        gfx::fillRect(barX, barY, fill, barH, GFX_WHITE);
}

#if HW_VERSION == 2
static void collectFiles()
{
    fileCount = 0;
    char name[13];
    bool isDir;
    uint32_t size;

    fat::openDir("/");
    while (fat::readDir(name, &isDir, &size) && fileCount < MAX_FILES) {
        FileEntry *e = &files[fileCount];
        for (uint8_t i = 0; i < 13; i++) {
            e->name[i] = name[i];
            if (name[i] == '\0') break;
        }
        e->isDir = isDir;
        e->size  = size;
        fileCount++;
    }
}
#endif

#if HW_VERSION == 2
static void formatSize(char *buf, uint32_t size)
{
    if (size < 1024) {
        uint8_t pos = 0;
        uint16_t n = (uint16_t)size;
        if (n >= 100) { buf[pos++] = '0' + (n / 100); n %= 100; }
        if (n >= 10 || size >= 100) { buf[pos++] = '0' + (n / 10); n %= 10; }
        buf[pos++] = '0' + n;
        buf[pos++] = 'B';
        buf[pos] = '\0';
    } else if (size < 1024 * 1024) {
        uint16_t k = (uint16_t)(size / 1024);
        uint8_t pos = 0;
        if (k >= 100) { buf[pos++] = '0' + (k / 100); k %= 100; }
        if (k >= 10)   { buf[pos++] = '0' + (k / 10); k %= 10; }
        buf[pos++] = '0' + k;
        buf[pos++] = 'K';
        buf[pos] = '\0';
    } else {
        buf[0] = '>'; buf[1] = '1'; buf[2] = 'M'; buf[3] = '\0';
    }
}
#endif

// --- Drawing ---

static void drawLoading()
{
    gfx::clear();
    gfx::setCursor(8, 22);
    gfx::print("PHSI245 OS Bootloader");

    drawProgressBar(loadingPct);

    gfx::setCursor(20, 52);
    gfx::print("Mounting SD card...");
    gfx::display();
}

#if HW_VERSION == 2
static void drawBrowse()
{
    gfx::clear();
    drawHeader("Files on SD:");

    if (fileCount == 0) {
        gfx::setCursor(2, LIST_Y);
        gfx::print("(empty)");
        gfx::display();
        return;
    }

    for (uint8_t i = 0; i < MAX_VISIBLE; i++) {
        uint8_t fi = scrollOff + i;
        if (fi >= fileCount) break;

        int16_t y = LIST_Y + i * ITEM_H;
        FileEntry *e = &files[fi];

        if (fi == cursor) {
            gfx::setCursor(0, y);
            gfx::print(">");
        }

        gfx::setCursor(8, y);
        if (e->isDir)
            gfx::print("[");
        gfx::print(e->name);
        if (e->isDir)
            gfx::print("]");

        if (!e->isDir) {
            char sz[6];
            formatSize(sz, e->size);
            gfx::setCursor(102, y);
            gfx::print(sz);
        }
    }

    if (fileCount > MAX_VISIBLE) {
        gfx::setCursor(GFX_WIDTH - 6, LIST_Y);
        gfx::print("+");
        if (scrollOff + MAX_VISIBLE < fileCount) {
            gfx::setCursor(GFX_WIDTH - 6, LIST_Y + (MAX_VISIBLE - 1) * ITEM_H);
            gfx::print("+");
        }
    }

    gfx::drawFastHLine(0, GFX_HEIGHT - 9, GFX_WIDTH, GFX_WHITE);
    gfx::setCursor(2, GFX_HEIGHT - 7);
    gfx::print("A:Open  B:Back");

    gfx::display();
}
#endif

#if HW_VERSION == 2
static void drawFileInfo()
{
    FileEntry *e = &files[cursor];

    gfx::clear();
    drawHeader("File Info");

    gfx::setCursor(2, LIST_Y);
    gfx::print("Name: ");
    gfx::print(e->name);

    gfx::setCursor(2, LIST_Y + 8);
    gfx::print("Type: ");
    if (e->isDir)
        gfx::print("Directory");
    else
        gfx::print("File");

    gfx::setCursor(2, LIST_Y + 16);
    gfx::print("Size: ");
    if (e->isDir) {
        gfx::print("--");
    } else {
        char sz[6];
        formatSize(sz, e->size);
        gfx::print(sz);
    }

    gfx::drawFastHLine(0, GFX_HEIGHT - 9, GFX_WIDTH, GFX_WHITE);
    gfx::setCursor(2, GFX_HEIGHT - 7);
    gfx::print("A:Exec  B:Back");

    gfx::display();
}
#endif

#if HW_VERSION == 2
static void drawExecuting()
{
    FileEntry *e = &files[cursor];

    gfx::clear();
    drawHeader("Executing...");

    gfx::setCursor(2, LIST_Y);
    gfx::print(e->name);

    if (!e->isDir && storage::sdAvailable()) {
        char path[64];
        path[0] = '/';
        uint8_t pos = 1;
        for (uint8_t i = 0; e->name[i] && pos < 63; i++)
            path[pos++] = e->name[i];
        path[pos] = '\0';

        fat::Result r = fat::open(path);
        if (r == fat::OK) {
            uint8_t header[16];
            uint16_t br;
            r = fat::read(header, 16, &br);
            fat::close();

            if (r == fat::OK && br > 0) {
                gfx::setCursor(2, LIST_Y + 10);
                gfx::print("Header:");
                for (uint8_t i = 0; i < 8 && i < br; i++) {
                    gfx::setCursor(2 + i * 15, LIST_Y + 20);
                    char hex[3];
                    hex[0] = "0123456789ABCDEF"[header[i] >> 4];
                    hex[1] = "0123456789ABCDEF"[header[i] & 0x0F];
                    hex[2] = '\0';
                    gfx::print(hex);
                }
            }
        }
    }

    drawProgressBar(100);

    gfx::drawFastHLine(0, GFX_HEIGHT - 9, GFX_WIDTH, GFX_WHITE);
    gfx::setCursor(2, GFX_HEIGHT - 7);
    gfx::print("B:Return to menu");

    gfx::display();
}
#endif

// --- Setup & Loop ---

void setup()
{
    os_libs_init();  // ensure all OS libraries are linked (even if unused elsewhere)

    // Blink test: 3 quick flashes to confirm the MCU is alive
    led::init();
    ostime::init();
    for (uint8_t i = 0; i < 3; i++) {
        led::on(PIN_LED_0);
        ostime::delay_ms(150);
        led::off(PIN_LED_0);
        ostime::delay_ms(150);
    }

    beep::init();
    beep::beep_startup();

    gfx::init();
    beep::beep_startup();  // confirm display init completed
    gfx::setTextSize(1);
    gfx::setTextColor(GFX_WHITE);
    input::init();

    // Wait for user to release all buttons (debounce from boot)
    // before attempting SD mount (touch ADC may interfere with SPI)
    ostime::delay_ms(100);
}

void loop()
{
    input::update();

    switch (state) {

    case STATE_LOADING:
        drawLoading();
        ostime::delay_ms(50);

        if (loadingPct < 30) {
            loadingPct += 2;
        } else if (loadingPct < 60) {
            if (storage::sdAvailable())
                loadingPct += 3;
            else
                loadingPct = 100;  // skip to end — no SD card
        } else if (loadingPct < 90) {
            loadingPct += 2;
        } else if (loadingPct < 100) {
            loadingPct += 1;
        } else {
            if (storage::sdAvailable()) {
#if HW_VERSION == 2
                collectFiles();
                scrollOff = 0;
#endif
                cursor    = 0;
                state     = STATE_BROWSE;
            } else {
                state = STATE_V1_INFO;
            }
        }
        break;

#if HW_VERSION == 2
    case STATE_BROWSE:
        drawBrowse();

        if (input::justPressed(PIN_BTN_UP)) {
            if (cursor > 0) {
                cursor--;
                if (cursor < scrollOff)
                    scrollOff = cursor;
            }
        }
        if (input::justPressed(PIN_BTN_DOWN)) {
            if (cursor + 1 < fileCount) {
                cursor++;
                if (cursor >= scrollOff + MAX_VISIBLE)
                    scrollOff = cursor - MAX_VISIBLE + 1;
            }
        }
        if (input::justPressed(PIN_BTN_A)) {
            if (fileCount > 0) {
                state = STATE_FILE_INFO;
            }
        }
        if (input::justPressed(PIN_BTN_B)) {
            cursor = 0;
            state = STATE_MENU;
        }

        ostime::delay_ms(80);
        break;

    case STATE_FILE_INFO:
        drawFileInfo();

        if (input::justPressed(PIN_BTN_A)) {
            if (!files[cursor].isDir) {
                state = STATE_EXECUTING;
            }
        }
        if (input::justPressed(PIN_BTN_B)) {
            state = STATE_BROWSE;
        }

        ostime::delay_ms(80);
        break;

    case STATE_EXECUTING:
        drawExecuting();

        if (input::justPressed(PIN_BTN_B)) {
            state = STATE_BROWSE;
        }

        ostime::delay_ms(80);
        break;
#endif

    case STATE_ERROR:
        beep::beep_error();
        gfx::clear();
        drawHeader("No SD Card");

        gfx::setCursor(4, LIST_Y + 4);
        gfx::print("No SD card detected.");
        gfx::setCursor(4, LIST_Y + 14);
        gfx::print("Insert a FAT-formatted");
        gfx::setCursor(4, LIST_Y + 22);
        gfx::print("SD card for storage.");
        gfx::setCursor(4, LIST_Y + 30);
        gfx::print("Press B for menu.");

        gfx::drawFastHLine(0, GFX_HEIGHT - 9, GFX_WIDTH, GFX_WHITE);
        gfx::setCursor(2, GFX_HEIGHT - 7);
        gfx::print("B:Menu");

        gfx::display();

        if (input::justPressed(PIN_BTN_B)) {
            cursor = 0;
            state = STATE_MENU;
        }
        ostime::delay_ms(80);
        break;

    case STATE_V1_INFO:
        gfx::clear();
        drawHeader("PHSI245 OS");

        gfx::setCursor(4, LIST_Y + 4);
        gfx::print("No SD card detected.");
        gfx::setCursor(4, LIST_Y + 14);
        gfx::print("Insert a FAT-formatted");
        gfx::setCursor(4, LIST_Y + 22);
        gfx::print("SD card for storage.");
        gfx::setCursor(4, LIST_Y + 30);
        gfx::print("Press A for menu.");

        gfx::drawFastHLine(0, GFX_HEIGHT - 9, GFX_WIDTH, GFX_WHITE);
        gfx::setCursor(2, GFX_HEIGHT - 7);
        gfx::print("A:Menu");

        gfx::display();

        if (input::justPressed(PIN_BTN_A)) {
            cursor = 0;
            state = STATE_MENU;
        }
        ostime::delay_ms(80);
        break;

    case STATE_MENU: {
        const char *items[5];
        uint8_t itemCount = 0;
        items[itemCount++] = "Play Pong";
        items[itemCount++] = "Calibrate Touch";
        if (storage::sdAvailable()) {
            items[itemCount++] = "Format SD Card";
        }
        items[itemCount++] = "Restart";

        gfx::clear();
        drawHeader("Main Menu");

        for (uint8_t i = 0; i < itemCount; i++) {
            int16_t y = LIST_Y + i * 14;
            if (i == cursor) {
                gfx::setCursor(0, y);
                gfx::print(">");
            }
            gfx::setCursor(8, y);
            gfx::print(items[i]);
        }

        gfx::drawFastHLine(0, GFX_HEIGHT - 9, GFX_WIDTH, GFX_WHITE);
        gfx::setCursor(2, GFX_HEIGHT - 7);
        gfx::print("A:Select  B:Back");

        gfx::display();

        if (input::justPressed(PIN_BTN_UP) && cursor > 0) {
            cursor--;
        }
        if (input::justPressed(PIN_BTN_DOWN) && cursor + 1 < itemCount) {
            cursor++;
        }
        if (input::justPressed(PIN_BTN_A)) {
            if (cursor == 0) {
                state = STATE_PONG;
            } else if (cursor == 1) {
                state = STATE_CALIBRATE;
            } else if (cursor == 2 && storage::sdAvailable()) {
                state = STATE_FORMAT;
            } else {
                NVIC_SystemReset();
            }
        }
        if (input::justPressed(PIN_BTN_B)) {
            if (storage::sdAvailable())
                state = STATE_BROWSE;
            else
                state = STATE_V1_INFO;
        }
        ostime::delay_ms(80);
        break;
    }

    case STATE_PONG: {
        pong::update();
        pong::draw();
        gfx::display();

        if (!pong::isPlaying() && input::justPressed(PIN_BTN_B)) {
            state = STATE_MENU;
            cursor = 0;
        }
        ostime::delay_ms(25);
        break;
    }

#if HW_VERSION == 2
    case STATE_FORMAT: {
        static bool confirmed = false;

        if (!confirmed) {
            gfx::clear();
            drawHeader("Format SD Card");
            gfx::setCursor(4, LIST_Y + 2);
            gfx::print("This will erase ALL");
            gfx::setCursor(4, LIST_Y + 12);
            gfx::print("data on the card!");
            gfx::setCursor(4, LIST_Y + 28);
            gfx::print("A: Confirm format");
            gfx::setCursor(4, LIST_Y + 38);
            gfx::print("B: Cancel");
            gfx::display();

            if (input::justPressed(PIN_BTN_A)) {
                confirmed = true;
            }
            if (input::justPressed(PIN_BTN_B)) {
                state = STATE_MENU;
                cursor = 2;
            }
            ostime::delay_ms(80);
            break;
        }

        // Do the format
        gfx::clear();
        drawHeader("Formatting...");
        gfx::setCursor(4, LIST_Y + 8);
        gfx::print("Please wait...");
        gfx::display();

        fat::Result r = fat::format();

        gfx::clear();
        drawHeader("Format");
        gfx::setCursor(4, LIST_Y + 8);
        if (r == fat::OK) {
            gfx::print("Format successful!");
            gfx::setCursor(4, LIST_Y + 18);
            gfx::print("Card is ready to use.");
        } else {
            gfx::print("Format failed!");
            gfx::setCursor(4, LIST_Y + 18);
            gfx::print("Error: ");
            gfx::print((int16_t)r);
        }
        gfx::setCursor(4, LIST_Y + 34);
        gfx::print("Press A to continue");
        gfx::display();

        if (input::justPressed(PIN_BTN_A)) {
            confirmed = false;
            state = STATE_MENU;
            cursor = 2;
        }
        ostime::delay_ms(80);
        break;
    }
#endif

#if HW_VERSION == 1
    case STATE_BROWSE:
    case STATE_FILE_INFO:
    case STATE_EXECUTING:
        break;
#endif
    }
}
