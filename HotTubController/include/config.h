#pragma once
#include <Arduino.h>
// These are in the global namespace so that they can be used as conditional compliation arguments
#define DEBUG false // false
#define IGNOREDEADMANSWITCH false // false

#define REPORTINGFREQUENCY 1 // 1
//currently reporting levels are:
// 0 = no reporting
// 1 = typical reporting (Durring action)
// 2 = frequent reporting (Durring safety check)
// 3 = continious reporting (Every loop) over serial, slows cycles per action from 1000 to 100.

namespace Config {
  // Our project configuration items are found here
  // Why are these assigned to macros instead of simply being constants?
  const float SERIESRESISTOR = 10000; // installed resistor in parallel with the temp thermistor
  const int THERMISTORPINPREHEATER = A0;
  const int THERMISTORPINPOSTHEATER = A2;
  const unsigned long SEARIALBAUDRATE = 1000000; 
  //long is an integer range from 0 to 4,294,967,295, but a unit16_t is only 0 to 65,535 

  const float alpha = 0.0005f; 
  const float alphaSafety = 0.001f; 
  // Alpha is the smoothing factor for our Exponential Moving Average (ema) formula.
  // EMA smooths our measured temp value to remove any noise from the signal.
  // A higher alpha value will result in a smoother EMA, but it will also be less 
  // responsive to changes in the measured resistance.
  // currently we are sampling at about 1 mhz (4 mhz if only running one thermistor) from the thermistor 
  // as we add complexity to the sketch we will be sampling at a slower rate, 
  // so it's important to continue to increase our alpha to maintain responsiveness, and test.

  // [TODO] it is possible to calculate the alpha so that it's 
  // responsive to N degree / N time period based on measured sample rate

  //These are just the defaults, eventually we may want to override these with a UI.
  // also as an enum we can't use decimal values, may need to switch to a struct of floats
  enum TargetTempature  {
    //Sleep
    SleepHi = 85, // 24832 ohms  ,//85F
    SleepLow = 80, // 27931 ohms, //80F

    //Active Heating
    Hi = 101, // 17247 ohms, //101F
    Low = 98, //20649, //95F

    //Safety Max
    SafetyMaxTemperature = 108 // 13589 ohms //111F
    //TODO: List
    //Add summer winter temp
  };

  const long ACTION_INTERVAL = 1000;
  const long SAFETY_INTERVAL = 100;

  //Green Ground
  //Red 5V
  //White Analog
  const int HEATERPIN = 2;
  const int SAFETYPIN = 3;
  const int SLEEPSWITCH = 7;  
 }