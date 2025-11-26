#include "pairingManager.h"
#include "hubManager.h"
#include "protoCodec.h"
#include "applicationInternal/omote_log.h"
#include "applicationInternal/gui/guiNotification.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "applicationInternal/gui/guiMemoryOptimizer.h"
#include "guis/gui_pairing.h"
#include <cstring>

namespace Hub {

PairingManager& PairingManager::getInstance() {
    static PairingManager instance;
    return instance;
}

void PairingManager::handlePairingStatus(const omote_PairingStatus& status) {
    deviceId = std::string(status.device_id);
    step = std::string(status.step);
    message = std::string(status.message);
    requiresPin_ = status.requires_pin;
    expectedPinLength = status.expected_pin_length;
    contextToken = std::string(status.context_token);
    
    omote_log_i("Pairing status: device=%s, step=%s, message=%s, requires_pin=%d\r\n",
               deviceId.c_str(), step.c_str(), message.c_str(), requiresPin_);
    
    if (step == "awaiting_pin") {
        active = true;
        // Show notification and switch to pairing GUI
        GuiNotification::showMessageNotification(message.c_str());
        
        // Switch to the pairing GUI tab
        gui_memoryOptimizer_setActiveGUIname(std::string(tabName_pairing));
        gui_pairing_update_status(message.c_str());
        gui_pairing_clear_pin();
        
    } else if (step == "success") {
        active = false;
        GuiNotification::showMessageNotification("Pairing successful!");
        gui_pairing_clear_pin();
        
    } else if (step == "failed" || step == "cancelled") {
        active = false;
        if (step == "failed") {
            GuiNotification::showErrorNotification(message.c_str());
        } else {
            GuiNotification::showMessageNotification("Pairing cancelled");
        }
        gui_pairing_clear_pin();
    }
}

void PairingManager::reset() {
    active = false;
    deviceId.clear();
    step.clear();
    message.clear();
    requiresPin_ = false;
    expectedPinLength = 0;
    contextToken.clear();
}

void PairingManager::submitPin(const char* pin) {
    if (!active) {
        omote_log_w("Cannot submit PIN: no active pairing session\r\n");
        return;
    }
    
    // Create protobuf message with PIN and context token in data field
    // Format: PIN first, then context token (both null-terminated strings)
    // Buffer needs to hold: PIN (up to 16 hex chars) + null + UUID (36 chars) + null = 54 bytes
    uint8_t data_buffer[64];
    size_t pin_len = strlen(pin);
    size_t token_len = contextToken.length();
    size_t data_len = pin_len + 1 + token_len + 1;
    
    if (data_len > sizeof(data_buffer)) {
        omote_log_e("PIN and context token too large: pin_len=%zu, token_len=%zu, total=%zu\r\n", 
                   pin_len, token_len, data_len);
        return;
    }
    
    // Pack PIN and token into data buffer
    memcpy(data_buffer, pin, pin_len + 1);
    memcpy(data_buffer + pin_len + 1, contextToken.c_str(), token_len + 1);
    
    omote_RemoteEvent event = ProtoCodec::createRemoteEvent(
        deviceId,
        omote_OmoteCommand_PAIRING_SUBMIT_PIN,
        omote_OmoteCommandType_SHORT,
        "",
        data_buffer,
        data_len
    );
    
    HubManager::getInstance().sendRemoteEvent(event);
    omote_log_i("Submitted PIN for pairing\r\n");
}

void PairingManager::cancel() {
    if (!active) {
        omote_log_w("Cannot cancel: no active pairing session\r\n");
        return;
    }
    
    omote_RemoteEvent event = ProtoCodec::createRemoteEvent(
        deviceId,
        omote_OmoteCommand_PAIRING_CANCEL,
        omote_OmoteCommandType_SHORT
    );
    
    HubManager::getInstance().sendRemoteEvent(event);
    omote_log_i("Cancelled pairing\r\n");
    
    reset();
}

} // namespace Hub
