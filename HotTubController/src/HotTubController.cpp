#include <Arduino.h>
#include <math.h>
// Now using patformio instead of the arduino ide / arduino vscode extention
// Converting from .ino to a .cpp source file as C++ file...
// This means that we need to declare each custom function see:
// https://docs.platformio.org/en/latest/faq/ino-to-cpp.html

#include "../include/config.h"
#include "../include/skeleton.h"
#include "../lib/HottubCalculations/src/calculations.cpp"
#include "../lib/HottubHeaterController/src/heaterControl.cpp"
#if (REPORTINGLEVEL !=0)
  #include "../lib/NaiveLogger/src/naiveLogger.cpp"
#endif

// enable soft reset
void(* resetFunc) (void) = 0;

// Initilzation
void setup(void) {
  // Check serial rates at: https://wormfood.net/avrbaudcalc.php
  // Uno typically has a 16Mhz crystal, could use conditional compilation arguments here to optimize for specific boards.
  Serial.begin(1000000);  //Serial.begin(9600); 


  pinMode(HEATERPIN, OUTPUT);
  pinMode(SAFETYPIN, OUTPUT);
  pinMode(SLEEPSWITCH, INPUT_PULLUP);
  #if (REPORTINGLEVEL !=0)
    _fileCompiledInfo = outFileCompiledInfo();
  #endif
  // initialize digital pin LED_BUILTIN as an output. Onboard LED and D13 for Uno/Duo/Mega (all but Gemma and MKR100)
  pinMode(LED_BUILTIN, OUTPUT);

  // initalize global variables
  _previousRunTime = 0;
  _previousRunCycles = 0;

  _isSleep = false;
  _deadManSwitch = true;

  HeatControl::HeatingMode _heatingStatus = HeatControl::HeatingMode::NEITHER;


  //_emaTemperaturePreHeater = 0.0f;
  //_emaTemperaturePostHeater = 0.0f;
  //_emaSafetyTemperaturePreHeater = 0.0f;
  //_emaSafetyTemperaturePostHeater = 0.0f;
  // Get intial resistance values on initialization, 
  // so we don't have to wait for the value to ramp up to valid values from 0
  _emaTemperaturePreHeater = degree_f_from_resistance(CalculateResistance(analogRead(THERMISTORPINPREHEATER)));
  _emaTemperaturePostHeater =  degree_f_from_resistance(CalculateResistance(analogRead(THERMISTORPINPOSTHEATER)));
  _emaSafetyTemperaturePreHeater = _emaTemperaturePreHeater;
  _emaSafetyTemperaturePostHeater = _emaTemperaturePostHeater;
}

