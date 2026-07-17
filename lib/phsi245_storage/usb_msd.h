#pragma once

namespace usb_msd {

// Initialize USB and start MSD class. Returns immediately; USB events
// are handled in the USB interrupt. Call update() in loop().
void init();

// Must be called regularly from loop() to process SCSI commands.
// Returns true if a read or write was processed this call.
bool update();

// Returns true if USB is connected to a host (VBUS detected).
bool isConnected();

} // namespace usb_msd
