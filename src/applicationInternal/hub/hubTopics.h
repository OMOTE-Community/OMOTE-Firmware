#pragma once
#include <string>

namespace Hub {
  // C++11: native_unit builds with -std=c++11, so no inline variables (C++17).
  static constexpr const char* RESPONSE_TOPIC = "remote_responses";

  inline bool isHubResponseTopic(const std::string& topic) {
    return topic == RESPONSE_TOPIC;
  }
}
