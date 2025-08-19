#include <string>
#include "applicationInternal/hardware/IRremoteProtocols.h"

std::string concatenateIRsendParams(std::string data, uint16_t nbits, uint16_t repeat) {
  return data + ":" + std::to_string(nbits) + ":" + std::to_string(repeat);
}