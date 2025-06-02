#include "EasyLoRa_Firmware.hpp"

EasyLoRa_Firmware::EasyLoRa_Firmware(SerialUSB& serialUSB, SerialUART& serialToLoRa, byte m0_Pin, byte m1_Pin, byte auxPin)
: serialUSB_m{ serialUSB }
, serialToLoRa_m{ serialToLoRa }
, LoRaConfigurator_m{ &serialToLoRa_m, m0_Pin, m1_Pin, auxPin }
{
}

void EasyLoRa_Firmware::begin() {
    if (!LoRaConfigurator_m.init()) {
        throw std::runtime_error{ "No se ha podido inicializar el módulo" };
    }
    
    // TODO: Esta velocidad debe estar sincronizada con el módulo, debo primero leer la configuración y después iniciarlo
    serialToLoRa_m.begin(9600);
    serialUSB_m.begin(9600);
}

void EasyLoRa_Firmware::start() {
    while (true) {
        if (serialUSB_m.available()) {
            manageIncomingUSBMessage();
        }

        if (serialToLoRa_m.available()) {
            manageIncomingLoRaMessage();
        }

        if (messageSent_m) {
            checkForMessageAcknowledgement();
        }
    }
}

void EasyLoRa_Firmware::manageIncomingLoRaMessage() {
    receivePrefixedMessage(serialToLoRa_m, receivedLoRaMessage_m);

    if (isACKMessage(receivedLoRaMessage_m)) {
        ACKReceived_m = true;
    }
    else {
        trySendFixedSizeMessage(serialToLoRa_m, ACKMessage.data(), ACKMessage.size());
    }
}

void EasyLoRa_Firmware::checkForMessageAcknowledgement() {
    if (!ACKReceived_m) {
        receivePrefixedMessage(serialToLoRa_m, receivedLoRaMessage_m);
        
        if (isACKMessage(receivedLoRaMessage_m)) {
            ACKReceived_m = true;
        }
        
        if (!ACKReceived_m) {
            handleUnacknowledgedPacket();
        }
    }
    
    ACKReceived_m = false;
    messageSent_m = false;
}

void EasyLoRa_Firmware::handleUnacknowledgedPacket() {
    // TODO: Implementar estrategia para paquetes no confirmados
    // Por ejemplo: reintentar envío, notificar error, almacenar para reenvío, etc.
}

void EasyLoRa_Firmware::tryGetFixedSizeMessage(HardwareSerial &serial, uint8_t* buffer, size_t expectedSize) {
    const auto receivedMessageSize{ serial.readBytes(buffer, expectedSize) };

    if (receivedMessageSize != expectedSize) {
        throw MessageSizeMissmatch{ expectedSize, receivedMessageSize, true };
    }
}

void EasyLoRa_Firmware::trySendFixedSizeMessage(HardwareSerial &serial, const uint8_t* message, size_t messageSize) {
    if (serial.write(message, messageSize) == 0) {
        throw MessageSizeMissmatch{ messageSize, 0, false };
    }
}

void EasyLoRa_Firmware::sendMessageToLoRa() {
    const auto messageSize{ static_cast<uint8_t>(receivedSerialMessage_m.size()) }; // TODO: Limitamos el paquete a 255 bytes, debo corregir

    trySendFixedSizeMessage(serialToLoRa_m, &messageSize, 1);
    trySendFixedSizeMessage(serialToLoRa_m, receivedSerialMessage_m.data(), receivedSerialMessage_m.size());
}

void EasyLoRa_Firmware::receivePrefixedMessage(HardwareSerial &serial, std::vector<uint8_t> &buffer) {
    uint8_t messageSize;
    tryGetFixedSizeMessage(serial, &messageSize, MessageLenghtSizeInbytes);

    buffer.resize(messageSize);
    tryGetFixedSizeMessage(serial, buffer.data(), messageSize);
}

void EasyLoRa_Firmware::applyIfConfigurationMessage(const SerialMessage &decodesMessage) {
    if (decodesMessage.which_PosibleData == SerialMessage_configuration_tag) {
        applyNewConfiguration(decodesMessage.PosibleData.configuration);
    }    
}

void EasyLoRa_Firmware::applyNewConfiguration(const ModuleConfiguration &newConfiguration) {
    // TODO
}

void EasyLoRa_Firmware::manageIncomingUSBMessage() {
    receivePrefixedMessage(serialUSB_m, receivedSerialMessage_m);
    const auto decodedMessage{ SerialMessageDecoder::decode(receivedSerialMessage_m) };
    applyIfConfigurationMessage(decodedMessage);

    sendMessageToLoRa();
    messageSent_m = true;
}

bool EasyLoRa_Firmware::isACKMessage(const std::vector<uint8_t>& message) noexcept {
    if (message.size() != ACKMessage.size()) {
        return false;
    }

    return std::equal(message.cbegin(), message.cend(), ACKMessage.cbegin(), ACKMessage.cend());
}