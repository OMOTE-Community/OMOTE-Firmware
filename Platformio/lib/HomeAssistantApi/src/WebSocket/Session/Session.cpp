#include "WebSocket/Session/Session.hpp"

#include "WebSocket/Message/MessageHandler.hpp"
#include "WebSocket/Request.hpp"

namespace HomeAssist::WebSocket {

Session::Session(
    std::unique_ptr<Request> aRequest,
    std::shared_ptr<MessageHandler> aMessageHandler,
    std::shared_ptr<HAL::WebSocket::Json::IChunkProcessor> aChunkProcessor)
    : mStartRequest(std::move(aRequest)),
      mMessageHandler(aMessageHandler),
      mChunkProcessor(aChunkProcessor) {}

bool Session::ProcessMessage(const Message& aMessage) {
  if (auto handler = mMessageHandler.lock(); handler) {
    return handler->ProcessMessage(aMessage);
  }
  return false;
}
std::shared_ptr<HAL::WebSocket::Json::IChunkProcessor>
Session::GetChunkProcessor() {
  return mChunkProcessor.lock();
}

bool Session::IsComplete() const {
  // Our message handler and chunk processor are no longer active so we are done
  return mMessageHandler.use_count() == 0 && mChunkProcessor.use_count() == 0;
}

Request* Session::BorrowStartRequest() { return mStartRequest.get(); }

Request* Session::BorrowEndRequest() { return mEndRequest.get(); }

bool Session::IsPreferringChunkProcessing() {
  return mMessageHandler.expired() && !mChunkProcessor.expired();
}

}  // namespace HomeAssist::WebSocket