#pragma once

extern uint16_t AIRCONDITIONER_COMMAND;

void register_device_airconditioner();

// https://github.com/zeroflow/ESPAircon
// De'Longhi PAC N81
class AirConditionerPAC_N81 {

private:
  typedef struct {
    bool on;
    int mode;
    int fan;
    int temperature;
    bool unitF;
    bool timer;
    int timer_value;
  } dl_aircon_msg_t;
  dl_aircon_msg_t airconditioner_state;

  unsigned long dl_assemble_msg(dl_aircon_msg_t* msg);
  void dl_decode_msg(unsigned long code, dl_aircon_msg_t* msg);
  void dl_print_msg(dl_aircon_msg_t *msg);
  unsigned char bit_reverse( unsigned char x );

  uint8_t minTempC = 16;
  uint8_t maxTempC = 32;
  uint8_t minTempF = 61;
  uint8_t maxTempF = 89;

public:
  // set default state
  void init();
  // setter
  void set_onoff(bool state);
  void set_mode_next();
  void set_fan_next();
  void set_temp_up();
  void set_temp_down();
  void set_CF_next();
  void set_timer(bool state);
  void set_timer_value(int time_value);
  // getter
  std::string get_onoff_str();
  std::string get_mode_str();
  std::string get_fan_str();
  int get_temp();
  std::string get_temp_str();
  std::string get_CF_str();
  std::string get_timer_str();
  std::string get_timer_value_str();
  // send IR command
  void sendIRcommand();

protected:

};

extern AirConditionerPAC_N81 airConditionerPAC_N81;
