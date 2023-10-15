#define SERIESRESISTOR 12700
#define THERMISTORPIN A0

//Green Ground
//Red 5V
//White Analog

enum TargetTempature {
  //Sleep
  SleepHi = 24832,//85F
  SleepLow = 27931, //80F

  //Active Heating
  Hi = 17247, //101F
  Low = 20649, //95F

  //Saftey Max
  Saftey = 13589 //111
};

enum HeatingMode {
  NEITHER,
  HEATING,
  COOLING
};

//TODO: List
//Add summer winter temp
const int HEATERPIN = 2;
const int SAFTEYPIN = 3;
const int SLEEPSWITCH = 7;
const long ACTION_INTERVAL = 1000;
unsigned long _previousRunTime = 0;
unsigned long _previousRunCycles = 0;
bool _isSleep = false;
bool _deadManSwitch = true;
HeatingMode _heatingStatus = NEITHER;

void setup(void) {
  // Check your searial rates at: https://wormfood.net/avrbaudcalc.php
  // Uno typically has a 16Mhz crystal
  Serial.begin(250000);  //Serial.begin(9600); 
  pinMode(HEATERPIN, OUTPUT);
  pinMode(SAFTEYPIN, OUTPUT);
  pinMode(SLEEPSWITCH, INPUT_PULLUP);
}

void loop(void) {
  //Allways execute
  //TODO: Add checks against DeadMansSwitch
  float reading = analogRead(THERMISTORPIN);
  //TODO: #3 handle common Thermistor Read errors 
  //  - reading > 1018 : a detached thermistor, but SERIESRESISTOR is still in place
  //  - reading < 7 : thermistor is grounded or A0 is detached
  //  - readings that frequently cycle from 0 through 1023 (oddly around 60 hz durring testing) 
  //    occures when A0 is connected, but not powered or grounded.
  float measuredResistance = CalculateResistance(reading);
  SafteyCheck(measuredResistance);

  //TODO: #2 Build an average of measuredResistance since _previousRunTime
  //      Consider refactoring from a sample every second, to a FLIT filtered measurement, or and exponential moving average
  //      so the only thing in the action part of loop would be reading user input and writting logs.
  //      https://electronics.stackexchange.com/questions/30370/fast-and-memory-efficient-moving-average-calculation

  unsigned long currentRunTime = millis(); 
  // The number of milliseconds since board's last reset
  // Unsigned Long can not exceed 4,294,967,295 

  if ((currentRunTime - _previousRunTime) > (ACTION_INTERVAL-1)) {
    // only take actions if ACTION_INTERVAL has passed
    if (_deadManSwitch = false) {
      float targetHi;
      float targetLow;
      OutGetTargetTemp(targetHi, targetLow);
      SetHeatingStatus(measuredResistance, targetHi, targetLow);
      SetHeater();
    }
    
    // Build Log Message as json-logs:https://signoz.io/blog/json-logs/
    // e.g.->  {"RunTime":1235,}
    String logOuput = "{";
    logOuput.concat("\"BootTime\":");
    logOuput.concat(currentRunTime);
    logOuput.concat(",\"A0_Reading\":");
    logOuput.concat((unsigned long)(reading));
    logOuput.concat(",\"Measured Resistance\":\"");
    logOuput.concat(measuredResistance);
    logOuput.concat("\",\"RunDurration\":");
    logOuput.concat(currentRunTime - _previousRunTime);
    logOuput.concat(",\"RunCycles\":");
    logOuput.concat( _previousRunCycles);
    logOuput.concat("}");

    // Out to log (sd card or wifi ftp + serial monitor?)
    Serial.println(logOuput);

    // Reset previous run variables
    _previousRunCycles = 0;
    _previousRunTime = currentRunTime ;
  } else {
    //execute only on non-action loop
    _previousRunCycles++;
  }
}

void SafteyCheck(float measuredResistance) {
  if (measuredResistance < Saftey)
  {
    ThrowDeadMansSwitch();
  }
}

float CalculateResistance(float reading) {
  float measuredResistance = SERIESRESISTOR / (float)((1023.0001 / reading)  - 1); // 10K / (1023/ADC - 1) 
  // using 1023.0001 instead of 1023 in the formula has no effect on typical values, and avoids having to handle an INF
  return measuredResistance;
}

void OutGetTargetTemp(float &targetHi, float &targetLow) {
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

void ThrowDeadMansSwitch() {
  _deadManSwitch = false;
  digitalWrite(SAFTEYPIN, _deadManSwitch);
  TurnOffHeater();
}

void SetHeatingStatus(float measuredResistance, float targetHi, float targetLow) {
  switch (_heatingStatus) {
    case NEITHER:
    case HEATING:
      if (measuredResistance < targetHi)
        _heatingStatus = COOLING;
      break;
    case COOLING:
      if (measuredResistance > targetLow)
        _heatingStatus = HEATING;
      break;
  }
}

void SetHeater() {  
  switch (_heatingStatus) {
    case COOLING:
      TurnOffHeater();
      break;
    case HEATING:
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
