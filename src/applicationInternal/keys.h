#pragma once

#include <map>
#include <stdint.h>

enum repeatModes {
  // only as fallback
  REPEAT_MODE_UNKNOWN,
  // if you short press or hold a key on the keypad, only one single command from keyCommands_short is sent
  // -> best used if you do not want a command to be sent more than once, even if you press the key (too) long, e.g. when toggling power
  SHORT,
  // if you hold a key on the keypad, a command from keyCommands_short is sent repeatedly
  // -> best used e.g. for holding the key for "volume up"
  SHORT_REPEATED,
  // if you short press a key, a command from keyCommands_short is sent once.
  // if you hold a key on the keypad, a command from keyCommands_long is sent (no command from keyCommands_short before)
  // -> best used if a long key press should send a different command than a short press
  SHORTorLONG,
};

typedef std::map<char, repeatModes> *key_repeatModes;
typedef std::map<char, uint16_t> *key_commands_short;
typedef std::map<char, uint16_t> *key_commands_long;

void keypad_loop(void);
