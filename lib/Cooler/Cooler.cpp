#include "Cooler.hpp"

Cooler::Cooler(uint8_t heatSensorI2CAddress, uint8_t coolerPin, uint8_t activationTempCelsius, uint8_t desactivationTempCelsius) 
: heatSensorI2CAddress_m{ heatSensorI2CAddress }
, coolerPin_m{ coolerPin }
, activationTempCelsius_m{ activationTempCelsius }
, desactivationTempCelsius_m{ desactivationTempCelsius }
{
}

// TODO: Agregar manejo de error
void Cooler::begin() {
    heatSensor_m.begin(heatSensorI2CAddress_m);
    coolerPin_m.begin();
}

void Cooler::handle() {
    const auto temperature{ heatSensor_m.readTempC() };

    if (!isOn_m && (temperature >= activationTempCelsius_m)) {
        coolerPin_m.write(true);
        isOn_m = true;
    }
    else if (isOn_m && (temperature <= desactivationTempCelsius_m)) {
        coolerPin_m.write(false);
        isOn_m = false;
    }
}
