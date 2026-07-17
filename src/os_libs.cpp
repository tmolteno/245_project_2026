// Force linker retention of all OS libraries.
// Even if main.cpp stops using a library directly, this file ensures
// the linker does not strip the library code. Add each new OS library here.
#include "os_libs.h"

void os_libs_init()
{
    // Init calls ensure each library's object files are pulled into the link.
    // These are normal init calls — they run once at boot and have no
    // side effects beyond what setup() already does elsewhere.
    gfx::init();
    input::init();
    led::init();
    ostime::init();
    beep::init();
    rng::init();
    pong::init();
#if HW_VERSION == 2
    // Pull in storage library (SD + FAT + EEPROM save) on v2 hardware
    storage::initSave();
    volatile bool storage_retain = false;
    if (storage_retain)
        fat::mount(&sd::DEVICE);
#endif
    // Touch calibration — runs on first boot or after firmware reflash
    {
        const char *s = __DATE__ " " __TIME__;
        uint8_t hash = 0;
        while (*s) hash ^= (uint8_t)*s++;
        touchCalibrate(hash);
    }
}
