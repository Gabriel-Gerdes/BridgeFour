#include <Arduino.h>
#ifndef digitalWriteFast
  #include <digitalWriteFast.h>
#endif
#ifndef SleepHi
  #include "../../../include/config.h"
#endif

// Declare local functions
// void TurnOffHeater();
// void TurnOnHeater();
// void ThrowDeadMansSwitch(bool &DeadmanSwitchStatus);

void TurnOnHeater(){
  digitalWriteFast(Config::HEATERPIN,HIGH);
}

void TurnOffHeater(){
  digitalWriteFast(Config::HEATERPIN,LOW);
}

void ThrowDeadMansSwitch(bool &DeadmanSwitchStatus) {
  DeadmanSwitchStatus = false; //assign variable value by ref
  digitalWriteFast(Config::SAFETYPIN, DeadmanSwitchStatus);
}

namespace heaterController {
  //----------------------------------------------------------------
  // Function declarations
  //----------------------------------------------------------------
  // void OutGetTargetTemp(float &targetHi, float &targetLow);
  // void SafetyCheck(float measuredValue, bool &DeadmanSwitchStatus);
  // void SetHeatingStatus(float targetHi, float targetLow, unsigned int &HeatStatusRequest, float currentTemperature); 
  // void SetHeater(unsigned int heatingStatus);
  enum HeatingMode {
      NEITHER,
      HEATING,
      COOLING
  };

  void SetHeatingStatus(float targetHi, float targetLow, unsigned int &HeatStatusRequest , float currentTemperature ) {
    if (currentTemperature < targetLow){
      // set to heat
      HeatStatusRequest = heaterController::HeatingMode::HEATING;
    } else if ( currentTemperature < targetHi ) {
      // in neither mode we do not set temp, just let it stay in heat or cooling mode
      // to drift through the target range
      HeatStatusRequest = heaterController::HeatingMode::NEITHER;
    } else if ( currentTemperature >= targetHi ){
      HeatStatusRequest = heaterController::HeatingMode::COOLING;
    }
  }

  void SetHeater(unsigned int heatingStatus) {
    switch (heatingStatus ) {
      case heaterController::HeatingMode::COOLING:
        digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN)); 
        TurnOffHeater( );
        break;
      case heaterController::HeatingMode::HEATING:
        TurnOnHeater( );
        break;
      case heaterController::HeatingMode::NEITHER:
        //do nothing
        break;
    }
  }

  // Exposed Functions
  void SafetyCheck(float measuredValue, bool &DeadmanSwitchStatus) {
  // Use < for resistance, and > for Temperature values
  if (measuredValue < Config::SafetyMax) 
    {
      ThrowDeadMansSwitch(DeadmanSwitchStatus);
      SetHeater(heaterController::HeatingMode::COOLING);
    }
  }

  void OutGetTargetTemp(float &targetHi, float &targetLow) {   // & passing variables by ref
    bool isSleep = digitalReadFast(Config::SLEEPSWITCH); 
    if (!isSleep) {
      targetHi = Config::Hi;
      targetLow = Config::Low;
    }
    else {
      targetHi = Config::SleepHi;
      targetLow = Config::SleepLow;
    }
  }
}