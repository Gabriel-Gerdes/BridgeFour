#ifndef Serial
  #include <Arduino.h>
#endif
#include "../include/config.h"
#include "../lib/HottubCalculations/src/calculations.cpp"
#include "../lib/HottubHeaterController/src/heaterControl.cpp"
#if REPORTINGLEVEL != 0
  #ifndef outFileCompiledInfo 
    #include "../lib/NaiveLogger/src/naiveLogger.cpp"
  #endif
#endif

// Now using patformio instead of the arduino ide / arduino vscode extention
// Converting from .ino to a .cpp source file as C++ file...
// This means that we need to declare each custom function see:
// https://docs.platformio.org/en/latest/faq/ino-to-cpp.html


//----------------------------------------------------------------
// Global variables
//----------------------------------------------------------------
// by convention every variable begining with an underscore in this project is a global varaible.
// really need to refactor these from external global variables to some other method of 
// passing this information arround, could use pointers? 
// normally we just use unsigned int and long, the arduino uno is a 8-bit machine, 
// so using sized ints to save space. https://www.gnu.org/software/libc/manual/html_node/Integers.html
unsigned int _heatingStatusRequest = heaterController::HeatingMode::NEITHER;
unsigned long _previousRunTime ;
unsigned long _previousRunCycles ;

bool _isSleep;
bool _deadManSwitchStatus;

float _emaTemperaturePreHeater;
float _emaTemperaturePostHeater;
float _emaSafetyTemperaturePreHeater;
float _emaSafetyTemperaturePostHeater;
// consider using a struct for all the temperature measures related to a themistor...
//struct TempratureMeasures {
//  float Resistance;
//  float Temperature;
//  float TemperatureExponentialMovingAverage;
//  float TemperatureCumulativeAverage;
//  unsigned int ThermistorId; // what specific thermistor are we talking about?
//}
// 

// enable soft reset
void(* resetFunc) (void) = 0;

// Initilzation
void setup(void) {
  // Check serial rates at: https://wormfood.net/avrbaudcalc.php
  // Uno typically has a 16Mhz crystal, could use conditional compilation arguments here to optimize for specific boards.

  pinMode(Config::HEATERPIN, OUTPUT);
  pinMode(Config::SAFETYPIN, OUTPUT);
  pinMode(Config::SLEEPSWITCH, INPUT_PULLUP);
  #if (REPORTINGLEVEL !=0)
    Serial.begin(Config::SEARIALBAUDRATE); 
    naiveLogger::outFileCompiledInfo();
  #endif
  // initialize digital pin LED_BUILTIN as an output. Onboard LED and D13 for Uno/Duo/Mega (all but Gemma and MKR100)
  pinMode(LED_BUILTIN, OUTPUT);

  // initalize global variables
  _previousRunTime = 0;
  _previousRunCycles = 0;

  _isSleep = false;
  _deadManSwitchStatus = true;

  _heatingStatusRequest = heaterController::HeatingMode::NEITHER;


  //_emaTemperaturePreHeater = 0.0f;
  //_emaTemperaturePostHeater = 0.0f;
  //_emaSafetyTemperaturePreHeater = 0.0f;
  //_emaSafetyTemperaturePostHeater = 0.0f;
  // Get intial resistance values on initialization, 
  // so we don't have to wait for the value to ramp up to valid values from 0
  _emaTemperaturePreHeater = degree_f_from_resistance(CalculateResistance(analogRead(Config::THERMISTORPINPREHEATER),Config::SERIESRESISTOR));
  _emaTemperaturePostHeater =  degree_f_from_resistance(CalculateResistance(analogRead(Config::THERMISTORPINPOSTHEATER),Config::SERIESRESISTOR));
  _emaSafetyTemperaturePreHeater = _emaTemperaturePreHeater;
  _emaSafetyTemperaturePostHeater = _emaTemperaturePostHeater;
}

