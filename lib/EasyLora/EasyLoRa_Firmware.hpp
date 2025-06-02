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

    EasyLoRa_Firmware(SerialUSB& serialUSB, SerialUART& serialToLoRa, byte m0_Pin, byte m1_Pin, byte auxPin);

    /// @brief Inicializa el dispositivo, debe ser llamado antes de cualquier otra función.
    void begin();

    /// @brief Inicia el dispositivo
    [[noreturn]]
    void start();

private:

    static constexpr std::array<uint8_t, 3> ACKMessage{ 'A', 'C', 'K' }; 
    
    std::vector<uint8_t> receivedSerialMessage_m;
    std::vector<uint8_t> receivedLoRaMessage_m;

    bool messageReceived_m{ false };
    bool messageSent_m{ false };
    bool ACKReceived_m{ false };

    SerialUSB& serialUSB_m;
    SerialUART& serialToLoRa_m;
    EBYTE LoRaConfigurator_m;

    void applyIfConfigurationMessage(const SerialMessage& decodesMessage);
    void applyChanges(const ModuleConfiguration& newConfiguration);
    void manageIncomingSerialUSBMessage();

    void processIncomingLoRaMessage();
    void checkForMessageAcknowledgement();
    void handleUnacknowledgedPacket();

    static void receivePrefixedSerialMessage(HardwareSerial& serial, std::vector<uint8_t>& buffer);

    [[nodiscard]]
    static bool isACKMessage(const std::vector<uint8_t>& message) noexcept;
};

#endif // !EASY_LORA_FIRMWARE_HEADER