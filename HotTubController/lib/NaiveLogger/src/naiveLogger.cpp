#ifndef Serial
  #include <Arduino.h>
#endif
#ifndef _previousRunCycles
  #include "../../../include/skeleton.h"  
#endif

#define macroOutArbitrary(...) outArbitraryReport(__VA_ARGS__, NULL)

static String _fileCompiledInfo; // static to make it a variabl scoped to this .cpp file

// ----------------------------------------------------------------
// Logging/Reporting Functions
// ----------------------------------------------------------------
// Build Log Message as json-logs:https://signoz.io/blog/json-logs/
// e.g.->  {"RunTime":1235,} Longs and ints don't need to be quoted...
// Strings are a pain and memory hogs, so just don't use them if we don't need them
String outFileCompiledInfo() {
  String FileInfo = (__FILE__); // filename
  FileInfo.concat("_");
  FileInfo.concat(__DATE__); // date file compiled
  FileInfo.concat("_");
  FileInfo.concat(__TIME__);  
  return FileInfo ;
}

// json begining, and all the values that we allways log...
bool outReportPrefix(unsigned int customStatusMessage){
    Serial.print("{");
    Serial.print("\"BoardId\":\"");Serial.print(_fileCompiledInfo);Serial.print("\"");
    Serial.print(",\"RunCycles\":");Serial.print(_previousRunCycles);
    Serial.print(",\"Time\" : ");Serial.print((unsigned long)(millis()));

    // we shouldn't have to declare this enum twice... should we? it's in the main file and here...
    enum ReportMessage {
        MsgRoutine,
        MsgErrorDeadMan,
        MsgErrorRebootPriorToOverflow,
        MsgAdHocReport
    };

    switch (customStatusMessage) {
      case MsgRoutine:
        Serial.print(",\"Status\":");Serial.print("\"Routine\"");
        return true;
      case MsgErrorDeadMan:
        Serial.print(",\"Status\":");Serial.print("\"Error:Deadman Switch Thrown\"");
        return false;
      case MsgErrorRebootPriorToOverflow:
        Serial.print(",\"Status\":");Serial.print("\"Error:Rebooting prior to unexpected overflow\"");
        return false;
      case MsgAdHocReport:
        Serial.print(",\"Status\":");Serial.print("\"Error:Rebooting prior to unexpected overflow\"");
        return true;
    }
}

// close json
void outReportSuffix(){
  Serial.println("}");
}

void outReport(
  unsigned int customStatusMessage,
  float resistancePre, 
  float resistancePost,
  float temperaturePreHeater,
  float temperaturePostHeater 
){
  if (outReportPrefix(customStatusMessage)) {
      // If outReportSuffix is false then we don't write any of these attributes.
      Serial.print(",\"LastRunTime\":");Serial.print((unsigned long)(_previousRunTime));
      // attributes that can be calculated from other attributes shouldn't be writtent to log 
      //XXXX Serial.print(",\"RunDurration\":\"");Serial.print((unsigned long)(currentRunTime - _previousRunTime));
      //XXXX Serial.print(",\"A0_readingPreHeater\":");Serial.print((unsigned long)(readingPreHeater));
      Serial.print(",\"PreHeatOhms\":\"");Serial.print(resistancePre);Serial.print("\"");
      Serial.print(",\"PostHeatOhms\":\"");Serial.print(resistancePost);Serial.print("\"");
      Serial.print(",\"PreHeatEmaTemp\":\"");Serial.print(_emaTemperaturePreHeater);Serial.print("\"");
      Serial.print(",\"PostHeatTemp\":\"");Serial.print(_emaTemperaturePostHeater);Serial.print("\"");
      Serial.print(",\"PreSafetyHeatEmaTemp\":\"");Serial.print(_emaSafetyTemperaturePreHeater);Serial.print("\"");
      Serial.print(",\"PostSafetyHeatEmaTemp\":\"");Serial.print(_emaSafetyTemperaturePostHeater);Serial.print("\"");
      Serial.println("}");
      // TODO: Add write out to log (sd card or wifi ftp, define at compile time with conditional compilation arguement? )
  }
  outReportSuffix();
}

void outArbitraryReport(const char *arg, ...){
  // [TODO] to handle any type of value passed review https://stackoverflow.com/a/57054219

  // macro macroOutArbitrary should be adding a null as the last argument to any call to this function...
  // This expects that every other character array passed is the json attribute name, the next is the attribute value. 
  // Using this method to allow for many parameters to the function: https://stackoverflow.com/a/9040731
  // ... supports up to 126 parameters (C compilers aren't required to support more than 127 parameters)  
  Serial.print("{");
  Serial.print("\"BoardId\":\"");Serial.print(_fileCompiledInfo);Serial.print("\"");
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