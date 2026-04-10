#ifndef COOLER_HEADER
#define COOLER_HEADER

#include <cstdint>
#include <Adafruit_MCP9808.h>
#include "DigitalOutput.hpp"

/// @brief Clase encargada de la regulación de temperatura
class Cooler {
public:

    /// @brief Constructor base
    /// @param heatSensorI2CAddress Dirección I2C del sensor de temperatura 
    /// @param coolerPin Pin para la activación del disipador de calor
    /// @param activationTemp Temperatura de activación del disipador. En celsius
    /// @param desactivationTemp Temperatura de desactivación del disipador. En celsius
    Cooler(uint8_t heatSensorI2CAddress, uint8_t coolerPin, uint8_t activationTemp, uint8_t desactivationTemp);

    /// @brief Inicializa el sensor de calor y el pin de activación, usar antes de nada
    void begin();

    /// @brief Maneja la activación y desactivación del disipador, llamar en cada bucle
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