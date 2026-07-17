#pragma once

// Central include for all OS libraries.
// Include this in main.cpp to guarantee dependency tracking by PlatformIO's LDF.
// os_libs.cpp provides the actual symbol references that prevent linker stripping.

#include "gfx.h"
#include "input.h"
#include "led.h"
#include "ostime.h"
#include "beep.h"
#include "random.h"
#include "sd_available.h"
#if HW_VERSION == 2
#include "storage.h"
#endif
#include <HAL.h>
#include "pong.h"
#if HW_VERSION == 2
#include "usb_msd.h"
#endif

void os_libs_init();
