#ifndef digitalWrite
  #include <Arduino.h>
#endif
#ifndef _deadManSwitch
  #include "../../../include/skeleton.h"
#endif
#ifndef SAFETYPIN
  #include "../../../include/config.h"
#endif

// Declare global variables used as external
extern bool _deadManSwitch;
extern HeatControl::HeatingMode _heatingStatus;
extern float _emaTemperaturePreHeater;

void TurnOffHeater(){
   digitalWrite(HEATERPIN, LOW);
}
void ThrowDeadMansSwitch() {
  _deadManSwitch = false;
  digitalWrite(SAFETYPIN, _deadManSwitch);
  TurnOffHeater();
}
void SafetyCheck(float measuredTemperature) {
if (measuredTemperature < Safety)
  {
    ThrowDeadMansSwitch();
  }
}
void SetHeatingStatus(float targetHi, float targetLow) {
  switch (_heatingStatus) {
    case HeatControl::HeatingMode::NEITHER:
    case HeatControl::HeatingMode::HEATING:
      if (_emaTemperaturePreHeater < targetHi)
        _heatingStatus = HeatControl::HeatingMode::COOLING;
      break;
    case HeatControl::HeatingMode::COOLING:
      if (_emaTemperaturePreHeater > targetLow)
        _heatingStatus = HeatControl::HeatingMode::HEATING;
      break;
  }
}
void SetHeater() {  
  switch (_heatingStatus) {
    case HeatControl::COOLING:
      TurnOffHeater();
      break;
    case HeatControl::HEATING:
      TurnOnHeater();
      break;
    default:
      TurnOffHeater();
  }
}

void TurnOnHeater();
void TurnOnHeater(){
      digitalWrite(HEATERPIN, HIGH);
}
void OutGetTargetTemp(float &targetHi, float &targetLow);
void OutGetTargetTemp(float &targetHi, float &targetLow) {   // & passing variables by ref
  bool isSleep = digitalRead(SLEEPSWITCH); 
  if (!isSleep) {
    targetHi = Hi;
    targetLow = Low;
  }
  else {
    targetHi = SleepHi;
    targetLow = SleepLow;
  }
}