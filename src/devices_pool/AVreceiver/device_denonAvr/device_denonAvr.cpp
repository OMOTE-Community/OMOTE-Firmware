#include <string>
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "device_denonAvr.h"

// Only activate the commands that are used. Every command takes 100 bytes, wether used or not.
// uint16_t DENON_POWER_TOGGLE      ; //"DENON_POWER_TOGGLE";
// uint16_t DENON_POWER_SLEEP       ; //"DENON_POWER_SLEEP";
// uint16_t DENON_VOL_MINUS         ; //"DENON_VOL_MINUS";
// uint16_t DENON_VOL_PLUS          ; //"DENON_VOL_PLUS";
// uint16_t DENON_VOL_MUTE          ; //"DENON_VOL_MUTE";
// uint16_t DENON_CHAN_PLUS         ; //"DENON_CHAN_PLUS";
// uint16_t DENON_CHAN_MINUS        ; //"DENON_CHAN_MINUS";
// uint16_t DENON_INPUT_CABLESAT    ; //"DENON_INPUT_CABLESAT";
// uint16_t DENON_INPUT_MEDIAPLAYER ; //"DENON_INPUT_MEDIAPLAYER";
// uint16_t DENON_INPUT_BLURAY      ; //"DENON_INPUT_BLURAY";
// uint16_t DENON_INPUT_GAME        ; //"DENON_INPUT_GAME";
// uint16_t DENON_INPUT_AUX1        ; //"DENON_INPUT_AUX1";
// uint16_t DENON_INPUT_AUX2        ; //"DENON_INPUT_AUX2";
// uint16_t DENON_INPUT_PHONO       ; //"DENON_INPUT_PHONO";
// uint16_t DENON_INPUT_TUNER       ; //"DENON_INPUT_TUNER";
// uint16_t DENON_INPUT_TV          ; //"DENON_INPUT_TV";
// uint16_t DENON_INPUT_USB         ; //"DENON_INPUT_USB";
// uint16_t DENON_INPUT_BLUETOOTH   ; //"DENON_INPUT_BLUETOOTH";
// uint16_t DENON_INPUT_INTERNET    ; //"DENON_INPUT_INTERNET";
// uint16_t DENON_INPUT_HEOS        ; //"DENON_INPUT_HEOS";
// uint16_t DENON_POWER_ECO         ; //"DENON_POWER_ECO";
// uint16_t DENON_INFO              ; //"DENON_INFO";
// uint16_t DENON_OPTION            ; //"DENON_OPTION";
// uint16_t DENON_BACK              ; //"DENON_BACK";
// uint16_t DENON_SETUP             ; //"DENON_SETUP";
// uint16_t DENON_MENU_ENTER        ; //"DENON_MENU_ENTER";
// uint16_t DENON_MENU_UP           ; //"DENON_MENU_UP";
// uint16_t DENON_MENU_LEFT         ; //"DENON_MENU_LEFT";
// uint16_t DENON_MENU_RIGHT        ; //"DENON_MENU_RIGHT";
// uint16_t DENON_MENU_DOWN         ; //"DENON_MENU_DOWN";
// uint16_t DENON_SOUNDMODE_MOVIE   ; //"DENON_SOUNDMODE_MOVIE";
// uint16_t DENON_SOUNDMODE_MUSIC   ; //"DENON_SOUNDMODE_MUSIC";
// uint16_t DENON_SOUNDMODE_GAME    ; //"DENON_SOUNDMODE_GAME";
// uint16_t DENON_SOUNDMODE_PURE    ; //"DENON_SOUNDMODE_PURE";
// uint16_t DENON_QUICKSELECT_1     ; //"DENON_QUICKSELECT_1";
// uint16_t DENON_QUICKSELECT_2     ; //"DENON_QUICKSELECT_2";
// uint16_t DENON_QUICKSELECT_3     ; //"DENON_QUICKSELECT_3";
// uint16_t DENON_QUICKSELECT_4     ; //"DENON_QUICKSELECT_4";
// uint16_t DENON_MEDIA_PREV        ; //"DENON_MEDIA_PREV";
// uint16_t DENON_MEDIA_PLAYPAUSE   ; //"DENON_MEDIA_PLAYPAUSE";
// uint16_t DENON_MEDIA_NEXT        ; //"DENON_MEDIA_NEXT";

void register_device_denonAvr() {
  // tested with Denon AVR-S660H, works also with others

  // Only activate the commands that are used. Every command takes 100 bytes, wether used or not.
  // register_command(&DENON_POWER_TOGGLE      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028A0088", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_POWER_SLEEP       , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C02822CAC", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_VOL_MINUS         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C0288E862", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_VOL_PLUS          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C0280E86A", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_VOL_MUTE          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C0284E86E", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_CHAN_PLUS         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C0288DC56", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_CHAN_MINUS        , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C0284DC5A", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_INPUT_CABLESAT    , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028CB43A", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_INPUT_MEDIAPLAYER , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C0286B430", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_INPUT_BLURAY      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C0288B43E", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_INPUT_GAME        , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028AB43C", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_INPUT_AUX1        , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C0289B43F", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_INPUT_AUX2        , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C0285B433", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_INPUT_PHONO       , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028034B6", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_INPUT_TUNER       , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028F34B9", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_INPUT_TV          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C0284B432", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_INPUT_USB         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028734B1", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_INPUT_BLUETOOTH   , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028F74F9", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_INPUT_INTERNET    , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028A74FC", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_INPUT_HEOS        , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028E34B8", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_POWER_ECO         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C02816CEF", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_INFO              , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C0280E466", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_OPTION            , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028ADC54", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_BACK              , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028440C6", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_SETUP             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028C40CE", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_MENU_ENTER        , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028F800D", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_MENU_UP           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028D800F", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_MENU_LEFT         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028B8009", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_MENU_RIGHT        , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C02878005", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_MENU_DOWN         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C02838001", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_SOUNDMODE_MOVIE   , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028928A3", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_SOUNDMODE_MUSIC   , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028528AF", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_SOUNDMODE_GAME    , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028D28A7", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_SOUNDMODE_PURE    , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028AC840", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_QUICKSELECT_1     , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028248C8", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_QUICKSELECT_2     , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028A48C0", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_QUICKSELECT_3     , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028648CC", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_QUICKSELECT_4     , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028E48C4", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_MEDIA_PREV        , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C028C7CF2", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_MEDIA_PLAYPAUSE   , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C02807CFE", kDenon48Bits, kNoRepeat)}));
  // register_command(&DENON_MEDIA_NEXT        , makeCommandData(IR, {std::to_string(IR_PROTOCOL_DENON), concatenateIRsendParams("0x2A4C02827CFC", kDenon48Bits, kNoRepeat)}));

}
