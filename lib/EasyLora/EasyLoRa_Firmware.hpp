#ifndef EASY_LORA_FIRMWARE_HEADER
#define EASY_LORA_FIRMWARE_HEADER

#include <Arduino.h>
#include <SerialUSB.h>
#include <vector>
#include "EnvelopeDecoder.hpp"
#include <array>

/// @brief Firmware del dispositivo EasyLoRa
class EasyLoRa_Firmware {
public:

    /// @brief Constructor base
    /// @param serialUSB Puerto en donde se enviarán y recibirán paquetes
    /// @param serialToLoRa Puerto en donde se comunicará el MCU con el chip EByte
    /// @param m0_Pin Pin a m0 del chip
    /// @param m1_Pin Pin a m1 del chip
    /// @param auxPin Pin a aux del chip
    EasyLoRa_Firmware(SerialUSB& serialUSB, SerialUART& serialToLoRa, uint8_t m0_Pin, uint8_t m1_Pin, uint8_t auxPin);

    /// @brief Inicializa el dispositivo, debe ser llamado antes de cualquier otra función.
    void begin();

    /// @brief Inicia el dispositivo
    [[noreturn]]
    void start();

private:

    SerialUSB& USBSerial_m;
    SerialUART& LoRaUART_m;
    
};

#endif // !EASY_LORA_FIRMWARE_HEADER