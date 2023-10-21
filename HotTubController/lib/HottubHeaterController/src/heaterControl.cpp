#ifndef digitalWrite
  #include <Arduino.h>
#endif
#ifndef _deadManSwitch
  #include "../include/skeleton.h"
#endif
#ifndef SAFETYPIN
  #include "../include/config.h"
#endif

void SafetyCheck(float measuredTemperature) {
#ifndef Saftey

#endif
  if (measuredTemperature < Safety)
  {
    ThrowDeadMansSwitch();
  }
}
void ThrowDeadMansSwitch() {
  _deadManSwitch = false;
  digitalWrite(SAFETYPIN, _deadManSwitch);
  TurnOffHeater();
}


void SetHeatingStatus(float targetHi, float targetLow) {
  switch (_heatingStatus) {
    case HeatControl::NEITHER:
    case HeatControl::HEATING:
      if (_emaTemperaturePreHeater < targetHi)
        _heatingStatus = HeatControl::COOLING;
      break;
    case HeatControl::COOLING:
      if (_emaTemperaturePreHeater > targetLow)
        _heatingStatus = HeatControl::HEATING;
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

void TurnOffHeater(){
   digitalWrite(HEATERPIN, LOW);
}

void TurnOnHeater(){
      digitalWrite(HEATERPIN, HIGH);
}

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
