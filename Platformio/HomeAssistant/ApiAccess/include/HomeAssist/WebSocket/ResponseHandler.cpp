#include "HomeAssist/WebSocket/ResponseHandler.hpp"

#include "HomeAssist/WebSocket/ChunkForwarder.hpp"
#include "HomeAssist/WebSocket/Message/Message.hpp"

using namespace HAL::WebSocket::Json;

namespace HomeAssist::WebSocket {

ResponseHandler::ResponseHandler(Api& aApi)
    : IProcessJsonMessage(
          [this](const auto& aDoc) { return ProcessResponseDoc(aDoc); },
          std::make_unique<ChunkForwarder>(aApi)),
      mApi(aApi) {}

bool ResponseHandler::ProcessResponseDoc(
    const MemConciousDocument& aDocFromSocket) {
  // auto prettyDebugString = ToPrettyString(aDocFromSocket);
  // HardwareFactory::getAbstract().debugPrint("%s", prettyDebugString.c_str());
  auto messageObj = std::make_unique<Message>(aDocFromSocket);
  if (mApi.PreProcessMessage(*messageObj)) {
    return true;
  }
  mApi.mIncomingMessageQueue.push(std::move(messageObj));
  return true;
}

ResponseHandler::~ResponseHandler() {}

}  // namespace HomeAssist::WebSocket
