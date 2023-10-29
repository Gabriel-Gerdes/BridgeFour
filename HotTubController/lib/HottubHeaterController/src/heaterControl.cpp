#include <Arduino.h>
#ifndef SleepHi
  #include "../../../include/config.h"
#endif

// Declare local functions
void TurnOffHeater();
void TurnOnHeater();
void ThrowDeadMansSwitch(bool &DeadmanSwitchStatus);

void TurnOnHeater(){
      digitalWrite(Config::HEATERPIN,HIGH);
}

void TurnOffHeater(){
   digitalWrite(Config::HEATERPIN,LOW);
}
void ThrowDeadMansSwitch(bool &DeadmanSwitchStatus) {
  DeadmanSwitchStatus = false; //assign variable value by ref
  digitalWrite( Config::SAFETYPIN, DeadmanSwitchStatus);
  TurnOffHeater();
}
namespace heaterController {
  //----------------------------------------------------------------
  // Function declarations
  //----------------------------------------------------------------
  void OutGetTargetTemp(float &targetHi, float &targetLow);
  void SafetyCheck(float measuredTemperature, bool &DeadmanSwitchStatus);
  void SetHeatingStatus(float targetHi, float targetLow, unsigned int &HeatStatusRequest, float currentTemperature);
  void SetHeater(unsigned int heatingStatus);
  enum HeatingMode {
      NEITHER,
      HEATING,
      COOLING
  };

  // Exposed Functions
  void SafetyCheck(float measuredTemperature, bool &DeadmanSwitchStatus) {
  if (measuredTemperature < Config::SafetyMaxTemperature)
    {
      ThrowDeadMansSwitch(DeadmanSwitchStatus);
    }
  }

  void SetHeatingStatus(float targetHi, float targetLow, unsigned int &HeatStatusRequest , float currentTemperature ) {
    switch (HeatStatusRequest) {
      case heaterController::HeatingMode::NEITHER:
      case heaterController::HeatingMode::HEATING:
        if (currentTemperature < targetHi)
          HeatStatusRequest = heaterController::HeatingMode::COOLING;
        break;
      case heaterController::HeatingMode::COOLING:
        if (currentTemperature >  targetLow)
          HeatStatusRequest = heaterController::HeatingMode::HEATING;
        break;
    }
  }

  void SetHeater(unsigned int heatingStatus) {  
    switch (heatingStatus ) {
      case heaterController::HeatingMode::COOLING:
        TurnOffHeater( );
        break;
      case heaterController::HeatingMode::HEATING:
        TurnOnHeater();
        break;
      default:
        TurnOffHeater();
    }
  }

  void OutGetTargetTemp(float &targetHi, float &targetLow) {   // & passing variables by ref
    bool isSleep = digitalRead(Config::SLEEPSWITCH); 
    if (!isSleep) {
      targetHi = Config::Hi;
      targetLow = Config::Low;
    }
    else {
      targetHi = Config::SleepHi;
      targetLow = Config::SleepLow;
    }

  };

}