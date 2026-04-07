#ifndef COOLER_HEADER
#define COOLER_HEADER

#include <cstdint>
#include <Adafruit_MCP9808.h>
#include "DigitalOutput.hpp"

class Cooler {
public:
    Cooler(uint8_t heatSensorI2CAddress, uint8_t coolerPin, uint8_t activationTemp, uint8_t desactivationTemp);

    void begin();

    void handle();

private:
    uint8_t heatSensorI2CAddress_m;
    DigitalOutput coolerPin_m;

    uint8_t activationTempCelsius_m;
    uint8_t desactivationTempCelsius_m;

    Adafruit_MCP9808 heatSensor_m;

    bool isOn_m{ false };
};

#endif // !COOLER_HEADER