#ifndef Serial
  #include <Arduino.h>
#endif
#include <math.h>
#ifndef SERIESRESISTOR 
    #include "../../../include/config.h"
#endif
#ifndef outArbitraryReport
    #include "../../../lib/NaiveLogger/src/naiveLogger.cpp"
#endif
//Model Name: Formula to Calculate Resistance
//power function curve: 55880.76 * (degree_f / 52) ** (-1.66)
//LogestModel: 207518.98*(0.9766636^degree_f)

//Model Name: Formula to Calculate Temperature
//power function curve: 52 * (resistance / 55880.76) ** (1 / -1.66)
//LogestModel: -42.3495 log(4.81884×10^-6 (resistance + 3000)) 

float degree_f_from_resistance(float resistance) {
  if (resistance > 279447) {
    return 15.0f;
  } else if (resistance < 9200) {
    return 131.0f;
  } else {
    // PowerFunctionModel =  52.0f * powf(resistance / 55880.76f, 1.0f / -1.66f);
    // LogestModel = -42.3495 * (log(4.81884*powf(10,-6)*(resistance+3000)));
    // Result = (0.92*PowerFunctionModel+1.08*LogestModel)/2
    float PowerFunctionModel = 52.0f * powf(resistance / 55880.76f, 1.0f / -1.66f);
    float LogestModel = -42.3495 * (log(4.81884*powf(10,-6)*(resistance+3000)));
    // finish outArbitraryReport and call that instead
    #if (REPORTINGLEVEL !=0)
      #if (DEBUGENABLED) // not ready to implement yet 
        outArbitraryReport("DEBUGMESSAGE:TEST");
        Serial.print("{DEBUGMESSAGE:");
        Serial.print("PowerFunctionModel="); Serial.print(PowerFunctionModel);
        Serial.print(";    LogestModel="); Serial.print(LogestModel);
        Serial.print(";    EnsembelModel="); Serial.print((0.92*PowerFunctionModel+1.08*LogestModel)/2);
        // {BLAME} next line (simplified formula) doesn't isn't returning the correct values...
        Serial.print(";    AlternateAttemptEnsembelModel="); Serial.print(
          ((0.92*(55880.76f * powf(resistance / 52.0f , -1.66f)))+
          1.08*(207518.98* powf(0.9766636,resistance) - 3000))/2
        );
        Serial.println("}");
        // {BLAME} next line (simplified formula) doesn't isn't returning the correct values...
        //return (17320.4/powf(resistance,0.60241)) - (22.8093 * log(resistance + 3000) + 279.254);
      #endif
    #endif
    return (PowerFunctionModel+LogestModel)/2;
  }
}

void CalculateExponentialMovingAverage(float alpha, float& Ema, float value) {
  Ema = (1.0f - alpha) * Ema +  alpha * value;
}

float CalculateResistance(float readingHeater, float resistorInSeries ) {
  // setting a cap on readingHeater of 1022 instead of 1023 to avoids having to handle an INF and OVF errors with our math  
  float measuredTemperature = 0.0f;
  if ((readingHeater) < 1022.0f ) {
    measuredTemperature = (resistorInSeries + 0.0f) / (float)((1023.001 / readingHeater)  - 1.0); // 10K / ((1023/ADC) - 1) 
  } else {
    measuredTemperature = (resistorInSeries + 0.0f) / (float)((1023.001 / 1022.0)  - 1.0); // 10K / ((1023/ADC) - 1) 
  }
  return measuredTemperature;
}
