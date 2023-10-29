# BridgeFour
Various Arduino Projects

# Hot tub controller
Current state is a minimal viable solution for controlling a heater to maintain temperature with thermistors as input.

## Installation Tips
- *This is a generic Arduino board tip:*  for some arduino boards on windows I had to install FDI drivers from https://ftdichip.com/drivers/vcp-drivers/ in order to communicate over COM ports.

## Temperature Formulas
Model Chart of Temperature from Resistance Values for Balboa, Sundance, or Bret Aqualine Spa Temperature Sensors
![Model Chart of Temperature from Resistance Values for Balboa Sundance Bret Aqualine Spa Temp Sensors](/documentation/resources/ModelChartOfTempfromResistanceValuesforBalboaSundanceBretAqualineSpaTempSensors.png)

Residuals of Temperature Models
![ResidualsOfTemperatureModels.png](/documentation/resources/ResidualsOfTemperatureModels.png)

### Derived models: Formula to Calculate Resistance
- power function curve: <br/>
   $`resistance = 55880.76\cdot\left(degree_f\over52\right)^{-1.66}`$ 
- LogestModel: <br/> $` 207518.98\cdot\left(0.9766636^{degree_f}\right) -3000  `$
- Ensemble Weighted Average: <br/>
$`\left(\left(0.92 \cdot 55880.76\cdot\left(degree_f\over52\right)^{-1.66} \right) + \left(1.08 \cdot 207518.98\cdot\left(0.9766636^{degree_f}\right)-3000 \right) \right) \over 2`$
- Ensemble Weighted Average Simplified: <br/>
$` 112060 \cdot  0.976664^{x} + \left((1.81378×10^{7}) \over x^{1.66}\right) - 3000 `$
### Derived models: Formula to Calculate Temperature
- power function curve:<br/> 
 $` 52 \cdot \left(resistance \over 55880.76 \right) ^ {1 \over -1.66} `$
- LogestModel: <br/>
$`-42.3495 \cdot log(4.81884×10^{-6} \cdot (resistance + 3000)) `$
- Ensemble Weighted Average: <br/> $`\left(\left(0.92 \cdot       52 \cdot \left(resistance \over 55880.76 \right) ^ {1 \over -1.66} \right) + \left(1.08 \cdot -42.3495 \cdot log(4.81884×10^{-6} \cdot (resistance + 3000)) \right) \right) \over 2`$

- Ensemble Weighted Average Simplified: <br/>
$` \left(17320.4 \over resistance^{0.60241} \right) -\left(22.8093 \cdot log(resistance+3000) + 279.254 \right)`$