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

bool _isSleep = false;
bool _deadManSwitch = true;
HeatingMode _heatingStatus = NEITHER;

void setup(void) {
  Serial.begin(9600);
  pinMode(HEATERPIN, OUTPUT);
  pinMode(SAFTEYPIN, OUTPUT);
  pinMode(SLEEPSWITCH, INPUT_PULLUP);
}


void loop(void) {

  float reading = analogRead(THERMISTORPIN);
  
  _isSleep = digitalRead(SLEEPSWITCH)?;
  
  float mesuredResistance = CalculateResistance(reading);

  //TODO: Add checks against DeadMansSwitch
  SafteyCheck(mesuredResistance);
  
  if(_deadManSwitch = false){
  
  float targetHi;
  float targetLow;
  OutGetTargetTemp(targetHi, targetLow);

  SetHeatingStatus(mesuredResistance, targetHi, targetLow);

  SetHeater();

}
    //Active
  delay(1000);
}

void SafteyCheck(float mesuredResistance) {
  if (mesuredResistance < Saftey)
  {
    ThrowDeadMansSwitch();
  }
}

float CalculateResistance(float reading) {
  // convert the value to resistance
  float mesuredResistance = SERIESRESISTOR / (float)((1023 / reading)  - 1); // 10K / (1023/ADC - 1)
  Serial.print("Mesured Resistance: ");
  Serial.println(mesuredResistance);
  return mesuredResistance;
}

void OutGetTargetTemp(float &targetHi, float &targetLow) {
  if (!_isSleep) {
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
  digitalWrite(SAFTEYPIN, _deadManSwith);
  TurnOffHeater();
}

void SetHeatingStatus(float mesuredResistance, float targetHi, float targetLow) {
  switch (_heatingStatus) {
    case NEITHER:
    case HEATING:
      if (mesuredResistance < targetHi)
        _heatingStatus = COOLING;
      break;
    case COOLING:
      if (mesuredResistance > targetLow)
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
