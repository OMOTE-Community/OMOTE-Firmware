#include "applicationInternal/commandHandler.h"
#include "applicationInternal/omote_log.h"
#include "applicationInternal/hardware/hardwarePresenter.h"
#include "device_airconditioner.h"

uint16_t AIRCONDITIONER_COMMAND;

void register_device_airconditioner() {
  register_command(&AIRCONDITIONER_COMMAND    , makeCommandData(IR, {std::to_string(IR_PROTOCOL_NEC)}));

  airConditionerPAC_N81.init();
}

AirConditionerPAC_N81 airConditionerPAC_N81;

void AirConditionerPAC_N81::init() {
  // this is the initial state we assume after startup.
  // Nibble 1: 1
  // Nibble 2: 2
  // Nibble 3: fan level
  airconditioner_state.fan         = 2;     // 1=high, 2=medium, 4=low
  // Nibble 4: mode
  airconditioner_state.mode        = 8;     // 8=A/C, 2=Dehumidify, 1=Fan only
  // Nibble 5: timer
  airconditioner_state.timer_value = 0;     // Hours until the timer action (on/off) will be executed
  // Nibble 6: |  |CF|T |ON|
  // 4 bits
  airconditioner_state.unitF       = false;
  airconditioner_state.timer       = false; // Enables the timer. When unit is off it serves as the switch-on timer, if the unit is on it switches off after the interval
  airconditioner_state.on          = false;
  // Nibble 7+8: temp
  airconditioner_state.temperature = 24;

/*
  // print this message
  omote_log_d("AC inital state:\r\n");
  dl_print_msg(&airconditioner_state);
  // convert it to hex command
  unsigned long msg_on_hex = dl_assemble_msg(&airconditioner_state);
  omote_log_d("AC hex command: 0x%lx\r\n", msg_on_hex);
  // dec 304611600 hex 0x12280110

  // sanity check: decode it back and print it
  omote_log_d("Sanity check: hex converted back to message\r\n");
  dl_aircon_msg_t msg_on_decoded;
  dl_decode_msg(msg_on_hex, &msg_on_decoded);
  dl_print_msg(&msg_on_decoded);
*/
}

/*
this is how the AC changes temperature when unit changes between C and F:
F     C     F
89 -> 31 -> 89
90 -> 32 -> 90
61 -> 16 -> 61
*/

void AirConditionerPAC_N81::set_onoff(bool state) {
  airconditioner_state.on = state;
}
void AirConditionerPAC_N81::set_mode_next() {
  switch (airconditioner_state.mode) {
    case 1: {airconditioner_state.mode = 2; break;}
    case 2: {airconditioner_state.mode = 8; break;}
    case 8: {airconditioner_state.mode = 1; break;}
  }
}
void AirConditionerPAC_N81::set_fan_next() {
  switch (airconditioner_state.fan) {
    case 1: {airconditioner_state.fan = 4; break;}
    case 2: {airconditioner_state.fan = 1; break;}
    case 4: {airconditioner_state.fan = 2; break;}
  }
}
void AirConditionerPAC_N81::set_temp_up() {
  if (!airconditioner_state.unitF) {
    // Celsius
    if (airconditioner_state.temperature < maxTempC) {
      airconditioner_state.temperature++;
    }
  } else {
    // Fahrenheit
    if (airconditioner_state.temperature < maxTempF) {
      airconditioner_state.temperature++;
    }
  }
}
void AirConditionerPAC_N81::set_temp_down() {
  if (!airconditioner_state.unitF) {
    // Celsius
    if (airconditioner_state.temperature > minTempC) {
      airconditioner_state.temperature--;
    }
  } else {
    // Fahrenheit
    if (airconditioner_state.temperature > minTempF) {
      airconditioner_state.temperature--;
    }
  }
}
void AirConditionerPAC_N81::set_CF_next() {
  if (airconditioner_state.unitF) {
    // change from Fahrenheit to Celsius
    airconditioner_state.temperature = (float(airconditioner_state.temperature - 32) * 5/9 + 0.5);
    airconditioner_state.unitF = false;
  } else {
    // change from Celsius to Fahrenheit
    airconditioner_state.temperature = (float(airconditioner_state.temperature     ) * 9/5 + 0.5) + 32;
    airconditioner_state.unitF = true;
  }
}
void AirConditionerPAC_N81::set_timer(bool state) {
  airconditioner_state.timer = state;
}
void AirConditionerPAC_N81::set_timer_value(int timer_value) {
  if ((timer_value >=0) && (timer_value <= 12)) {
    airconditioner_state.timer_value = timer_value;
  }
}

std::string AirConditionerPAC_N81::get_onoff_str() {
  if (airconditioner_state.on) {
    return "on";
  } else {
    return "off";
  }
}
std::string AirConditionerPAC_N81::get_mode_str() {
  switch (airconditioner_state.mode) {
    case 1: {return "fan only"; break;}
    case 2: {return "dehumidify"; break;}
    case 8: {return "A/C"; break;}
  }
  return "";
}
std::string AirConditionerPAC_N81::get_fan_str() {
  switch (airconditioner_state.fan) {
    case 1: {return "high"; break;}
    case 2: {return "medium"; break;}
    case 4: {return "low"; break;}
  }
  return "";
}
int AirConditionerPAC_N81::get_temp() {
  return airconditioner_state.temperature;
}
std::string AirConditionerPAC_N81::get_temp_str() {
  return std::to_string(airconditioner_state.temperature);
}
std::string AirConditionerPAC_N81::get_CF_str() {
  if (airconditioner_state.unitF) {
    return "F";
  } else {
    return "C";
  }
}
std::string AirConditionerPAC_N81::get_timer_str() {
  if (airconditioner_state.timer) {
    return "timer on";
  } else {
    return "timer off";
  }
}
std::string AirConditionerPAC_N81::get_timer_value_str() {
  return std::to_string(airconditioner_state.timer_value);
}


