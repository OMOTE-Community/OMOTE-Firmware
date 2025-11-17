#include "websocket_hal_windows_linux.h"
#include "secrets.h"
#include "lib/easywsclient/easywsclient.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <cstring>
#include <string>

using easywsclient::WebSocket;

tAnnounceWebSocketMessageProto_cb thisAnnounceWebSocketMessageProto_cb = nullptr;
WebSocket::pointer ws = nullptr;
std::string hubUrl;
std::mutex wsMutex;
std::thread reconnectThread;
bool shouldReconnect = true;
const int RECONNECT_DELAY_MS = 5000;

void set_announceWebSocketMessageProto_cb_HAL(tAnnounceWebSocketMessageProto_cb pAnnounceWebSocketMessageProto_cb) {
    thisAnnounceWebSocketMessageProto_cb = pAnnounceWebSocketMessageProto_cb;
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
    
    if (!thisAnnounceWebSocketMessageProto_cb) {
        return;
    }
    
    auto callback = thisAnnounceWebSocketMessageProto_cb;
    ws->dispatchBinary([callback](const std::vector<uint8_t>& message) {
        callback(message.data(), message.size());
    });
}

bool publishWebSocketMessageProto_HAL(const uint8_t* data, size_t len) {
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
        std::vector<uint8_t> message(data, data + len);
        ws->sendBinary(message);
        std::cout << "[WebSocket HAL] Sent protobuf message (" << len << " bytes)" << std::endl;
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
