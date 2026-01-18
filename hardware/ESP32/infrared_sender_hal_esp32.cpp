#include <Arduino.h>
#include <string>
#include <list>
#include <sstream>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <applicationInternal/omote_log.h>

#if(OMOTE_HARDWARE_REV >= 5)
const uint8_t IR_LED_GPIO = 5;  // IR LED output
#else
const uint8_t IR_LED_GPIO = 33; // IR LED output
#endif

IRsend IrSender(IR_LED_GPIO, true);

void init_infraredSender_HAL(void) {
  // IR Pin Definition
  pinMode(IR_LED_GPIO, OUTPUT);
  digitalWrite(IR_LED_GPIO, HIGH); // HIGH off - LOW on

  IrSender.begin();
}

void getProtocolDefaultBitsAndRepeat(int protocol, uint16_t *defaultParams) {
  // for more defaults, see file IRremoteESP8266/src/IRsend.h
  switch (protocol) {
    case RC5:                  {defaultParams[0] = kRC5XBits;              defaultParams[1] = kNoRepeat;               return;}
    case NEC:                  {defaultParams[0] = kNECBits;               defaultParams[1] = kNoRepeat;               return;}
    case SONY:                 {defaultParams[0] = kSony20Bits;            defaultParams[1] = kSonyMinRepeat;          return;}
    case JVC:                  {defaultParams[0] = kJvcBits;               defaultParams[1] = kNoRepeat;               return;}
    case SAMSUNG:              {defaultParams[0] = kSamsungBits;           defaultParams[1] = kNoRepeat;               return;}
    case LG:                   {defaultParams[0] = kLgBits;                defaultParams[1] = kNoRepeat;               return;}
    case SANYO:                {defaultParams[0] = kSanyoLC7461Bits;       defaultParams[1] = kNoRepeat;               return;}
    case SHARP:                {defaultParams[0] = kSharpBits;             defaultParams[1] = kNoRepeat;               return;}
    case DENON:                {defaultParams[0] = kDenonBits;             defaultParams[1] = kNoRepeat;               return;}
    case SHERWOOD:             {defaultParams[0] = kSherwoodBits;          defaultParams[1] = kSherwoodMinRepeat;      return;}
    case SAMSUNG_AC:           {defaultParams[0] = kSamsungAcStateLength;  defaultParams[1] = kSamsungAcDefaultRepeat; return;}
    case LG2:                  {defaultParams[0] = kLgBits;                defaultParams[1] = kNoRepeat;               return;}
    case SAMSUNG36:            {defaultParams[0] = kSamsung36Bits;         defaultParams[1] = kNoRepeat;               return;}
    case SHARP_AC:             {defaultParams[0] = kSharpAcStateLength;    defaultParams[1] = kSharpAcDefaultRepeat;   return;}
    case SANYO_AC:             {defaultParams[0] = kSanyoAcStateLength;    defaultParams[1] = kNoRepeat;               return;}
    case SANYO_AC88:           {defaultParams[0] = kSanyoAc88StateLength;  defaultParams[1] = kSanyoAc88MinRepeat;     return;}
    case SANYO_AC152:          {defaultParams[0] = kSanyoAc152StateLength; defaultParams[1] = kSanyoAc152MinRepeat;    return;}
    case WOWWEE:               {defaultParams[0] = kWowweeBits;            defaultParams[1] = kWowweeDefaultRepeat;    return;}
    case YORK:                 {defaultParams[0] = kYorkStateLength;       defaultParams[1] = kNoRepeat;               return;}

    default: {
      omote_log_d("sendIRcode_HAL: WARNING: no defaults for nbits and repeat available for protocol %d\r\n", protocol);
      defaultParams[0] = 0; defaultParams[1] = kNoRepeat; return;
    }
  }
}
void sendIRcode_HAL(int protocol, std::list<std::string> commandPayloads, std::string additionalPayload) {

  // first determine if data was provided by commandPayload or by additionalPayload. Only one of these will be used.
  std::string::size_type sz = 0;   // alias of size_t
  std::string dataStr;
  uint64_t data;
  if (commandPayloads.empty() && (additionalPayload == "")) {
    omote_log_d("sendIRcode_HAL: cannot send IR command, because both data and payload are empty\r\n");
    return;
  } else {
    if (additionalPayload != "") {
      omote_log_d("sendIRcode_HAL: Setting data stream to additional payload.\r\n");
      dataStr = additionalPayload;
    } else {
      auto current = commandPayloads.begin();
      dataStr = *current;
    }
  }

  if (protocol == GLOBALCACHE) {
    // not a protocol, but an encoding
    // first create array of needed size
    std::string::difference_type size = std::count(dataStr.begin(), dataStr.end(), ',');
    size += 1;
    uint16_t *buf = new uint16_t[size];
    // now get comma separated values and fill array
    int pos = 0;
    std::stringstream ss(dataStr);
    while(ss.good())  {
      std::string valueStr;
      std::getline(ss, valueStr, ',');
      // https://cplusplus.com/reference/string/stoull/
      data = std::stoull(valueStr, &sz, 0);
      // omote_log_d("  next string value %s (%" PRIu64 ")\r\n", valueStr.c_str(), data);
      buf[pos] = data;
      pos += 1;
    }
    omote_log_d("sendIRcode_HAL: will send IR GC, array size %d\r\n", size);
    IrSender.sendGC(buf, size);
    delete [] buf;

  } else if (protocol == PRONTO) {
    // not a protocol, but an encoding
    omote_log_d("sendIRcode_HAL: protocol IR_PROTOCOL_PRONTO not yet implemented\r\n");

  } else if (protocol == RAW) {
    // not a protocol, but an encoding
    omote_log_d("sendIRcode_HAL: protocol IR_PROTOCOL_RAW not yet implemented\r\n");

  } else {
    // generic implementation for all other protocols

    // check if nbits and repeat have been provided in the command
    std::string::difference_type elementCount = std::count(dataStr.begin(), dataStr.end(), ':');
    if (elementCount == 0) {
      // no nbits and repeat have been provided in the command. Try to get defaults.
      uint16_t defaultBitsAndRepeat[2];
      getProtocolDefaultBitsAndRepeat(protocol, defaultBitsAndRepeat);
      if ((defaultBitsAndRepeat[0] == 0) && (defaultBitsAndRepeat[1] == kNoRepeat)) {
        omote_log_d("sendIRcode_HAL: either payload is expected as 'data:nbits:repeat', or defaults for nbits and repeat have to be defined. Will abort and not send IR code\r\n");
        return;
      } else {
        dataStr.append(":").append(std::to_string(defaultBitsAndRepeat[0])).append(":").append(std::to_string(defaultBitsAndRepeat[1]));
      }
    }
    // check if we now have the expected format 'data:nbits:repeat'
    elementCount = std::count(dataStr.begin(), dataStr.end(), ':');
    if (elementCount != 2) {
      omote_log_d("sendIRcode_HAL: payload is expected as 'data:nbits:repeat'. Will abort and not send IR code\r\n");
      return;
    }
  
    std::string valueAsStr;
    uint16_t nbits, repeat; 
    std::stringstream dataStrAsStream(dataStr);
    std::getline(dataStrAsStream, valueAsStr, ':');
    data = std::stoull(valueAsStr, nullptr, 0);
    std::getline(dataStrAsStream, valueAsStr, ':');
    nbits = std::stoul(valueAsStr, nullptr, 0);
    std::getline(dataStrAsStream, valueAsStr, ':');
    repeat = std::stoul(valueAsStr, nullptr, 0);

    omote_log_d("sendIRcode_HAL: will send data %s with protocol %d, data 0x%llx, nbits %u, repeat %u\r\n", dataStr.c_str(), protocol, data, nbits, repeat);
    IrSender.send((decode_type_t)protocol, data, nbits, repeat);
  }
}
