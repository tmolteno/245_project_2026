#include "sd_available.h"

#if HW_VERSION == 2
#include "storage.h"   // for fat::mount, sd::DEVICE
#endif

namespace storage {

static bool sdReady = false;

bool sdAvailable()
{
#if HW_VERSION == 1
    return false;
#else
    if (sdReady) return true;

    fat::Result r = fat::mount(&sd::DEVICE);
    if (r == fat::OK) {
        sdReady = true;
        return true;
    }
    return false;
#endif
}

} // namespace storage
