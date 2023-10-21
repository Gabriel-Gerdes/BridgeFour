
// Global 
namespace Report{
    enum ReportMessage {
        MsgRoutine,
        MsgErrorDeadMan,
        MsgErrorRebootPriorToOverflow,
        MsgAdHocReport
    };
}
namespace HeatControl {
    enum HeatingMode {
        NEITHER,
        HEATING,
        COOLING
    };
}
// Global variables
// by convention every variable begining with an underscore in this project is a global varaible.
// really need to refactor these from external global variables to some other method of 
// passing this information arround, could use pointers? 

unsigned long _previousRunTime ;
unsigned long _previousRunCycles ;

bool _isSleep;
bool _deadManSwitch;
HeatControl::HeatingMode _heatingStatus = HeatControl::NEITHER;
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
