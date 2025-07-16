#ifndef EASY_LORA_FIRMWARE_HEADER
#define EASY_LORA_FIRMWARE_HEADER

#include <Arduino.h>
#include <SerialUSB.h>
#include <vector>
#include <pb_encode.h>
#include "MessageDecoder.hpp"
#include <array>
#include "E22_400T37S_Configurator.hpp"
#include "MessageEncoder.hpp"
#include "ResponseSender.hpp"

/*
    TODO: Solucionar los siguientes problemas
    - Quitar todos los Serial.println(), son solo para debug temprano
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

    E22_400T37S_Configurator configurator_m;
    SerialParser serialToUSB_m;
    ResponseSender responseSender_m;
    
    Envelope receivedEnvelope_m;

    uint16_t timeoutInMs_m{ 1000 };

    [[nodiscard]]
    bool tryReceiveEnvelope();

    void applyConfigurationMessage();
};

#endif // !EASY_LORA_FIRMWARE_HEADER