// https://github.com/zeroflow/ESPAircon
/*
 * DeLonghi PAC N81 IR Commands
 * 
 * 32 Bit Value
 * NEC Encoding
 * 
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |   0x01    |   0x02    |  |Lo|Md|Hi|Ac|  |Dh|Bl|
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * | Timer     |  |CF|T |ON|    Temperature        |
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * 
 * 0x01 and 0x02 is a fixed prefix
 * Lo / Md / Hi are bits for the fan setting
 * Cl / Dh / Bl are mode switches
 *    Ac  Airconditioning
 *    Dh  Dehumidify
 *    Bl  Blow (Fan Mode)
 * Timer is the set value for timer control.     
 *    Set to 1 even if not active via the bit
 * CF   Celsius / Fahrenheit Setting. Low C, High F
 * T    Timer Low Off, High On
 * On   On/Off Switch
 * 
 * Temperature    8-Bit Temperature Value
 * 
 * 0-16 means 16-32°C
 * 61-89 means 61-89°F
 * 
 * Both the temperature and timer have their bit-order reversed.
 */

unsigned long AirConditionerPAC_N81::dl_assemble_msg(dl_aircon_msg_t* msg){
  unsigned long buf = 0x12000000;

  if (!msg->unitF){
    // msg->temperature = constrain(msg->temperature, 16, 32);
    msg->temperature = (msg->temperature) < minTempC ? minTempC : ((msg->temperature) > maxTempC ? maxTempC : (msg->temperature));
    buf |= bit_reverse(msg->temperature-16);
  }else{
    // msg->temperature = constrain(msg->temperature, 61, 89);
    msg->temperature = (msg->temperature) < minTempF ? minTempF : ((msg->temperature) > maxTempF ? maxTempF : (msg->temperature));
    buf |= bit_reverse(msg->temperature);
  }

  buf |= bit_reverse(msg->timer_value) << 8;

  if (msg->on)    buf |= 0x1 << 8;
  if (msg->timer) buf |= 0x1 << 9;
  if (msg->unitF) buf |= 0x1 << 10;

  if (msg->mode == 8 || msg->mode == 2 || msg->mode == 1){
    buf |= msg->mode << 16;
  }else{
    buf |= 0x8 << 16;
  }

  if (msg->fan == 4 || msg->fan == 2 || msg->fan == 1){
    buf |= msg->fan << 20;
  }else{
    buf |= 0x2 << 20;
  }

  return buf;
}

void AirConditionerPAC_N81::dl_decode_msg(unsigned long code, dl_aircon_msg_t* msg){
  
  msg->on    = ( (code) >> 8  ) & 0x01;
  msg->timer = ( (code) >> 9  ) & 0x01;
  msg->unitF = ( (code) >> 10 ) & 0x01;

  msg->temperature = bit_reverse(code & 0xFF);
  if (!msg->unitF) { //degC has a -16 offset
      msg->temperature += 16;
  }

  msg->timer_value = bit_reverse((code >> 12) & 0xF) >> 4;
  msg->mode = (code >> 16) & 0xF;
  msg->fan  = (code >> 20) & 0xF;
}

void AirConditionerPAC_N81::dl_print_msg(dl_aircon_msg_t *msg){
  omote_log_d("Airconditioner settings:\r\n");

  omote_log_d("  %s: %d %s\r\n", get_onoff_str().c_str(), msg->temperature, get_CF_str().c_str());
  omote_log_d("  %s: %s\r\n",    get_mode_str().c_str(), get_fan_str().c_str());
  omote_log_d("  %s: %d\r\n",    get_timer_str().c_str(), msg->timer_value);

}

// Reverse the order of bits in a byte. 
// I.e. MSB is swapped with LSB, etc. 
unsigned char AirConditionerPAC_N81::bit_reverse( unsigned char x ) 
{ 
   x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa); 
   x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc); 
   x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0); 
   return x;    
}

void AirConditionerPAC_N81::sendIRcommand() {
  // print this message
  dl_print_msg(&airconditioner_state);
  // convert it to hex command
  unsigned long msg_on_hex = dl_assemble_msg(&airconditioner_state);
  //omote_log_d("AC hex command: 0x%lx\r\n", msg_on_hex);
  char buffer[12];
  sprintf(buffer, "0x%lx", msg_on_hex);
  //omote_log_d("buffer: %s\r\n", buffer);
  executeCommand(AIRCONDITIONER_COMMAND, std::string(buffer));

/*
  // sanity check: decode it back and print it
  omote_log_d("Sanity check: hex converted back to message\r\n");
  dl_aircon_msg_t msg_on_decoded;
  dl_decode_msg(msg_on_hex, &msg_on_decoded);
  dl_print_msg(&msg_on_decoded);
*/
}