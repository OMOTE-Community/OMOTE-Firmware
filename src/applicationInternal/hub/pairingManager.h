#pragma once

#include <string>
#include "remote_messages.pb.h"

namespace Hub {

class PairingManager {
public:
    static PairingManager& getInstance();
    
    void handlePairingStatus(const omote_PairingStatus& status);
    
    bool isActive() const { return active; }
    const char* getDeviceId() const { return deviceId.c_str(); }
    omote_PairingStep getStep() const { return step; }
    const char* getMessage() const { return message.c_str(); }
    bool requiresPin() const { return requiresPin_; }
    uint32_t getExpectedPinLength() const { return expectedPinLength; }
    const char* getContextToken() const { return contextToken.c_str(); }
    
    void reset();
    
    void submitPin(const char* pin);
    void cancel();
    
private:
    PairingManager() = default;
    PairingManager(const PairingManager&) = delete;
    PairingManager& operator=(const PairingManager&) = delete;
    
    bool active = false;
    std::string deviceId;
    omote_PairingStep step = omote_PairingStep_PAIRING_STEP_UNSPECIFIED;
    std::string message;
    bool requiresPin_ = false;
    uint32_t expectedPinLength = 0;
    std::string contextToken;
};

} // namespace Hub
