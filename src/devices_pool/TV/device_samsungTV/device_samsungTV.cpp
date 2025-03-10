#include <string>
#include "applicationInternal/commandHandler.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "device_samsungTV.h"

// Only activate the commands that are used. Every command takes 100 bytes, wether used or not.
// uint16_t SAMSUNG_POWER_TOGGLE    ; //"Samsung_power_toggle";
// uint16_t SAMSUNG_SOURCE          ; //"Samsung_source";
// uint16_t SAMSUNG_HDMI            ; //"Samsung_hdmi";
uint16_t SAMSUNG_NUM_1           ; //"Samsung_num_1";
uint16_t SAMSUNG_NUM_2           ; //"Samsung_num_2";
uint16_t SAMSUNG_NUM_3           ; //"Samsung_num_3";
uint16_t SAMSUNG_NUM_4           ; //"Samsung_num_4";
uint16_t SAMSUNG_NUM_5           ; //"Samsung_num_5";
uint16_t SAMSUNG_NUM_6           ; //"Samsung_num_6";
uint16_t SAMSUNG_NUM_7           ; //"Samsung_num_7";
uint16_t SAMSUNG_NUM_8           ; //"Samsung_num_8";
uint16_t SAMSUNG_NUM_9           ; //"Samsung_num_9";
uint16_t SAMSUNG_NUM_0           ; //"Samsung_num_0";
// uint16_t SAMSUNG_TTXMIX          ; //"Samsung_ttxmix";
// uint16_t SAMSUNG_PRECH           ; //"Samsung_prech";
// uint16_t SAMSUNG_VOL_MINUS       ; //"Samsung_vol_minus";
// uint16_t SAMSUNG_VOL_PLUS        ; //"Samsung_vol_plus";
// uint16_t SAMSUNG_MUTE_TOGGLE     ; //"Samsung_mute_toggle";
// uint16_t SAMSUNG_CHLIST          ; //"Samsung_chlist";
uint16_t SAMSUNG_CHANNEL_UP      ; //"Samsung_channel_up";
uint16_t SAMSUNG_CHANNEL_DOWN    ; //"Samsung_channel_down";
uint16_t SAMSUNG_MENU            ; //"Samsung_menu";
// uint16_t SAMSUNG_APPS            ; //"Samsung_apps";
uint16_t SAMSUNG_GUIDE           ; //"Samsung_guide";
// uint16_t SAMSUNG_TOOLS           ; //"Samsung_tools";
// uint16_t SAMSUNG_INFO            ; //"Samsung_info";
uint16_t SAMSUNG_UP              ; //"Samsung_up";
uint16_t SAMSUNG_DOWN            ; //"Samsung_down";
uint16_t SAMSUNG_LEFT            ; //"Samsung_left";
uint16_t SAMSUNG_RIGHT           ; //"Samsung_right";
uint16_t SAMSUNG_SELECT          ; //"Samsung_select";
// uint16_t SAMSUNG_RETURN          ; //"Samsung_return";
uint16_t SAMSUNG_EXIT            ; //"Samsung_exit";
// uint16_t SAMSUNG_KEY_A           ; //"Samsung_key_a";
// uint16_t SAMSUNG_KEY_B           ; //"Samsung_key_b";
// uint16_t SAMSUNG_KEY_C           ; //"Samsung_key_c";
// uint16_t SAMSUNG_KEY_D           ; //"Samsung_key_d";
// uint16_t SAMSUNG_FAMILYSTORY     ; //"Samsung_familystory";
// uint16_t SAMSUNG_SEARCH          ; //"Samsung_search";
// uint16_t SAMSUNG_DUALI_II        ; //"Samsung_duali-ii";
// uint16_t SAMSUNG_SUPPORT         ; //"Samsung_support";
// uint16_t SAMSUNG_PSIZE           ; //"Samsung_psize";
// uint16_t SAMSUNG_ADSUBT          ; //"Samsung_adsubt";
uint16_t SAMSUNG_REWIND          ; //"Samsung_rewind";
uint16_t SAMSUNG_PAUSE           ; //"Samsung_pause";
uint16_t SAMSUNG_FASTFORWARD     ; //"Samsung_fastforward";
// uint16_t SAMSUNG_RECORD          ; //"Samsung_record";
uint16_t SAMSUNG_PLAY            ; //"Samsung_play";
// uint16_t SAMSUNG_STOP            ; //"Samsung_stop";
uint16_t SAMSUNG_POWER_OFF       ; //"Samsung_power_off";
uint16_t SAMSUNG_POWER_ON        ; //"Samsung_power_on";
uint16_t SAMSUNG_INPUT_HDMI_1    ; //"Samsung_input_hdmi_1";
uint16_t SAMSUNG_INPUT_HDMI_2    ; //"Samsung_input_hdmi_2";
uint16_t SAMSUNG_INPUT_HDMI_3    ; //"Samsung_input_hdmi_3";
// uint16_t SAMSUNG_INPUT_HDMI_4    ; //"Samsung_input_hdmi_4";
// uint16_t SAMSUNG_INPUT_COMPONENT ; //"Samsung_input_component";
uint16_t SAMSUNG_INPUT_TV        ; //"Samsung_input_tv";

