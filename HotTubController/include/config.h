// Our project configuration items are found here

#define DEBUGENABLED true
#define REPORTINGLEVEL 1
//currently reporting levels are:
// 0 = no reporting
// 1 = typical reporting (Durring action)
// 2 = frequent reporting (Durring safety check)
// 3 = continious reporting (Every loop)

const float _alpha = 0.005f; 
const float _alphaSafety = 0.01f; 
// Alpha is the smoothing factor for our Exponential Moving Average (ema) formula.
// EMA smooths our measured temp value to remove any noise from the signal.
// A higher alpha value will result in a smoother EMA, but it will also be less 
// responsive to changes in the measured resistance.
// currently we are calculating about 1 sample per millisecond (6.5 when grounded on one thermistor) from the thermistor 
// as we add complexity to the sketch we will be sampling at a slower rate, 
// so it's important to continue to increase our alpha to maintain responsiveness, and test.
// [TODO] it should be possible to adjust the alpha so that it's responsive to some  time period

//These are just the defaults, eventually we may want to override these with a UI.
enum TargetTempature {
  //Sleep
  SleepHi = 85, // 24832 ohms  ,//85F
  SleepLow = 80, // 27931 ohms, //80F

  //Active Heating
  Hi = 101, // 17247 ohms, //101F
  Low = 98, //20649, //95F

  //Safety Max
  Safety = 108 // 13589 ohms //111F
  //TODO: List
  //Add summer winter temp
};

const long ACTION_INTERVAL = 1000;
const long SAFETY_INTERVAL = 100;

//pins 
#define SERIESRESISTOR 12700 // installed resistor in parallel with the temp thermistor
#define THERMISTORPINPREHEATER A0
#define THERMISTORPINPOSTHEATER A2
//Green Ground
//Red 5V
//White Analog
const int HEATERPIN = 2;
const int SAFETYPIN = 3;
const int SLEEPSWITCH = 7;
