#ifndef Serial
  #include <Arduino.h>
#endif
#ifndef digitalReadFast
  #include <digitalWriteFast.h>
#endif
#ifndef SleepHi
  #include "../../../include/config.h"
#endif
namespace naiveLogger {
  // ----------------------------------------------------------------
  // Prototyping all functions
  // ----------------------------------------------------------------
  #if (DEBUGENABLED) // not ready to implement yet 
    #define macroOutArbitrary(...) outArbitraryReport(__VA_ARGS__, NULL)
  #endif

  char * _board_id;

  enum ReportMessage {
      MsgRoutine,
      MsgErrorDeadMan,
      MsgErrorRebootPriorToOverflow,
      MsgAdHocReport
  };

  void outReportSuffix();

  bool outReportPrefix(unsigned long previousRunCycles, ReportMessage customStatusMessage);

  void outReport(
    ReportMessage customStatusMessage,
    float resistancePre, 
    float resistancePost,
    float temperaturePreHeater,
    float temperaturePostHeater,
    unsigned long previousRunCycles,
    long previousRunTime,
    float emaTemperaturePreHeater,
    float emaTemperaturePostHeater,
    float emaSafetyTemperaturePreHeater,
    float emaSafetyTemperaturePostHeater,
    unsigned int _heatingStatusRequest
  ) ;

  // ----------------------------------------------------------------
  // Defining Functions
  // ----------------------------------------------------------------

  void outReport(
    naiveLogger::ReportMessage customStatusMessage,
    float resistancePre, 
    float resistancePost,
    float temperaturePreHeater,
    float temperaturePostHeater,
    unsigned long previousRunCycles,
    long previousRunTime,
    float emaTemperaturePreHeater,
    float emaTemperaturePostHeater,
    float emaSafetyTemperaturePreHeater,
    float emaSafetyTemperaturePostHeater,
    unsigned int _heatingStatusRequest
  ){
    if (outReportPrefix(previousRunCycles, customStatusMessage)) {
        // If outReportSuffix is false then we don't write any of these attributes.
        Serial.print(",\"LastRunTime\":");Serial.print((long)(previousRunTime));
        // attributes that can be calculated from other attributes shouldn't be writtent to log 
        //XXXX Serial.print(",\"RunDurration\":\"");Serial.print((unsigned long)(currentRunTime - _previousRunTime));
        //XXXX Serial.print(",\"A0_readingPreHeater\":");Serial.print((unsigned long)(readingPreHeater));
        Serial.print(",\"PreHeatOhms\":\"");Serial.print(resistancePre);Serial.print("\"");
        Serial.print(",\"PostHeatOhms\":\"");Serial.print(resistancePost);Serial.print("\"");
        Serial.print(",\"PreHeatEmaTemp\":\"");Serial.print(emaTemperaturePreHeater);Serial.print("\"");
        Serial.print(",\"PostHeatTemp\":\"");Serial.print(emaTemperaturePostHeater);Serial.print("\"");
        Serial.print(",\"PreSafetyHeatEmaTemp\":\"");Serial.print(emaSafetyTemperaturePreHeater);Serial.print("\"");
        Serial.print(",\"PostSafetyHeatEmaTemp\":\"");Serial.print(emaSafetyTemperaturePostHeater);Serial.print("\"");
        Serial.print(",\"HeatingStatusRequest\":\"");Serial.print(_heatingStatusRequest);Serial.print("\"");
        
        // TODO: Add write out to log (sd card or wifi ftp, define at compile time with conditional compilation arguement? )
    }
    outReportSuffix();
  }
  #if (DEBUGENABLED) // not ready to implement yet 
    void outArbitraryReport(const char *arg, ...);
    void outArbitraryReport(const char *arg, ...){
      // [TODO] to handle any type of value passed review https://stackoverflow.com/a/57054219

      // macro macroOutArbitrary should be adding a null as the last argument to any call to this function...
      // This expects that every other character array passed is the json attribute name, the next is the attribute value. 
      // Using this method to allow for many parameters to the function: https://stackoverflow.com/a/9040731
      // ... supports up to 126 parameters (C compilers aren't required to support more than 127 parameters)  
      Serial.print("{");
      Serial.print("\"BoardId\":\"");Serial.print(_board_id);Serial.print("\"");
      Serial.print(",\"RunCycles\":");Serial.print(_previousRunCycles);
      Serial.print(",\"Running\" : ");Serial.print((unsigned long)(millis()));
      Serial.print(",\"Status\":");Serial.print("\"AdHoc\"");

      va_list arguments;

        for (va_start(arguments, arg); arg != NULL; arg = va_arg(arguments, const char *)) {
            //cout << arg << endl;
            Serial.print(arg);
        }

        va_end(arguments);


        Serial.println("}");
      // TODO: Add write out to log (sd card or wifi ftp, define at compile time with conditional compilation arguement?)
    }
  #endif