void register_device_samsungTV() {
  // tested with Samsung UE32EH5300, works also with others
  // both GC and SAMSUNG work well

  // https://github.com/natcl/studioimaginaire/blob/master/arduino_remote/ircodes.py
  // Only activate the commands that are used. Every command takes 100 bytes, wether used or not.
  // register_command(&SAMSUNG_POWER_TOGGLE      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E040BF"}));
  // register_command(&SAMSUNG_SOURCE            , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0807F"}));
  // register_command(&SAMSUNG_HDMI              , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0D12E"}));
  register_command(&SAMSUNG_NUM_1             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E020DF"}));
  register_command(&SAMSUNG_NUM_2             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0A05F"}));
  register_command(&SAMSUNG_NUM_3             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0609F"}));
  register_command(&SAMSUNG_NUM_4             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E010EF"}));
  register_command(&SAMSUNG_NUM_5             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0906F"}));
  register_command(&SAMSUNG_NUM_6             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E050AF"}));
  register_command(&SAMSUNG_NUM_7             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E030CF"}));
  register_command(&SAMSUNG_NUM_8             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0B04F"}));
  register_command(&SAMSUNG_NUM_9             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0708F"}));
  register_command(&SAMSUNG_NUM_0             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E08877"}));
  // register_command(&SAMSUNG_TTXMIX            , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E034CB"}));
  // register_command(&SAMSUNG_PRECH             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0C837"}));
  // register_command(&SAMSUNG_VOL_MINUS         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0D02F"}));
  // register_command(&SAMSUNG_VOL_PLUS          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0E01F"}));
  // register_command(&SAMSUNG_MUTE_TOGGLE       , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0F00F"}));
  // register_command(&SAMSUNG_CHLIST            , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0D629"}));
  register_command(&SAMSUNG_CHANNEL_UP        , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E048B7"}));
  register_command(&SAMSUNG_CHANNEL_DOWN      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E008F7"}));
  register_command(&SAMSUNG_MENU              , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E058A7"}));
  // register_command(&SAMSUNG_APPS              , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E09E61"}));
  register_command(&SAMSUNG_GUIDE             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0F20D"}));
  // register_command(&SAMSUNG_TOOLS             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0D22D"}));
  // register_command(&SAMSUNG_INFO              , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0F807"}));
  register_command(&SAMSUNG_UP                , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E006F9"}));
  register_command(&SAMSUNG_DOWN              , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E08679"}));
  register_command(&SAMSUNG_LEFT              , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0A659"}));
  register_command(&SAMSUNG_RIGHT             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E046B9"}));
  register_command(&SAMSUNG_SELECT            , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E016E9"}));
  // register_command(&SAMSUNG_RETURN            , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E01AE5"}));
  register_command(&SAMSUNG_EXIT              , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0B44B"}));
  // register_command(&SAMSUNG_KEY_A             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E036C9"}));
  // register_command(&SAMSUNG_KEY_B             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E028D7"}));
  // register_command(&SAMSUNG_KEY_C             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0A857"}));
  // register_command(&SAMSUNG_KEY_D             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E06897"}));
  // register_command(&SAMSUNG_FAMILYSTORY       , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0639C"}));
  // register_command(&SAMSUNG_SEARCH            , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0CE31"}));
  // register_command(&SAMSUNG_DUALI_II          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E000FF"}));
  // register_command(&SAMSUNG_SUPPORT           , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0FC03"}));
  // register_command(&SAMSUNG_PSIZE             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E07C83"}));
  // register_command(&SAMSUNG_ADSUBT            , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0A45B"}));
  register_command(&SAMSUNG_REWIND            , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0A25D"}));
  register_command(&SAMSUNG_PAUSE             , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E052AD"}));
  register_command(&SAMSUNG_FASTFORWARD       , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E012ED"}));
  // register_command(&SAMSUNG_RECORD            , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0926D"}));
  register_command(&SAMSUNG_PLAY              , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0E21D"}));
  // register_command(&SAMSUNG_STOP              , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0629D"}));
  register_command(&SAMSUNG_POWER_OFF         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E019E6"}));
  register_command(&SAMSUNG_POWER_ON          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E09966"}));
  register_command(&SAMSUNG_INPUT_HDMI_1      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E09768"}));
  register_command(&SAMSUNG_INPUT_HDMI_2      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E07D82"}));
  register_command(&SAMSUNG_INPUT_HDMI_3      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E043BC"}));
  // register_command(&SAMSUNG_INPUT_HDMI_4      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0A35C"}));
  // register_command(&SAMSUNG_INPUT_COMPONENT   , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0619E"}));
  register_command(&SAMSUNG_INPUT_TV          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0D827"}));
  // unknown commands. Not on my remote
  // register_command(&-                         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E0C43B"}));
  // register_command(&favorite_channel          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_SAMSUNG), "0xE0E022DD"}));

  // GC also works well
  //register_command(&SAMSUNG_POWER_TOGGLE      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_GLOBALCACHE), "38000,1,1,170,170,20,63,20,63,20,63,20,20,20,20,20,20,20,20,20,20,20,63,20,63,20,63,20,20,20,20,20,20,20,20,20,20,20,20,20,63,20,20,20,20,20,20,20,20,20,20,20,20,20,63,20,20,20,63,20,63,20,63,20,63,20,63,20,63,20,1798"}));
  //register_command(&SAMSUNG_POWER_OFF         , makeCommandData(IR, {std::to_string(IR_PROTOCOL_GLOBALCACHE), "38000,1,1,173,173,21,65,21,65,21,65,21,21,21,21,21,21,21,21,21,21,21,65,21,65,21,65,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,65,21,65,21,21,21,21,21,65,21,65,21,65,21,65,21,21,21,21,21,65,21,65,21,21,21,1832"}));
  //register_command(&SAMSUNG_POWER_ON          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_GLOBALCACHE), "38000,1,1,172,172,22,64,22,64,22,64,22,21,22,21,22,21,22,21,22,21,22,64,22,64,22,64,22,21,22,21,22,21,22,21,22,21,22,64,22,21,22,21,22,64,22,64,22,21,22,21,22,64,22,21,22,64,22,64,22,21,22,21,22,64,22,64,22,21,22,1820"}));
  //register_command(&SAMSUNG_INPUT_HDMI_1      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_GLOBALCACHE), "38000,1,1,173,173,21,65,21,65,21,65,21,21,21,21,21,21,21,21,21,21,21,65,21,65,21,65,21,21,21,21,21,21,21,21,21,21,21,65,21,21,21,21,21,65,21,21,21,65,21,65,21,65,21,21,21,65,21,65,21,21,21,65,21,21,21,21,21,21,21,1832"}));
  //register_command(&SAMSUNG_INPUT_HDMI_2      , makeCommandData(IR, {std::to_string(IR_PROTOCOL_GLOBALCACHE), "38000,1,1,173,173,21,65,21,65,21,65,21,21,21,21,21,21,21,21,21,21,21,65,21,65,21,65,21,21,21,21,21,21,21,21,21,21,21,21,21,65,21,65,21,65,21,65,21,65,21,21,21,65,21,65,21,21,21,21,21,21,21,21,21,21,21,65,21,21,21,1832"}));
  //register_command(&SAMSUNG_INPUT_TV          , makeCommandData(IR, {std::to_string(IR_PROTOCOL_GLOBALCACHE), "38000,1,1,172,172,21,64,21,64,21,64,21,21,21,21,21,21,21,21,21,21,21,64,21,64,21,64,21,21,21,21,21,21,21,21,21,21,21,64,21,64,21,21,21,64,21,64,21,21,21,21,21,21,21,21,21,21,21,64,21,21,21,21,21,64,21,64,21,64,21,1673"}));
}
