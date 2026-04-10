#ifndef EASY_LORA_FIRMWARE_HEADER
#define EASY_LORA_FIRMWARE_HEADER

// <>

#include <Arduino.h>
#include <vector>
#include <pb_encode.h>
#include "MessageDecoder.hpp"
#include <array>
#include "E22_400T37S_Configurator.hpp"
#include "MessageEncoder.hpp"
#include <optional>
#include <string>
#include <cstdint>
#include "EnvelopeFactory.hpp"
#include "Cooler.hpp"

/*
    TODO:
    - Ahora los errores son irrecuperables, esto piensa cambiarse   
    
    - Los errores relacionados con mensajes no bien estructurados o mal decodificados deben ser ignorados A MENOS que sean críticos como en los casos de
        solicitar configuraciones o esperar ACK

    - Solucionar el error de problemas de decodificación al momento de enviar muchos paquetes
*/

/// @brief Firmware del dispositivo EasyLoRa
class EasyLoRa_Firmware {
public:

    /// @brief Constructor base
    /// @param serialUSB Puerto en donde se enviarán y recibirán paquetes
    /// @param serialToLoRa Puerto en donde se comunicará el MCU con el chip EByte
    /// @param m0_Pin Pin a m0 del chip
    /// @param m1_Pin Pin a m1 del chip
    /// @param auxPin Pin a aux del chip
    /// @param heatSensorI2CAddress Dirección I2C al sensor de temperatura
    /// @param coolerPin Pin para activar el disipador de calor
    /// @param coolerActivationTempC Temperatura en Celsius para la activación de la disipación
    /// @param coolerDesactivationTempC Temperatura en Celsius para la desactivación de la disipación
    EasyLoRa_Firmware(arduino::HardwareSerial& serialUSB, arduino::HardwareSerial& serialToLoRa, uint8_t m0_Pin, uint8_t m1_Pin, 
        uint8_t auxPin, uint8_t heatSensorI2CAddress, uint8_t coolerPin, uint8_t coolerActivationTempC, uint8_t coolerDesactivationTempC);

    /// @brief Inicializa el dispositivo, debe ser llamado antes de cualquier otra función.
    void begin();

    /// @brief Inicia el dispositivo
    [[noreturn]]
    void start();
    
private:

    struct EnvelopeBundle;

    static constexpr uint16_t Reboot_Time_In_Ms{ 1000 };

    static constexpr std::array toValueUARTBaudRate{ 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 };

    SerialParser serialToLoRa_m;
    E22_400T37S_Configurator configurator_m;
    SerialParser serialToAPI_m;

    DigitalInput auxPin_m;

    Cooler cooler_m;

    ModuleConfiguration actualConfiguration_m = ModuleConfiguration_init_zero;

    void manageAPIEnvelope(const EnvelopeBundle& envelopeBundle);
    void manageLoRaEnvelope(const EnvelopeBundle& envelopeBundle);

    void applyConfiguration(const ModuleConfiguration& configuration);
    void sendConfigurationToAPI();
    void sendToSerial(SerialParser& serial, const std::vector<uint8_t>& data);

    void syncModuleConfiguration();

    [[nodiscard]]
    std::optional<EnvelopeBundle> receiveEnvelopeFromSerial(SerialParser& serial, bool isStrangePackageValid = false);

    [[noreturn]]
    void putIntoMalfunctionMode(std::string_view errorMessage);

    void trySendErrorToAPI(std::string_view errorMessage);

    void sendACK(SerialParser& serial);

    void syncConfigurationWithReceiver(const std::vector<uint8_t>& data, uint32_t timeout);
};

#endif // !EASY_LORA_FIRMWARE_HEADER
