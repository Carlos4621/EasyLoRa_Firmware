#ifndef EASY_LORA_FIRMWARE_HEADER
#define EASY_LORA_FIRMWARE_HEADER

// <>

#include <Arduino.h>
#include <SerialUSB.h>
#include <vector>
#include <pb_encode.h>
#include "MessageDecoder.hpp"
#include <array>
#include "E22_400T37S_Configurator.hpp"
#include "MessageEncoder.hpp"
#include "ResponseSender.hpp"
#include "EasyLoRa_StatusLED.hpp"

/*
    TODO:
    - Ahora los errores son irrecuperables, esto piensa cambiarse
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

    void setTimeout(uint16_t timeoutInMs);

private:

    SerialParser serialToLoRa_m;
    E22_400T37S_Configurator configurator_m;
    SerialParser serialToAPI_m;
    ResponseSender responseSender_m;
    
    Envelope received_API_Envelope_m;
    Envelope received_LoRa_Envelope_m;

    std::vector<uint8_t> received_API_SerializedData_m;
    std::vector<uint8_t> received_LoRa_SerializedData_m;

    uint16_t timeoutInMs_m{ 1000 };

    DigitalInput auxPin_m;

    EasyLoRa_SatusLED statusLED_m{ PIN_NEOPIXEL };

    [[nodiscard]]
    bool tryReceive_API_Envelope();

    [[nodiscard]]
    bool tryReceive_LoRa_Envelope();

    void applyConfigurationMessage();
    void sendConfigurationToAPI();
    void sendToAPI(const std::vector<uint8_t>& dataToSend);
    void sendReceivedDataToAPI();
    void sendReceivedDataToLoRa();
    void syncBaudRateWithModule();

    [[nodiscard]]
    static uint32_t toValueBaudRate(UARTBaudRate enumedBaudRate);

    void manageAPIEnvelope();
    void manageLoRaEnvelope();

    [[nodiscard]]
    static bool tryReceiveEnvelopeFromSerial(SerialParser& serial, Envelope& receivedEnvelope, std::vector<uint8_t>& receivedCrudeData);

    void putIntoMalfunctionMode(std::string_view errorMessage, EasyLoRa_SatusLED::Status error);
};

#endif // !EASY_LORA_FIRMWARE_HEADER