# BridgeFour
Various Arduino Projects

# Hot tub controller
Current state is a minimal viable solution for controlling a heater to maintain temperature with thermistors as input.

## Temperature Formulas

Model Chart of Temperature from Resistance Values for Balboa, Sundance, or Bret Aqualine Spa Temperature Sensors
![Model Chart of Temperature from Resistance Values for Balboa Sundance Bret Aqualine Spa Temp Sensors](/documentation/resources/ModelChartOfTempfromResistanceValuesforBalboaSundanceBretAqualineSpaTempSensors.png)

Residuals of Temperature Models
![ResidualsOfTemperatureModels.png](/documentation/resources/ResidualsOfTemperatureModels.png)

### Derived models: Formula to Calculate Resistance
- power function curve: <br/>
   $`resistance = 55880.76 \left(degree_f \over 52 \right)^-1.66`$ 
- LogestModel: 207518.98*(0.9766636^degree_f)
- Ensemble Weighted Average: ((0.92*55880.76*($degree_f / 52)^(-1.66))+(1.08*207518.98*(0.9766636^degree_f)))/2
- Ensemble Weighted Average Simplified: 112060 * 0.976664^x + (1.81378×10^7)/x^1.66

### Derived models: Formula to Calculate Temperature
- //Model Name: Formula to Calculate Temperature
//power function curve: 52 * (resistance / 55880.76) ** (1 / -1.66)
//LogestModel: -42.3495 log(4.81884×10^-6 (resistance + 3000)) 