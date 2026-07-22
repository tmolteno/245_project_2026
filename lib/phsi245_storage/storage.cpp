#include "sd_available.h"

#if HW_VERSION == 2
#include "storage.h"   // for fat::mount, sd::DEVICE
#endif

namespace storage {

static bool sdReady = false;
static bool sdChecked = false;  // don't retry after first attempt

bool sdAvailable()
{
#if HW_VERSION == 1
    return false;
#else
    if (sdReady) return true;
    if (sdChecked) return false;  // already tried — don't retry

    sdChecked = true;
    fat::Result r = fat::mount(&sd::DEVICE);
    if (r == fat::OK) {
        sdReady = true;
        return true;
    }
    return false;
#endif
}

} // namespace storage
