#include "espnow_hal_windows_linux.h"
#include "mock_hub_simulator.h"
#include <iostream>

#if !defined(WIN32)
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#endif

// Function to get MAC address for Windows/Linux/macOS
std::string getMACaddress() {
#if defined(__APPLE__)
  // For macOS simulator, return a mock MAC address
  return "AA:BB:CC:DD:EE:FF";
#elif defined(WIN32)
  // For Windows, return a mock MAC address
  return "AA:BB:CC:DD:EE:FF";
#else
  // For Linux, try to get real MAC address
  struct ifreq s;
  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

  strcpy(s.ifr_name, "eth0");
  if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
    char buffer[6*3];
    int i;
    for (i = 0; i < 6; ++i) {
      sprintf(&buffer[i*3], "%02x:", (unsigned char) s.ifr_addr.sa_data[i]);
    }
    std::string MACaddress = std::string(buffer, 17);
    close(fd);
    return MACaddress;
  }
  close(fd);
  return "AA:BB:CC:DD:EE:FF"; // Fallback to mock MAC
#endif
}

// Callback function pointer
static EspNowMessageCallback espNowMessageCallback = nullptr;

void set_announceEspNowMessage_cb_HAL(EspNowMessageCallback callback) {
  espNowMessageCallback = callback;
  std::cout << "ESP-NOW callback registered (simulator with mock hub)" << std::endl;
}

void init_espnow_HAL() {
  std::cout << "ESP-NOW initialized (simulator with mock hub)" << std::endl;
  
  // Start the mock hub simulator
  if (espNowMessageCallback) {
    startMockHubSimulator(espNowMessageCallback);
  }
}

void espnow_loop_HAL() {
  // Nothing to do in the simulator - mock hub runs in background thread
}

bool publishEspNowMessage_HAL(json payload) {
  std::cout << "ESP-NOW message sent to mock hub: " << payload.dump() << std::endl;
  
  // Forward the command to the mock hub for processing
  handleMockHubCommand(payload);
  
  return true; // Always return success in the simulator
}

void espnow_shutdown_HAL() {
  std::cout << "ESP-NOW shutdown (stopping mock hub)" << std::endl;
  stopMockHubSimulator();
}