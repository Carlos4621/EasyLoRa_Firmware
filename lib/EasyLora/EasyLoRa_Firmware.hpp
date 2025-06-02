#ifndef EASY_LORA_FIRMWARE_HEADER
#define EASY_LORA_FIRMWARE_HEADER

#include <Arduino.h>
#include <SerialUSB.h>
#include <vector>
#include <EBYTE.h>
#include "SerialMessageDecoder.hpp"
#include "EBYTE.h"
#include <span>
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
    EasyLoRa_Firmware(SerialUSB& serialUSB, SerialUART& serialToLoRa, byte m0_Pin, byte m1_Pin, byte auxPin);

    /// @brief Inicializa el dispositivo, debe ser llamado antes de cualquier otra función.
    void begin();

    /// @brief Inicia el dispositivo
    [[noreturn]]
    void start();

private:

    static constexpr std::array<uint8_t, 3> ACKMessage{ 'A', 'C', 'K' }; 
    static constexpr uint8_t MessageLenghtSizeInbytes{ 1 };
    
    std::vector<uint8_t> receivedSerialMessage_m;
    std::vector<uint8_t> receivedLoRaMessage_m;

    bool messageSent_m{ false };
    bool ACKReceived_m{ false };

    SerialUSB& serialUSB_m;
    SerialUART& serialToLoRa_m;
    EBYTE LoRaConfigurator_m;

    void applyIfConfigurationMessage(const SerialMessage& decodedMessage);
    void applyNewConfiguration(const ModuleConfiguration& newConfiguration);
    void manageIncomingUSBMessage();

    void manageIncomingLoRaMessage();
    void checkForMessageAcknowledgement();
    void handleUnacknowledgedPacket();

    void sendMessageToLoRa();

    static void tryGetFixedSizeMessage(HardwareSerial& serial, uint8_t* buffer, size_t expectedSize);
    static void trySendFixedSizeMessage(HardwareSerial& serial, const uint8_t* const message, size_t messageSize);

    static void receivePrefixedMessage(HardwareSerial& serial, std::vector<uint8_t>& buffer);

    [[nodiscard]]
    static bool isACKMessage(const std::vector<uint8_t>& message) noexcept;
};

class MessageSizeMissmatch : std::exception {
public:

    MessageSizeMissmatch(size_t sizeExpected, size_t sizeReceived, bool onRead)
    : sizeExpected_m{ sizeExpected }
    , sizeReceived_m{ sizeReceived }
    , onRead_m{ onRead }
    {
    }

    const char* what() const noexcept override {
        return (std::string("Tamaño del mensaje incorrecto. Esperado: ") + std::to_string(sizeExpected_m) + (onRead_m ? " Recibido: " : " Escrito: ") 
        + std::to_string(sizeReceived_m)).data();
    }

private:

    size_t sizeExpected_m;
    size_t sizeReceived_m;
    bool onRead_m;
};

#endif // !EASY_LORA_FIRMWARE_HEADER