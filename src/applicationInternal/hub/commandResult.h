#pragma once

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Hub {

enum class ResponseKind {
    ACK,
    VOLUME,
    POWER,
    RAW_COMMAND,
    ERROR,
    STATE_SYNC,
    NONE
};

// Response data structures
struct Ack {
    // Empty struct for acknowledgment
};

struct Volume {
    double level;
    bool is_muted;
    
    Volume(double lvl = 0.0, bool muted = false) : level(lvl), is_muted(muted) {}
};

struct Power {
    bool is_on;
    
    Power(bool on = false) : is_on(on) {}
};

struct RawCommand {
    std::string raw_response;
    bool success;
    
    RawCommand(const std::string& response = "", bool succeeded = false) 
        : raw_response(response), success(succeeded) {}
};

struct Error {
    std::string message;
    
    Error(const std::string& msg = "") : message(msg) {}
};

struct Metadata {
    std::string title;
    std::string artist;
    std::string album;
    int duration;  // Duration in seconds
    int position;  // Current position in seconds
    std::string state;  // "playing", "paused", "stopped", etc.
    
    Metadata(const std::string& t = "", const std::string& a = "", const std::string& alb = "",
             int dur = 0, int pos = 0, const std::string& st = "")
        : title(t), artist(a), album(alb), duration(dur), position(pos), state(st) {}
};

struct Time {
    unsigned long timestamp;  // Unix epoch timestamp
    int timezone_offset;      // Timezone offset in seconds
    
    Time(unsigned long ts = 0, int tz_offset = 0) : timestamp(ts), timezone_offset(tz_offset) {}
};

struct StateSync {
    Metadata metadata;
    Time time;
    bool has_metadata;
    bool has_time;
    
    StateSync() : has_metadata(false), has_time(false) {}
};

// Main CommandResult class
class CommandResult {
public:
    ResponseKind kind;
    bool supports_response;
    
    // Union-like storage for different data types
    Ack ack_data;
    Volume volume_data;
    Power power_data;
    RawCommand raw_command_data;
    Error error_data;
    StateSync state_sync_data;
    
    // Constructors for different types
    static CommandResult createAck() {
        CommandResult result;
        result.kind = ResponseKind::ACK;
        result.supports_response = false;
        result.ack_data = Ack();
        return result;
    }
    
    static CommandResult createVolume(double level, bool is_muted = false) {
        CommandResult result;
        result.kind = ResponseKind::VOLUME;
        result.supports_response = true;
        result.volume_data = Volume(level, is_muted);
        return result;
    }
    
    static CommandResult createPower(bool is_on) {
        CommandResult result;
        result.kind = ResponseKind::POWER;
        result.supports_response = true;
        result.power_data = Power(is_on);
        return result;
    }
    
    static CommandResult createRawCommand(const std::string& raw_response, bool success) {
        CommandResult result;
        result.kind = ResponseKind::RAW_COMMAND;
        result.supports_response = true;
        result.raw_command_data = RawCommand(raw_response, success);
        return result;
    }
    
    static CommandResult createError(const std::string& message) {
        CommandResult result;
        result.kind = ResponseKind::ERROR;
        result.supports_response = false;
        result.error_data = Error(message);
        return result;
    }
    
    static CommandResult createStateSync(const StateSync& state_sync) {
        CommandResult result;
        result.kind = ResponseKind::STATE_SYNC;
        result.supports_response = true;
        result.state_sync_data = state_sync;
        return result;
    }
    
    static CommandResult createNone() {
        CommandResult result;
        result.kind = ResponseKind::NONE;
        result.supports_response = false;
        return result;
    }
    
    // Factory method to create from JSON payload
    static CommandResult fromJson(const json& payload) {
        if (!payload.contains("kind")) {
            return createError("Missing 'kind' field in payload");
        }
        
        std::string kind_str = payload["kind"];
        bool supports_response = payload.value("supports_response", false);
        
        CommandResult result;
        result.supports_response = supports_response;
        
        if (kind_str == "ACK") {
            return createAck();
        }
        else if (kind_str == "VOLUME" && payload.contains("data") && payload["data"] != nullptr) {
            auto data = payload["data"];
            if (data.contains("level") && data.contains("is_muted")) {
                double level = data["level"];
                bool is_muted = data["is_muted"];
                return createVolume(level, is_muted);
            }
            return createError("Invalid volume data");
        }
        else if (kind_str == "POWER" && payload.contains("data") && payload["data"] != nullptr) {
            auto data = payload["data"];
            if (data.contains("is_on")) {
                bool is_on = data["is_on"];
                return createPower(is_on);
            }
            return createError("Invalid power data");
        }
        else if (kind_str == "RAW_COMMAND" && payload.contains("data") && payload["data"] != nullptr) {
            auto data = payload["data"];
            if (data.contains("raw_response") && data.contains("success")) {
                std::string raw_response = data["raw_response"];
                bool success = data["success"];
                return createRawCommand(raw_response, success);
            }
            return createError("Invalid raw command data");
        }
        else if (kind_str == "ERROR" && payload.contains("data") && payload["data"] != nullptr) {
            auto data = payload["data"];
            if (data.contains("message")) {
                std::string message = data["message"];
                return createError(message);
            }
            return createError("Invalid error data");
        }
        else if (kind_str == "STATE_SYNC" && payload.contains("data") && payload["data"] != nullptr) {
            auto data = payload["data"];
            StateSync state_sync;
            
            // Parse metadata if present
            if (data.contains("metadata") && data["metadata"] != nullptr) {
                auto metadata = data["metadata"];
                state_sync.metadata = Metadata(
                    metadata.value("title", ""),
                    metadata.value("artist", ""),
                    metadata.value("album", ""),
                    metadata.value("duration", 0),
                    metadata.value("position", 0),
                    metadata.value("state", "")
                );
                state_sync.has_metadata = true;
            }
            
            // Parse time if present
            if (data.contains("time") && data["time"] != nullptr) {
                auto time_data = data["time"];
                if (time_data.contains("timestamp")) {
                    state_sync.time = Time(
                        time_data["timestamp"],
                        time_data.value("timezone_offset", 0)
                    );
                    state_sync.has_time = true;
                }
            }
            
            return createStateSync(state_sync);
        }
        else if (kind_str == "NONE") {
            return createNone();
        }
        
        return createError("Unknown response kind: " + kind_str);
    }
    
    // Getter methods for type-safe access
    const Volume& getVolume() const { return volume_data; }
    const Power& getPower() const { return power_data; }
    const RawCommand& getRawCommand() const { return raw_command_data; }
    const Error& getError() const { return error_data; }
    const StateSync& getStateSync() const { return state_sync_data; }
    
private:
    CommandResult() : supports_response(false) {}
};

} // namespace Hub
