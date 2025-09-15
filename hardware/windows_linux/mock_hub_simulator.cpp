#include "mock_hub_simulator.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

static EspNowMessageCallback messageCallback = nullptr;
static bool simulatorRunning = false;
static std::thread simulatorThread;

// Mock hub responses for different commands
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
        } else if (command == "GET_METADATA") {
            response = {
                {"kind", "METADATA"},
                {"data", {
                    {"title", "S4 · E1: Cakey's Cupcake Cousins"},
                    {"artist", "Cakey's Cupcake Cousins"},
                    {"album", "Album 1 • Collection"},
                    {"duration", 200},
                    {"position", 75},
                    {"state", "playing"}
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
            
            response = {
                {"kind", "METADATA"},
                {"data", {
                    {"title", "Song 2 • Test"},
                    {"artist", "Artist's Music"},
                    {"album", "Album 2 · Collection"},
                    {"duration", 240},
                    {"position", position},
                    {"state", playing ? "playing" : "paused"}
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
            
            json metadata = {
                {"kind", "METADATA"},
                {"data", {
                    {"title", title},
                    {"artist", artist},
                    {"album", album},
                    {"duration", song_duration},
                    {"position", song_position},
                    {"state", (counter % 2) ? "playing" : "paused"}
                }},
                {"supports_response", true}
            };
            
            std::cout << "Mock Hub: Sending periodic metadata update..." << std::endl;
            messageCallback(metadata);
            counter++;
        }
    }
}

void startMockHubSimulator(EspNowMessageCallback callback) {
    messageCallback = callback;
    simulatorRunning = true;
    
    simulatorThread = std::thread(simulatePeriodicUpdates);
    
    std::cout << "Mock Hub Simulator started!" << std::endl;
    std::cout << "   - Volume commands will show notifications" << std::endl;
    std::cout << "   - Power commands will show status" << std::endl;
    std::cout << "   - Apple TV commands will update metadata" << std::endl;
    std::cout << "   - Periodic metadata updates every 5-8 seconds" << std::endl;
}

void stopMockHubSimulator() {
    simulatorRunning = false;
    if (simulatorThread.joinable()) {
        simulatorThread.join();
    }
    std::cout << "Mock Hub Simulator stopped!" << std::endl;
}

void handleMockHubCommand(const json& command) {
    if (!messageCallback) return;
    
    std::string device = command.value("device", "");
    std::string cmd = command.value("command", "");
    
    std::cout << "Mock Hub: Receive... " << device << " -> " << cmd << std::endl;
    
    // Simulate processing delay
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    json response = createMockResponse(device, cmd);
    
    if (!response.empty()) {
        std::cout << "Mock Hub: Sending response..." << std::endl;
        messageCallback(response);
    }
}
