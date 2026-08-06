// Force linker retention of all OS libraries.
// Even if main.cpp stops using a library directly, this file ensures
// the linker does not strip the library code. Add each new OS library here.
#include "os_libs.h"

void os_libs_init()
{
    // CRITICAL ORDER FIX: the SD mount (SPI) must run BEFORE the display
    // (I2C) init. Initializing SPI after the I2C display is up kills the
    // display and hangs the MCU (observed empirically on this board).
    volatile bool sdCheck = storage::sdAvailable();
    (void)sdCheck;

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
    storage::initSave();
#endif

    // Touch calibration — runs on first boot or after firmware reflash
    {
        const char *s = __DATE__ " " __TIME__;
        uint8_t hash = 0;
        while (*s) hash ^= (uint8_t)*s++;
        touchCalibrate(hash);
    }
}
