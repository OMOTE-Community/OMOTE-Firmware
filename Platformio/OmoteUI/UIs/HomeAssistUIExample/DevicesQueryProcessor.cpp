#include "DevicesQueryProcessor.hpp"

namespace UI {

bool DevicesQueryProcessor::Null() { return true; }
bool DevicesQueryProcessor::Bool(bool b) { return true; }
bool DevicesQueryProcessor::Int(int i) { return true; }
bool DevicesQueryProcessor::Uint(unsigned u) { return true; }
bool DevicesQueryProcessor::Int64(int64_t i) { return true; }
bool DevicesQueryProcessor::Uint64(uint64_t u) { return true; }
bool DevicesQueryProcessor::Double(double d) { return true; }
bool DevicesQueryProcessor::RawNumber(const Ch* str, rapidjson::SizeType length,
                                      bool copy) {
  return true;
}
bool DevicesQueryProcessor::String(const Ch* str, rapidjson::SizeType length,
                                   bool copy) {
  return true;
}
bool DevicesQueryProcessor::StartObject() { return true; }
bool DevicesQueryProcessor::Key(const Ch* str, rapidjson::SizeType length,
                                bool copy) {
  return true;
}
bool DevicesQueryProcessor::EndObject(rapidjson::SizeType memberCount) {
  return true;
}
bool DevicesQueryProcessor::StartArray() { return true; }
bool DevicesQueryProcessor::EndArray(rapidjson::SizeType elementCount) {
  return true;
}

}  // namespace UI
