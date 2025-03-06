#include "HomeAssist/WebSocket/ChunkForwarder.hpp"

#include "HomeAssist/WebSocket/Api.hpp"
#include "HomeAssist/WebSocket/Session/ISession.hpp"

using namespace HAL::WebSocket::Json;

namespace HomeAssist::WebSocket {

ChunkForwarder::ChunkForwarder(Api& aApi) : mApi(aApi) {}

void ChunkForwarder::SetProcessor(std::shared_ptr<IChunkProcessor> aProcessor) {
  mProcessor = aProcessor;
}

bool ChunkForwarder::Null() {
  if (auto processor = mProcessor.lock()) {
    return processor->Null();
  }
  return true;
}
bool ChunkForwarder::Bool(bool b) {
  if (auto processor = mProcessor.lock()) {
    return processor->Bool(b);
  }
  return true;
}
bool ChunkForwarder::Int(int i) {
  if (auto processor = mProcessor.lock()) {
    return processor->Int(i);
  }
  return true;
}
bool ChunkForwarder::Uint(unsigned u) {
  if (mProcessingId) {
    mCurrentId = u;
    mFoundId = true;
    mProcessingId = false;

    if (auto& session = mApi.mSessions[mCurrentId]; session) {
      if (auto newProcessor = session->GetChunkProcessor(); newProcessor) {
        mProcessor = newProcessor;
      }
    } else {
      // We did not have a session to handle this so cancel processing
      return false;
    }
  }
  if (auto processor = mProcessor.lock()) {
    return processor->Uint(u);
  }
  return true;
}
bool ChunkForwarder::Int64(int64_t i) {
  if (auto processor = mProcessor.lock()) {
    return processor->Int64(i);
  }
  return true;
}
bool ChunkForwarder::Uint64(uint64_t u) {
  if (auto processor = mProcessor.lock()) {
    return processor->Uint64(u);
  }
  return true;
}
bool ChunkForwarder::Double(double d) {
  if (auto processor = mProcessor.lock()) {
    return processor->Double(d);
  }
  return true;
}
bool ChunkForwarder::RawNumber(const Ch* str, rapidjson::SizeType length,
                               bool copy) {
  if (auto processor = mProcessor.lock()) {
    return processor->RawNumber(str, length, copy);
  }
  return true;
}
bool ChunkForwarder::String(const Ch* str, rapidjson::SizeType length,
                            bool copy) {
  if (auto processor = mProcessor.lock()) {
    return processor->String(str, length, copy);
  }
  return true;
}
bool ChunkForwarder::StartObject() {
  mInObject = true;
  if (auto processor = mProcessor.lock()) {
    return processor->StartObject();
  }
  return true;
}
bool ChunkForwarder::Key(const Ch* str, rapidjson::SizeType length, bool copy) {
  if (strcmp(str, "id") == 0) {
    mProcessingId = true;
  }
  if (auto processor = mProcessor.lock()) {
    return processor->Key(str, length, copy);
  }
  return true;
}
bool ChunkForwarder::EndObject(rapidjson::SizeType memberCount) {
  mInObject = false;
  mFoundId = false;
  mCurrentId = InvalidId;
  if (auto processor = mProcessor.lock()) {
    return processor->EndObject(memberCount);
  }
  return true;
}
bool ChunkForwarder::StartArray() {
  if (auto processor = mProcessor.lock()) {
    return processor->StartArray();
  }
  return true;
}
bool ChunkForwarder::EndArray(rapidjson::SizeType elementCount) {
  if (auto processor = mProcessor.lock()) {
    return processor->EndArray(elementCount);
  }
  return true;
}

}  // namespace HomeAssist::WebSocket
