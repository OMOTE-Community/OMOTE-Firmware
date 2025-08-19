#include <Arduino.h>

#if (OMOTE_HARDWARE_REV <= 3)
  const uint8_t ADC_BAT_GPIO = 36;  // Battery voltage sense input (1/2 divider), GPIO36, ADC1_CH0, RTC_GPIO0
  // const uint8_t CRG_STAT_GPIO = 21; // battery charger feedback,                  GPIO21, VSPIHD, EMAC_TX_EN
#elif (OMOTE_HARDWARE_REV == 4)
  #include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
  // Initialize MAX17048 battery fuel gauge
  SFE_MAX1704X fuelGauge(MAX1704X_MAX17048);
  const uint8_t CRG_STAT_GPIO = 21; // battery charger feedback,                  GPIO21, VSPIHD, EMAC_TX_EN
#elif (OMOTE_HARDWARE_REV >= 5)
  #include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>
  // Initialize MAX17048 battery fuel gauge
  SFE_MAX1704X fuelGauge(MAX1704X_MAX17048);
  const uint8_t CRG_STAT_GPIO = 1;  // battery charger feedback
#endif

void init_battery_HAL(void) {
  #if (OMOTE_HARDWARE_REV >= 4)
    // Initialize battery charger indicator input
    pinMode(CRG_STAT_GPIO, INPUT_PULLUP);   

    fuelGauge.begin();
  #else
    // With hardware rev 3 the battery charge status cannot be recognized in a reliable way due to a design flaw in the PCB.
    // See https://github.com/CoretechR/OMOTE/issues/55
    // So charge status is deactivated for now.
    //pinMode(CRG_STAT_GPIO, INPUT_PULLUP);
    pinMode(ADC_BAT_GPIO, INPUT);
  #endif
}

void get_battery_status_HAL(int *battery_voltage, int *battery_percentage, bool *battery_ischarging) {
  #if (OMOTE_HARDWARE_REV >= 4)
    // With hardware rev 4, battery state of charge is monitored by a MAX17048 fuel gauge
    *battery_voltage = (int)(fuelGauge.getVoltage()*1000);
    float soc = fuelGauge.getSOC();
    if (soc > 100.0) soc = 100.0;
    *battery_percentage = (int)soc;
    *battery_ischarging = !digitalRead(CRG_STAT_GPIO);

    //Serial.print(" LiIon Voltage: ");
    //Serial.print(fuelGauge.getVoltage());  // Print the battery voltage
    //Serial.println("V");

    //Serial.print(" Percentage: ");
    //Serial.print(fuelGauge.getSOC(), 2); // Print the battery state of charge with 2 decimal places
    //Serial.println("%");
  #else
    int battery_analogRead = 0;

    battery_analogRead = analogRead(ADC_BAT_GPIO);
    // original values
    // battery_voltage = battery_analogRead*2*3300/4095 + 350; // 350mV ADC offset
    // adjusted values due to new measurements
    *battery_voltage = battery_analogRead*2*3350/4095 + 325;
    *battery_percentage = constrain(map(*battery_voltage, 3700, 4200, 0, 100), 0, 100);
    // Check if battery is charging, fully charged or disconnected
    /*
      "disconnected" cannot be recognized
      https://electronics.stackexchange.com/questions/615215/level-shifting-of-a-3-state-pin
      https://electrical.codidact.com/posts/286209
      https://how2electronics.com/lithium-ion-battery-charger-circuit-using-mcp73831/
    */
    //*battery_ischarging = !digitalRead(CRG_STAT_GPIO);
  #endif
}