//Exectuion Loop
void loop(void) {
  //Allways execute
  //TODO: Add checks against DeadMansSwitch
  float readingPreHeater = analogRead(Config::THERMISTORPINPREHEATER);
  float readingPostHeater = analogRead(Config::THERMISTORPINPOSTHEATER);
  //TODO: #3 handle common Thermistor Read errors 
  //  - readingPreHeater > 1018 :A a detached thermistor, but SERIESRESISTOR is still in place
  //  - readingPreHeater < 7 : thermistor is grounded or A0 is detached
  //  - readingPreHeaters that frequently cycle from 0 through 1023 
  //    occures when A0 is connected, but not powered or grounded.
  //    might need to capture a min / max value between action cycles, and if the diff exceeds some threshold, throw deadman switch
  float measuredResistancePreHeater = CalculateResistance(readingPreHeater,Config::SERIESRESISTOR);
  float measuredResistancePostHeater = CalculateResistance(readingPostHeater,Config::SERIESRESISTOR);
  
  float temperaturePreHeater = degree_f_from_resistance(measuredResistancePreHeater);
  float temperaturePostHeater = degree_f_from_resistance(measuredResistancePostHeater);

   // Calculate the EMA of the measured resistance.
  CalculateExponentialMovingAverage(Config::alpha,_emaTemperaturePreHeater, temperaturePreHeater);
  CalculateExponentialMovingAverage(Config::alpha,_emaTemperaturePostHeater, temperaturePostHeater);
  // _emaSafetyTemperaturePreHeater is much more responsive than _emaTemperaturePreHeater
  CalculateExponentialMovingAverage(Config::alphaSafety,_emaSafetyTemperaturePreHeater, temperaturePreHeater);
  CalculateExponentialMovingAverage(Config::alphaSafety,_emaSafetyTemperaturePostHeater, temperaturePostHeater);
  //long currentRunTime = millis(); 
  
  long currentRunTime = millis(); 
  // The number of milliseconds since board's last reset
  // Unsigned Long is 32 bit (long) and overflows after 4,294,967,295  (2^32-1)
  // millis overflows ever 49.8 days
  // an unsigned negitive value is a positive value
  //for testing time overflows by using micros() as it oveflowed in 70 minutes showed no issues durring overflow as of 2023-10-15   
  // no need for this level of granularity -> micros();

  // Perform saftey checks more frequently than actions

  // Initialize msgToReport as a variable of type enum ReportMessage
  #if (REPORTINGLEVEL !=0)
    naiveLogger::ReportMessage msgToReport;
    msgToReport = naiveLogger::ReportMessage::MsgRoutine;
  #endif
  
  if ((long)(currentRunTime - _previousRunTime) > (Config::SAFETY_INTERVAL-1)) {
   // only do safety checks if Config::SAFETY_INTERVAL has passed
    heaterController::SafetyCheck(
      _emaSafetyTemperaturePreHeater,
      _deadManSwitchStatus
    );
    // only after install -> heaterController::SafetyCheck(_emaSafetyTemperaturePostHeater);  
    #if (REPORTINGLEVEL !=0)
      if (_deadManSwitchStatus == false){
        msgToReport = naiveLogger::ReportMessage::MsgErrorDeadMan;
      };
    #endif
    #if (REPORTINGLEVEL == 2)
      outReport(
        msgToReport, 
        measuredResistancePreHeater,
        measuredResistancePostHeater,
        temperaturePreHeater,
        temperaturePostHeater,
        _previousRunCycles,
        _previousRunTime,
        _emaTemperaturePreHeater,
        _emaTemperaturePostHeater,
        _emaSafetyTemperaturePreHeater,
        _emaSafetyTemperaturePostHeater
      );    
    #endif
  }

  // Perform actions based on calculations / state at ACTION_INTERVAL time
  if ((long)(currentRunTime - _previousRunTime) > (Config::ACTION_INTERVAL-1)) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Toggle the LED on or off, just a i'm alive indicator
    // only take actions if ACTION_INTERVAL has passed
    if (_deadManSwitchStatus != false) {
      float targetHi;
      float targetLow;
      // sets what our hi and low should be
      heaterController::OutGetTargetTemp(targetHi, targetLow);
      heaterController::SetHeatingStatus(targetHi, targetLow,_heatingStatusRequest,_emaTemperaturePreHeater);
      heaterController::SetHeater(_heatingStatusRequest);
      #if (REPORTINGLEVEL != 0)
        msgToReport=naiveLogger::ReportMessage::MsgRoutine;
      #endif
    }
    
    #if (REPORTINGLEVEL == 1)
      outReport(
        msgToReport, 
        measuredResistancePreHeater,
        measuredResistancePostHeater,
        temperaturePreHeater,
        temperaturePostHeater,
        _previousRunCycles,
        _previousRunTime,
        _emaTemperaturePreHeater,
        _emaTemperaturePostHeater,
        _emaSafetyTemperaturePreHeater,
        _emaSafetyTemperaturePostHeater
      );
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
        msgToReport=naiveLogger::ReportMessage::MsgErrorRebootPriorToOverflow;
        outReport(
          msgToReport,
          measuredResistancePreHeater,
          measuredResistancePostHeater,
          temperaturePreHeater,
          temperaturePostHeater,
        _previousRunCycles,
        _previousRunTime,
        _emaTemperaturePreHeater,
        _emaTemperaturePostHeater,
        _emaSafetyTemperaturePreHeater,
        _emaSafetyTemperaturePostHeater
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
      temperaturePostHeater,
        _previousRunCycles,
        _previousRunTime,
        _emaTemperaturePreHeater,
        _emaTemperaturePostHeater,
        _emaSafetyTemperaturePreHeater,
        _emaSafetyTemperaturePostHeater
    );
  #endif
}