//Exectuion Loop
void loop(void) {
  //Allways execute
  //TODO: Add checks against DeadMansSwitch
  float readingPreHeater = analogRead(THERMISTORPINPREHEATER);
  float readingPostHeater = analogRead(THERMISTORPINPOSTHEATER);
  //TODO: #3 handle common Thermistor Read errors 
  //  - readingPreHeater > 1018 :A a detached thermistor, but SERIESRESISTOR is still in place
  //  - readingPreHeater < 7 : thermistor is grounded or A0 is detached
  //  - readingPreHeaters that frequently cycle from 0 through 1023 
  //    occures when A0 is connected, but not powered or grounded.
  //    might need to capture a min / max value between action cycles, and if the diff exceeds some threshold, throw deadman switch
  float measuredResistancePreHeater = CalculateResistance(readingPreHeater);
  float measuredResistancePostHeater = CalculateResistance(readingPostHeater);
  
  float temperaturePreHeater = degree_f_from_resistance(measuredResistancePreHeater);
  float temperaturePostHeater = degree_f_from_resistance(measuredResistancePostHeater);

   // Calculate the EMA of the measured resistance.
  _emaTemperaturePreHeater = CalculateExponentialMovingAverage(_alpha,_emaTemperaturePreHeater, temperaturePreHeater);
  _emaTemperaturePostHeater = CalculateExponentialMovingAverage(_alpha,_emaTemperaturePostHeater, temperaturePostHeater);
  // _emaSafetyTemperaturePreHeater is much more responsive than _emaTemperaturePreHeater
  _emaSafetyTemperaturePreHeater = CalculateExponentialMovingAverage(_alphaSafety,_emaSafetyTemperaturePreHeater, temperaturePreHeater);
  _emaSafetyTemperaturePostHeater = CalculateExponentialMovingAverage(_alphaSafety,_emaSafetyTemperaturePostHeater, temperaturePostHeater);
  //unsigned long currentRunTime = millis(); 
  
  unsigned long currentRunTime = millis(); 
  // The number of milliseconds since board's last reset
  // Unsigned Long is 32 bit and overflows after 4,294,967,295  (2^32-1)
  // millis overflows ever 49.8 days
  // an unsigned negitive value is a positive value
  //for testing time overflows by using micros() as it oveflowed in 70 minutes showed no issues durring overflow as of 2023-10-15   
  // no need for this level of granularity -> micros();

  // Perform saftey checks more frequently than actions

  // Initialize msgToReport
  #if (REPORTINGLEVEL !=0)
    Report::ReportMessage msgToReport;
  #endif
  
  if ((unsigned long)(currentRunTime - _previousRunTime) > (SAFETY_INTERVAL-1)) {
   // only do safety checks if SAFETY_INTERVAL has passed
    SafetyCheck(_emaSafetyTemperaturePreHeater);
    // only after install -> SafetyCheck(_emaSafetyTemperaturePostHeater);  
    #if (REPORTINGLEVEL !=0)
      if (_deadManSwitch == false){
        msgToReport = Report::ReportMessage::MsgErrorDeadMan;
      }
    #endif
    #if (REPORTINGLEVEL == 2)
      outReport(
        msgToReport, 
        measuredResistancePreHeater,
        measuredResistancePostHeater,
        temperaturePreHeater,
        temperaturePostHeater
      );    
    #endif
  }

  // Perform actions based on calculations / state at ACTION_INTERVAL time
  if ((unsigned long)(currentRunTime - _previousRunTime) > (ACTION_INTERVAL-1)) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Toggle the LED on or off, just a i'm alive indicator
    // only take actions if ACTION_INTERVAL has passed
    if (_deadManSwitch != false) {
      float targetHi;
      float targetLow;
      // sets what our hi and low should be
      OutGetTargetTemp(targetHi, targetLow);
      SetHeatingStatus(targetHi, targetLow);
      SetHeater();
      #if (REPORTINGLEVEL != 0)
        msgToReport=Report::ReportMessage::MsgRoutine;
      #endif
    }
    
    #if (REPORTINGLEVEL == 1)
      outReport(
        msgToReport, 
        measuredResistancePreHeater,
        measuredResistancePostHeater,
        temperaturePreHeater,
        temperaturePostHeater
      );
    #elif (REPORTINGLEVEL == 0)
      // just an I'm alive written out to serial if we don't have any reporting
      // Serial.print(_previousRunCycles);
      //Serial.print(":");
      Serial.println(micros());
    #endif

    // Reset previous run variables
    _previousRunCycles = 0;
    _previousRunTime = currentRunTime;  
  } else {
    //execute only on non-action loop
    _previousRunCycles++;
    if (_previousRunCycles > (unsigned long) (pow(2,8*sizeof(_previousRunCycles))-2)){
      //if _previousRunCycles is about to overflow then reset board
      #if (REPORTINGLEVEL !=0)
        msgToReport=Report::ReportMessage::MsgErrorRebootPriorToOverflow;
        outReport(
          msgToReport,
          measuredResistancePreHeater,
          measuredResistancePostHeater,
          temperaturePreHeater,
          temperaturePostHeater
        );
      #endif
      resetFunc();
    }
  }
  #if (REPORTINGLEVEL == 3)
    outReport(
      msgToReport, 
      measuredResistancePreHeater,
      measuredResistancePostHeater,
      temperaturePreHeater,
      temperaturePostHeater
    );
  #endif
}

