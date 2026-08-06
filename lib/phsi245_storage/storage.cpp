#include "sd_available.h"
#include "storage.h"   // for fat::mount, sd::DEVICE

namespace storage {

static bool sdReady = false;
static bool sdChecked = false;  // don't retry after first attempt
static int  lastErr = -1;       // last fat::Result from mount (diagnostics)

bool sdAvailable()
{
    if (sdReady) return true;
    if (sdChecked) return false;  // already tried — don't retry

    sdChecked = true;
    fat::Result r = fat::mount(&sd::DEVICE);
    lastErr = (int)r;
    if (r == fat::OK) {
        sdReady = true;
        return true;
    }
    return false;
}

// Returns the last FAT mount result code (for diagnostics).
// -1 = never attempted, otherwise a value from fat::Result.
int lastMountError()
{
    return lastErr;
}

} // namespace storage
