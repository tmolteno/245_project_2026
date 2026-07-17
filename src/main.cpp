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
static bool     sdReady = false;
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

    if (!e->isDir && sdReady) {
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

#if HW_VERSION == 1
    state = STATE_V1_INFO;
#endif
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
#if HW_VERSION == 2
            fat::Result r = fat::mount(&sd::DEVICE);
            if (r == fat::OK) {
                sdReady = true;
                loadingPct += 3;
            } else {
                // Card missing or unreadable — skip to error immediately
                loadingPct = 100;
                sdReady = false;
            }
#else
            loadingPct += 3;
#endif
        } else if (loadingPct < 90) {
            loadingPct += 2;
        } else if (loadingPct < 100) {
            loadingPct += 1;
        } else {
#if HW_VERSION == 2
            if (sdReady) {
                collectFiles();
            }
            cursor    = 0;
            scrollOff = 0;
            state     = sdReady ? STATE_BROWSE : STATE_ERROR;
#endif
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

        gfx::setCursor(14, LIST_Y + 4);
        gfx::print("Hardware v1");

        gfx::setCursor(4, LIST_Y + 16);
        gfx::print("This revision has");
        gfx::setCursor(4, LIST_Y + 26);
        gfx::print("no SD card slot.");
        gfx::setCursor(4, LIST_Y + 36);
        gfx::print("Storage unavailable.");

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
        static const char *items[] = {
            "Play Pong",
            "Calibrate Touch",
            "Restart",
        };
        const uint8_t itemCount = 3;

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
            } else {
                NVIC_SystemReset();
            }
        }
        if (input::justPressed(PIN_BTN_B)) {
#if HW_VERSION == 1
            state = STATE_V1_INFO;
#else
            state = STATE_BROWSE;
#endif
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

    case STATE_CALIBRATE: {
        // Run interactive calibration and save to flash
        const char *s = __DATE__ " " __TIME__;
        uint8_t hash = 0;
        while (*s) hash ^= (uint8_t)*s++;
        touchCalibrate(hash);
        state = STATE_MENU;
        cursor = 1;
        break;
    }

#if HW_VERSION == 1
    case STATE_BROWSE:
    case STATE_FILE_INFO:
    case STATE_EXECUTING:
        break;
#endif
    }
}
