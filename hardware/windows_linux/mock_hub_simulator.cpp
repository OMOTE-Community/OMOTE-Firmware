#include "mock_hub_simulator.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <ctime>

static EspNowMessageProtoCallback messageCallback = nullptr;
static bool simulatorRunning = false;
static std::thread simulatorThread;

// Helper function to get current time and timezone offset
static std::pair<long, int> getCurrentTimeAndOffset() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    
    // Get timezone offset by comparing UTC and local time
    std::time_t time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm* utc_tm = std::gmtime(&time_t_now);
    std::tm* local_tm = std::localtime(&time_t_now);
    
    // Calculate offset in seconds
    int offset_seconds = (local_tm->tm_hour - utc_tm->tm_hour) * 3600 + 
                        (local_tm->tm_min - utc_tm->tm_min) * 60;
    
    return {timestamp, offset_seconds};
}

// Mock hub responses for different commands using new CommandResult format
static json createMockResponse(const std::string& device, const std::string& command) {
    json response;
    
    if (device == "SONY_AVR") {
        if (command == "VOL_PLUS" || command == "VOL_MINUS") {
            // Simulate volume response
            static double volume = -30.0;
            if (command == "VOL_PLUS") volume += 1.0;
            if (command == "VOL_MINUS") volume -= 1.0;
            volume = std::max(-80.0, std::min(23.0, volume)); // Sony range
            
            response = {
                {"kind", "VOLUME"},
                {"data", {
                    {"level", volume},
                    {"is_muted", false}
                }},
                {"supports_response", true}
            };
        } else if (command == "VOL_MUTE") {
            // Simulate mute toggle
            static bool is_muted = false;
            is_muted = !is_muted;
            
            response = {
                {"kind", "VOLUME"},
                {"data", {
                    {"level", -30.0},
                    {"is_muted", is_muted}
                }},
                {"supports_response", true}
            };
        } else if (command == "POWER_ON" || command == "POWER_OFF") {
            response = {
                {"kind", "POWER"},
                {"data", {
                    {"is_on", command == "POWER_ON"}
                }},
                {"supports_response", true}
            };
        }
    } else if (device == "APPLE_TV") {
        if (command == "POWER_ON" || command == "POWER_OFF") {
            response = {
                {"kind", "POWER"},
                {"data", {
                    {"is_on", command == "POWER_ON"}
                }},
                {"supports_response", true}
            };
        } else if (command == "SYNC_STATE") {
            auto time_data = getCurrentTimeAndOffset();
            long timestamp = time_data.first;
            int timezone_offset = time_data.second;
            
            response = {
                {"kind", "STATE_SYNC"},
                {"data", {
                    {"metadata", {
                        {"title", "S4 · E1: Cakey's Cupcake Cousins"},
                        {"artist", "Cakey's Cupcake Cousins"},
                        {"album", "Album 1 • Collection"},
                        {"duration", 200},
                        {"position", 75},
                        {"state", "playing"}
                    }},
                    {"time", {
                        {"timestamp", timestamp},
                        {"timezone_offset", timezone_offset}
                    }},
                    {"has_metadata", true},
                    {"has_time", true}
                }},
                {"supports_response", true}
            };
        } else if (command == "PLAY_PAUSE") {
            static bool playing = true;
            static int position = 45;
            playing = !playing;
            
            if (playing) {
                position += 5;
                if (position > 240) position = 10;
            }
            
            auto time_data = getCurrentTimeAndOffset();
            long timestamp = time_data.first;
            int timezone_offset = time_data.second;
            
            response = {
                {"kind", "STATE_SYNC"},
                {"data", {
                    {"metadata", {
                        {"title", "Song 2 • Test"},
                        {"artist", "Artist's Music"},
                        {"album", "Album 2 · Collection"},
                        {"duration", 240},
                        {"position", position},
                        {"state", playing ? "playing" : "paused"}
                    }},
                    {"time", {
                        {"timestamp", timestamp},
                        {"timezone_offset", timezone_offset}
                    }},
                    {"has_metadata", true},
                    {"has_time", true}
                }},
                {"supports_response", true}
            };
        }
    }
    
    return response;
}

static void simulatePeriodicUpdates() {
    static int counter = 0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 3);
    
    while (simulatorRunning) {
        std::this_thread::sleep_for(std::chrono::seconds(5 + dis(gen)));
        
        if (messageCallback && simulatorRunning) {
            // Simulate Apple TV metadata updates with progressive playback
            static int song_position = 30;
            int song_duration = 180 + (counter % 60);
            
            // Advance position if playing
            if (counter % 2 == 1) {
                song_position += 6; // Advance by ~6 seconds per update
                if (song_position >= song_duration) {
                    song_position = 0;
                }
            }
            
            std::string title, artist, album;
            switch (counter % 4) {
                case 0:
                    title = "S4 · E1: Cakey's Cupcake Cousins";
                    artist = "Cakey's Cupcake Cousins";
                    album = "Album 1 • Collection";
                    break;
                case 1:
                    title = "Song 2 • Test";
                    artist = "Artist's Music";
                    album = "Album 2 · Collection";
                    break;
                case 2:
                    title = "Test Song • Unicode";
                    artist = "Test Artist's Collection";
                    album = "Test Album • Special Edition";
                    break;
                case 3:
                    title = "Current Song • Playing";
                    artist = "Current Artist's Music";
                    album = "Current Album • Now Playing";
                    break;
            }
            
            auto time_data = getCurrentTimeAndOffset();
            long timestamp = time_data.first;
            int timezone_offset = time_data.second;
            
            json state_sync = {
                {"kind", "STATE_SYNC"},
                {"data", {
                    {"metadata", {
                        {"title", title},
                        {"artist", artist},
                        {"album", album},
                        {"duration", song_duration},
                        {"position", song_position},
                        {"state", (counter % 2) ? "playing" : "paused"}
                    }},
                    {"time", {
                        {"timestamp", timestamp},
                        {"timezone_offset", timezone_offset}
                    }},
                    {"has_metadata", true},
                    {"has_time", true}
                }},
                {"supports_response", true}
            };
            
            std::cout << "Mock Hub: Sending periodic state sync update..." << std::endl;
            messageCallback(state_sync);
            counter++;
        }
    }
}

void startMockHubSimulatorProto(EspNowMessageProtoCallback callback) {
    messageCallback = callback;
    simulatorRunning = false;  // Disable for now
    
    std::cout << "Mock Hub Simulator (Protobuf mode):" << std::endl;
    std::cout << "   NOTE: Mock hub simulator not yet implemented for protobuf." << std::endl;
    std::cout << "   The simulator requires NanoPB integration to encode/decode messages." << std::endl;
    std::cout << "   For now, use a real hub instance for testing protobuf communication." << std::endl;
}

void stopMockHubSimulator() {
    simulatorRunning = false;
    if (simulatorThread.joinable()) {
        simulatorThread.join();
    }
    std::cout << "Mock Hub Simulator stopped!" << std::endl;
}

void handleMockHubCommandProto(const uint8_t* data, size_t len) {
    std::cout << "Mock Hub: Received protobuf command (" << len << " bytes)" << std::endl;
    std::cout << "   NOTE: Protobuf decoding not yet implemented in simulator." << std::endl;
    std::cout << "   Use a real hub instance to test protobuf messages." << std::endl;
}
