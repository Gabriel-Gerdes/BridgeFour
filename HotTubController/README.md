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
   $`resistance = 55880.76\cdot\left(degree_f\over52\right)^{-1.66}`$ 
- LogestModel: <br/> $`207518.98\cdot\left(0.9766636^{degree_f}\right)`$
- Ensemble Weighted Average: <br/>
$`\left(\left(0.92 \cdot 55880.76\cdot\left(degree_f\over52\right)^{-1.66} \right) + \left(1.08 \cdot 207518.98\cdot\left(0.9766636^{degree_f}\right) \right) \right) \over 2`$
- Ensemble Weighted Average Simplified: 112060 * 0.976664^x + (1.81378×10^7)/x^1.66

### Derived models: Formula to Calculate Temperature
- //Model Name: Formula to Calculate Temperature
//power function curve: 52 * (resistance / 55880.76) ** (1 / -1.66)
//LogestModel: -42.3495 log(4.81884×10^-6 (resistance + 3000)) 