  // ----------------------------------------------------------------
  // Logging/Reporting Functions
  // ----------------------------------------------------------------
  // Build Log Message as json-logs:https://signoz.io/blog/json-logs/
  // e.g.->  {"RunTime":1235,} Longs and ints don't need to be quoted...
  // Strings are a pain and memory hogs, so just don't use them if we don't need them
  // json begining, and all the values that we allways log...
  bool outReportPrefix(unsigned long PreviousRunCycles, naiveLogger::ReportMessage customStatusMessage){
      Serial.print("{");
      Serial.print("\"BoardId\":\"");Serial.print(_board_id);Serial.print("\"");
      Serial.print(",\"RunCycles\":");Serial.print(PreviousRunCycles);
      Serial.print(",\"Time\" : ");Serial.print((unsigned long)(millis()));
      Serial.print(",\"HeaterPin\" : ");Serial.print(digitalReadFast(Config::HEATERPIN));
      Serial.print(",\"SleepSwitch\" : ");Serial.print(digitalReadFast(Config::SLEEPSWITCH));
      Serial.print(",\"SafetyPin\" : ");Serial.print(digitalReadFast(Config::SAFETYPIN));
      switch (customStatusMessage) {
        case naiveLogger::ReportMessage::MsgRoutine:
          Serial.print(",\"Status\":");Serial.print("\"Routine\"");
          return true;
        case naiveLogger::ReportMessage::MsgErrorDeadMan:
          Serial.print(",\"Status\":");Serial.print("\"Error:Deadman Switch Thrown\"");
          return false;
        case naiveLogger::ReportMessage::MsgErrorRebootPriorToOverflow:
          Serial.print(",\"Status\":");Serial.print("\"Error:Rebooting prior to unexpected overflow\"");
          return false;
        case naiveLogger::ReportMessage::MsgAdHocReport:
          Serial.print(",\"Status\":");Serial.print("\"Error:Rebooting prior to unexpected overflow\"");
          return true;
        default:
          return false;
      };
  }

  // close json
  void outReportSuffix(){
    Serial.println("}");
  }

  // get board id information
  void outBoard_Id() {
    // Compiled date, time, and board concatinated for a board ID...
    _board_id = (char *) malloc(0);
    _board_id[0]='\0';
    strcat(_board_id, __DATE__);
    strcat(_board_id, "_");
    strcat(_board_id, __TIME__);
    strcat(_board_id, "_");
    
    // each board type can be defined by the compile flags on build.
    // https://community.platformio.org/t/can-i-check-the-board-platform-from-the-code/20353/
    #ifdef ARDUINO_AVR_MEGA2560
      strcat(_board_id, (char *)"MEGA2560");
    #elif ARDUINO_AVR_UNO
      strcat(_board_id,"UNO");
    #elif ARDUINO_AVR_NANO
      strcat(_board_id, "NANO");
    #elif ARDUINO_AVR_MICRO
      strcat(_board_id,"MICRO");
    #else
      strcat(_board_id,"UNKNOWN");
    #endif
  }
}