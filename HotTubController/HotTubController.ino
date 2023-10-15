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
  //Consider: As we are using an EMA for temperature measurements means we can likely
  // narrow the band of active heating to 101-98

  //Saftey Max, this can be lowered to 108
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
const long ACTION_INTERVAL = 1000000;  // using 1000000 for micros() overflow test, instead of 1000

unsigned long _previousRunTime = 0;
unsigned long _previousRunCycles = 0;

float _emaResistance = 0.0f;
float _alpha = 0.0001f; 
// This is the smoothing factor for our Exponential Moving Average (ema) formula. This
// A higher alpha value will result in a smoother EMA, but it will also be less 
// responsive to changes in the measured resistance.

float _emaSafteyResistance = 0.0f;
float _alphaSaftey = 0.0004f; 
// currently we are taking about 4.5 samples per millisecond (6.5 when grounded) from the thermistor 
// alpha of 0.001f responds from a full open to full closed in about 3 seconds,
// where as an alpha of 0.0001f takes about 30 seconds for a similar response. 
String _fileCompiledInfo;

bool _isSleep = false;
bool _deadManSwitch = true;
HeatingMode _heatingStatus = NEITHER;

// enable soft reset
void(* resetFunc) (void) = 0;

// Initilzation
void setup(void) {
  // Check serial rates at: https://wormfood.net/avrbaudcalc.php
  // Uno typically has a 16Mhz crystal, could use conditional compilation arguments here to optimize for specific boards.
  Serial.begin(250000);  //Serial.begin(9600); 
  pinMode(HEATERPIN, OUTPUT);
  pinMode(SAFTEYPIN, OUTPUT);
  pinMode(SLEEPSWITCH, INPUT_PULLUP);
  _fileCompiledInfo = outFileCompiledInfo();
}

//Exectuion Loop
void loop(void) {
  //Allways execute
  //TODO: Add checks against DeadMansSwitch
  float reading = analogRead(THERMISTORPIN);
  //TODO: #3 handle common Thermistor Read errors 
  //  - reading > 1018 :A a detached thermistor, but SERIESRESISTOR is still in place
  //  - reading < 7 : thermistor is grounded or A0 is detached
  //  - readings that frequently cycle from 0 through 1023 (oddly around 60 hz durring testing) 
  //    occures when A0 is connected, but not powered or grounded.
  float measuredResistance = CalculateResistance(reading);

   // Calculate the EMA of the measured resistance.
  _emaResistance = CalculateExponentialMovingAverage(_alpha,_emaResistance, measuredResistance);
  // _emaSafteyResistance is much more responsive than _emaResistance
  _emaSafteyResistance = CalculateExponentialMovingAverage(_alphaSaftey,_emaSafteyResistance, measuredResistance);
 
  SafteyCheck(_emaSafteyResistance);
  //unsigned long currentRunTime = millis(); 
  
  //for testing time overflows by using micros() as it oveflows in 70 minutes   
  unsigned long currentRunTime = micros();
  
  // The number of milliseconds since board's last reset
  // Unsigned Long is 32 bit and overflows after 4,294,967,295  (2^32-1)
  // millis overflows ever 49.8 days
  // an unsigned negitive value is a positive value
  if ((unsigned long)(currentRunTime - _previousRunTime) > (ACTION_INTERVAL-1)) {
    // only take actions if ACTION_INTERVAL has passed
    if (_deadManSwitch = false) {
      float targetHi;
      float targetLow;
      OutGetTargetTemp(targetHi, targetLow);
      SetHeatingStatus(targetHi, targetLow);
      SetHeater();
    }
    outReport(currentRunTime,measuredResistance);
    // Reset previous run variables
    _previousRunCycles = 0;
    _previousRunTime = currentRunTime ;
  } else {
    //execute only on non-action loop
    _previousRunCycles++;
    if (_previousRunCycles > (unsigned long) (pow(2,8*sizeof(_previousRunCycles))-2)){
      //if _previousRunCycles is about to overflow then reset board
      Serial.println("Rebooting prior to _previousRunCycles overflow");
      resetFunc();
    }
  }
}

String outFileCompiledInfo() {
  String FileInfo = (__FILE__); // filename
  FileInfo.concat("_");
  FileInfo.concat(__DATE__); // date file compiled
  FileInfo.concat("_");
  FileInfo.concat(__TIME__);  
  return FileInfo ;
}

void outReport(unsigned long runTime, float resistance){
    
    // Build Log Message as json-logs:https://signoz.io/blog/json-logs/
    // e.g.->  {"RunTime":1235,} Longs and ints don't need to be quoted...
    // Strings are a pain and memory hogs, so just don't use them if we don't need them
    Serial.print("{");
    Serial.print("\"RunTimeStamp\":");Serial.print((unsigned long)(runTime));
    Serial.print("\"PreviousRunCompletionTime\":");Serial.print((unsigned long)(_previousRunTime));
    // attributes that can be calculated from other attributes shouldn't be writtent to log 
    //XXXX Serial.print(",\"RunDurration\":\"");Serial.print((unsigned long)(currentRunTime - _previousRunTime));
    //XXXX Serial.print(",\"A0_Reading\":");Serial.print((unsigned long)(reading));
    Serial.print(",\"MeasuredResistance\":\"");Serial.print(resistance);Serial.print("\"");
    Serial.print(",\"EmaResistance\":\"");Serial.print(_emaResistance);Serial.print("\"");
    Serial.print(",\"EmaSafteyResistance\":\"");Serial.print(_emaSafteyResistance);Serial.print("\"");
    Serial.print(",\"RunCycles\":");Serial.print(_previousRunCycles);
    Serial.print(",\"FileCompiledInfo\":\"");Serial.print(_fileCompiledInfo);Serial.print("\"");
    Serial.println("}");

    // TODO: Add write out to log (sd card or wifi ftp)

}

void SafteyCheck(float measuredResistance) {
  if (measuredResistance < Saftey)
  {
    ThrowDeadMansSwitch();
  }
}

float CalculateExponentialMovingAverage(float alpha, float currentEma, float value) {
  float ema = (1.0f - alpha) * currentEma + alpha * value;
  return ema;
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

void SetHeatingStatus(float targetHi, float targetLow) {
  switch (_heatingStatus) {
    case NEITHER:
    case HEATING:
      if (_emaResistance < targetHi)
        _heatingStatus = COOLING;
      break;
    case COOLING:
      if (_emaResistance > targetLow)
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
