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
#include "StatusLED.hpp"
#include <optional>
#include <string>
#include "EnvelopeFactory.hpp"

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
    EasyLoRa_Firmware(arduino::HardwareSerial& serialUSB, arduino::HardwareSerial& serialToLoRa, uint8_t m0_Pin, uint8_t m1_Pin, uint8_t auxPin);

    /// @brief Inicializa el dispositivo, debe ser llamado antes de cualquier otra función.
    void begin();

    /// @brief Inicia el dispositivo
    [[noreturn]]
    void start();
    
private:

    struct EnvelopeBundle;

    static constexpr uint16_t Reboot_Time_In_Ms{ 1000 };

    SerialParser serialToLoRa_m;
    E22_400T37S_Configurator configurator_m;
    SerialParser serialToAPI_m;

    DigitalInput auxPin_m;

    StatusLED statusLED_m{ PIN_NEOPIXEL };

    ModuleConfiguration actualConfiguration_m = ModuleConfiguration_init_zero;

    void manageAPIEnvelope(const EnvelopeBundle& envelopeBundle);
    void manageLoRaEnvelope(const EnvelopeBundle& envelopeBundle);

    void applyConfiguration(const ModuleConfiguration& configuration);
    void sendConfigurationToAPI();
    void sendToSerial(SerialParser& serial, const std::vector<uint8_t>& data, StatusLED::Status errorIfFails);

    void syncModuleConfiguration();

    [[nodiscard]]
    std::optional<EnvelopeBundle> receiveEnvelopeFromSerial(SerialParser& serial, bool isStrangePackageValid = false);

    [[noreturn]]
    void putIntoMalfunctionMode(std::string_view errorMessage, StatusLED::Status error);

    void trySendErrorToAPI(std::string_view errorMessage, StatusLED::Status errorStatus);

    void sendACK(SerialParser& serial);

    void syncConfigurationWithReceiver(const std::vector<uint8_t>& data, uint32_t timeout);

    [[nodiscard]]
    static uint32_t toValueUARTBaudRate(UARTBaudRate enumedBaudRate);
};

#endif // !EASY_LORA_FIRMWARE_HEADER
