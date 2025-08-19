#pragma once

#include <string>

void start_infraredReceiver_HAL(void);
void shutdown_infraredReceiver_HAL(void);
void infraredReceiver_loop_HAL(void);

bool get_irReceiverEnabled_HAL();
void set_irReceiverEnabled_HAL(bool aIrReceiverEnabled);

typedef void (*tAnnounceNewIRmessage_cb)(std::string message);
void set_announceNewIRmessage_cb_HAL(tAnnounceNewIRmessage_cb pAnnounceNewIRmessage_cb);
