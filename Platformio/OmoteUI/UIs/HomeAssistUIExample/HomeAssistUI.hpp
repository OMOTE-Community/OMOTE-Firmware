#pragma once
#include "BasicUI.hpp"
#include "HomeAssist/WebSocket/WebSocketApi.hpp"

namespace UI {

class HomeAssistUI : public BasicUI {
 public:
  HomeAssistUI();

 private:
  std::unique_ptr<HomeAssist::WebSocket::WebSocketApi> mHomeAssistSock =
      nullptr;
};

}  // namespace UI
