#include "protoCodec.h"
#include <cstring>

namespace Hub {

omote_OmoteCommand ProtoCodec::stringToCommand(const std::string& cmd) {
    if (cmd == "POWER_ON") return omote_OmoteCommand_POWER_ON;
    if (cmd == "POWER_OFF") return omote_OmoteCommand_POWER_OFF;
    if (cmd == "DOWN") return omote_OmoteCommand_DOWN;
    if (cmd == "UP") return omote_OmoteCommand_UP;
    if (cmd == "RIGHT") return omote_OmoteCommand_RIGHT;
    if (cmd == "LEFT") return omote_OmoteCommand_LEFT;
    if (cmd == "SELECT") return omote_OmoteCommand_SELECT;
    if (cmd == "HOME") return omote_OmoteCommand_HOME;
    if (cmd == "MENU") return omote_OmoteCommand_MENU;
    if (cmd == "PLAY_PAUSE") return omote_OmoteCommand_PLAY_PAUSE;
    if (cmd == "VOL_PLUS") return omote_OmoteCommand_VOL_PLUS;
    if (cmd == "VOL_MINUS") return omote_OmoteCommand_VOL_MINUS;
    if (cmd == "VOL_MUTE") return omote_OmoteCommand_VOL_MUTE;
    if (cmd == "SKIP_BACKWARD") return omote_OmoteCommand_SKIP_BACKWARD;
    if (cmd == "SKIP_FORWARD") return omote_OmoteCommand_SKIP_FORWARD;
    if (cmd == "PAIRING_START") return omote_OmoteCommand_PAIRING_START;
    if (cmd == "PAIRING_SUBMIT_PIN") return omote_OmoteCommand_PAIRING_SUBMIT_PIN;
    if (cmd == "PAIRING_CANCEL") return omote_OmoteCommand_PAIRING_CANCEL;
    if (cmd == "SYNC_STATE") return omote_OmoteCommand_SYNC_STATE;
    if (cmd == "STOP") return omote_OmoteCommand_STOP;
    if (cmd == "REWIND") return omote_OmoteCommand_REWIND;
    if (cmd == "FORWARD") return omote_OmoteCommand_FORWARD;
    if (cmd == "CONF") return omote_OmoteCommand_CONF;
    if (cmd == "INFO") return omote_OmoteCommand_INFO;
    if (cmd == "OK") return omote_OmoteCommand_OK;
    if (cmd == "BACK") return omote_OmoteCommand_BACK;
    if (cmd == "SRC") return omote_OmoteCommand_SRC;
    if (cmd == "CHANNEL_UP") return omote_OmoteCommand_CHANNEL_UP;
    if (cmd == "REC") return omote_OmoteCommand_REC;
    if (cmd == "CHANNEL_DOWN") return omote_OmoteCommand_CHANNEL_DOWN;
    if (cmd == "RED") return omote_OmoteCommand_RED;
    if (cmd == "GREEN") return omote_OmoteCommand_GREEN;
    if (cmd == "YELLOW") return omote_OmoteCommand_YELLOW;
    if (cmd == "BLUE") return omote_OmoteCommand_BLUE;
    if (cmd == "GUI_EVENT") return omote_OmoteCommand_GUI_EVENT;
    
    return omote_OmoteCommand_OMOTE_COMMAND_UNSPECIFIED;
}

omote_OmoteCommandType ProtoCodec::stringToCommandType(const std::string& type) {
    if (type == "SHORT") return omote_OmoteCommandType_SHORT;
    if (type == "LONG") return omote_OmoteCommandType_LONG;
    return omote_OmoteCommandType_SHORT;
}

omote_RemoteEvent ProtoCodec::createRemoteEvent(
    const std::string& device,
    omote_OmoteCommand command,
    omote_OmoteCommandType type,
    const std::string& remote_id,
    const uint8_t* data,
    size_t data_len
) {
    omote_RemoteEvent event = omote_RemoteEvent_init_zero;
    
    // Set device
    strncpy(event.device, device.c_str(), sizeof(event.device) - 1);
    event.device[sizeof(event.device) - 1] = '\0';
    
    // Set command and type
    event.command = command;
    event.type = type;
    
    // Set remote_id if provided
    if (!remote_id.empty()) {
        strncpy(event.remote_id, remote_id.c_str(), sizeof(event.remote_id) - 1);
        event.remote_id[sizeof(event.remote_id) - 1] = '\0';
    }
    
    // Set data if provided
    if (data && data_len > 0) {
        size_t copy_len = (data_len < sizeof(event.data.bytes)) ? data_len : sizeof(event.data.bytes);
        memcpy(event.data.bytes, data, copy_len);
        event.data.size = copy_len;
    }
    
    return event;
}

size_t ProtoCodec::encodeRemoteEvent(const omote_RemoteEvent& event, uint8_t* buffer, size_t buffer_size) {
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, buffer_size);
    bool status = pb_encode(&stream, omote_RemoteEvent_fields, &event);
    
    if (!status) {
        return 0;
    }
    
    return stream.bytes_written;
}

omote_CommandResult ProtoCodec::decodeCommandResult(const uint8_t* buffer, size_t buffer_size) {
    static omote_CommandResult proto_result;
    memset(&proto_result, 0, sizeof(proto_result));
    
    // Decode from protobuf
    pb_istream_t stream = pb_istream_from_buffer(buffer, buffer_size);
    bool status = pb_decode(&stream, omote_CommandResult_fields, &proto_result);
    
    if (!status) {
        // Return error result on decode failure
        omote_CommandResult error = omote_CommandResult_init_zero;
        error.kind = omote_ResponseKind_ERROR;
        error.which_data = omote_CommandResult_error_tag;
        strncpy(error.data.error.message, "Failed to decode protobuf message", sizeof(error.data.error.message) - 1);
        return error;
    }
    
    return proto_result;
}

} // namespace Hub
