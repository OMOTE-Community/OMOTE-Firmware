#include "infrared_receiver_hal_windows_linux.h"

bool irReceiverEnabled = false;

void start_infraredReceiver_HAL() {
}

void shutdown_infraredReceiver_HAL() {
}

// The repeating section of the code
void infraredReceiver_loop_HAL() {
}

bool get_irReceiverEnabled_HAL() {
  return irReceiverEnabled;
}
void set_irReceiverEnabled_HAL(bool aIrReceiverEnabled) {
  irReceiverEnabled = aIrReceiverEnabled;
}

void set_announceNewIRmessage_cb_HAL(tAnnounceNewIRmessage_cb pAnnounceNewIRmessage_cb) {}
