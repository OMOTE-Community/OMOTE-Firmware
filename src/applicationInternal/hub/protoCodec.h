#pragma once

#include <pb_encode.h>
#include <pb_decode.h>
#include <string>
#include "remote_messages.pb.h"

namespace Hub {

/**
 * Codec for encoding and decoding protobuf messages using NanoPB.
 * 
 * Works directly with protobuf structs - no JSON conversion needed.
 */
class ProtoCodec {
public:
    /**
     * Encode a RemoteEvent protobuf struct to bytes.
     * 
     * @param event RemoteEvent struct to encode
     * @param buffer Output buffer for encoded bytes
     * @param buffer_size Size of the output buffer
     * @return Number of bytes written, or 0 on error
     */
    static size_t encodeRemoteEvent(const omote_RemoteEvent& event, uint8_t* buffer, size_t buffer_size);
    
    /**
     * Decode protobuf bytes to a CommandResult.
     * 
     * @param buffer Input buffer containing protobuf bytes
     * @param buffer_size Size of the input buffer
     * @return omote_CommandResult protobuf struct, or error result on failure
     */
    static omote_CommandResult decodeCommandResult(const uint8_t* buffer, size_t buffer_size);
    
    /**
     * Helper to create a RemoteEvent struct.
     */
    static omote_RemoteEvent createRemoteEvent(
        const std::string& device,
        omote_OmoteCommand command,
        omote_OmoteCommandType type = omote_OmoteCommandType_SHORT,
        const std::string& remote_id = "",
        const uint8_t* data = nullptr,
        size_t data_len = 0
    );
    
    /**
     * Map string command to protobuf enum.
     */
    static omote_OmoteCommand stringToCommand(const std::string& cmd);
    
    /**
     * Map string command type to protobuf enum.
     */
    static omote_OmoteCommandType stringToCommandType(const std::string& type);
};

} // namespace Hub
