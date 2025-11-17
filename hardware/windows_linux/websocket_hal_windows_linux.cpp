#include "websocket_hal_windows_linux.h"
#include "secrets.h"
#include "lib/easywsclient/easywsclient.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <cstring>

using easywsclient::WebSocket;

tAnnounceWebSocketMessage_cb thisAnnounceWebSocketMessage_cb = nullptr;
WebSocket::pointer ws = nullptr;
std::string hubUrl;
std::mutex wsMutex;
std::thread reconnectThread;
bool shouldReconnect = true;
const int RECONNECT_DELAY_MS = 5000;

void set_announceWebSocketMessage_cb_HAL(tAnnounceWebSocketMessage_cb pAnnounceWebSocketMessage_cb) {
    thisAnnounceWebSocketMessage_cb = pAnnounceWebSocketMessage_cb;
}

void attempt_connection() {
    std::lock_guard<std::mutex> lock(wsMutex);
    
    if (ws && ws->getReadyState() != WebSocket::CLOSED) {
        return;
    }
    
    std::cout << "[WebSocket HAL] Attempting to connect to " << hubUrl << std::endl;
    
    if (ws) {
        delete ws;
        ws = nullptr;
    }
    
    ws = WebSocket::from_url(hubUrl);
    
    if (!ws) {
        std::cout << "[WebSocket HAL] Failed to create WebSocket connection" << std::endl;
        return;
    }
    
    if (ws->getReadyState() == WebSocket::OPEN) {
        std::cout << "[WebSocket HAL] Connected successfully" << std::endl;
    }
}

void reconnect_loop() {
    while (shouldReconnect) {
        {
            std::lock_guard<std::mutex> lock(wsMutex);
            if (!ws || ws->getReadyState() == WebSocket::CLOSED) {
                std::cout << "[WebSocket HAL] Connection lost, reconnecting..." << std::endl;
            }
        }
        
        attempt_connection();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY_MS));
    }
}

void init_websocket_HAL(const char* hub_url) {
    hubUrl = hub_url;
    std::cout << "[WebSocket HAL] Initializing WebSocket connection to " << hub_url << std::endl;
    
    shouldReconnect = true;
    attempt_connection();
    
    reconnectThread = std::thread(reconnect_loop);
}

void websocket_loop_HAL() {
    std::lock_guard<std::mutex> lock(wsMutex);
    
    if (!ws) {
        return;
    }
    
    if (ws->getReadyState() == WebSocket::CLOSED) {
        return;
    }
    
    ws->poll(0);
    
    if (!thisAnnounceWebSocketMessage_cb) {
        return;
    }
    
    auto callback = thisAnnounceWebSocketMessage_cb;
    ws->dispatchBinary([callback](const std::vector<uint8_t>& message) {
        try {
            auto unpacked_json = json::from_msgpack(message);
            callback(unpacked_json);
        } catch (const std::exception& e) {
            std::cout << "[WebSocket HAL] Error parsing message: " << e.what() << std::endl;
        }
    });
}

bool publishWebSocketMessage_HAL(json payload) {
    std::lock_guard<std::mutex> lock(wsMutex);
    
    if (!ws) {
        std::cout << "[WebSocket HAL] WebSocket not initialized" << std::endl;
        return false;
    }
    
    if (ws->getReadyState() != WebSocket::OPEN) {
        std::cout << "[WebSocket HAL] WebSocket not connected" << std::endl;
        return false;
    }
    
    try {
        std::vector<uint8_t> packed_json = json::to_msgpack(payload);
        ws->sendBinary(packed_json);
        std::cout << "[WebSocket HAL] Sent message: " << payload.dump() << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cout << "[WebSocket HAL] Error sending message: " << e.what() << std::endl;
        return false;
    }
}

void websocket_shutdown_HAL() {
    std::cout << "[WebSocket HAL] Shutting down WebSocket connection" << std::endl;
    
    shouldReconnect = false;
    
    if (reconnectThread.joinable()) {
        reconnectThread.join();
    }
    
    std::lock_guard<std::mutex> lock(wsMutex);
    
    if (ws) {
        if (ws->getReadyState() != WebSocket::CLOSED) {
            ws->close();
            ws->poll(100);
        }
        delete ws;
        ws = nullptr;
    }
}

bool websocket_is_connected_HAL() {
    std::lock_guard<std::mutex> lock(wsMutex);
    return ws && ws->getReadyState() == WebSocket::OPEN;
}

const char* get_websocket_hub_url_HAL() {
    return hubUrl.c_str();
}
