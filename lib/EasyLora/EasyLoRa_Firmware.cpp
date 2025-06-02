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
            manageIncomingSerialUSBMessage();
        }

        if (serialToLoRa_m.available()) {
            processIncomingLoRaMessage();
        }

        if (messageSent_m) {
            checkForMessageAcknowledgement();
        }
    }
}

void EasyLoRa_Firmware::processIncomingLoRaMessage() {
    receivePrefixedSerialMessage(serialToLoRa_m, receivedLoRaMessage_m);
    
    if (messageReceived_m) {
        if (isACKMessage(receivedLoRaMessage_m)) {
            ACKReceived_m = true;
        }
        else {
            serialToLoRa_m.write(ACKMessage.data(), ACKMessage.size());
        }
        
        messageReceived_m = false;
    }
}

void EasyLoRa_Firmware::checkForMessageAcknowledgement() {
    if (!ACKReceived_m) {
        receivePrefixedSerialMessage(serialToLoRa_m, receivedLoRaMessage_m);
        
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

void EasyLoRa_Firmware::receivePrefixedSerialMessage(HardwareSerial &serial, std::vector<uint8_t> &buffer) {
    uint8_t messageLength;
    serial.readBytes(&messageLength, 1);
    buffer.resize(messageLength);

    serial.readBytes(buffer.data(), messageLength);
}

void EasyLoRa_Firmware::applyIfConfigurationMessage(const SerialMessage &decodesMessage) {
    if (decodesMessage.which_PosibleData == SerialMessage_configuration_tag) {
        applyChanges(decodesMessage.PosibleData.configuration);
    }    
}

void EasyLoRa_Firmware::applyChanges(const ModuleConfiguration &newConfiguration) {
    // TODO
}

void EasyLoRa_Firmware::manageIncomingSerialUSBMessage() {
    receivePrefixedSerialMessage(serialUSB_m, receivedSerialMessage_m);
    const auto decodedMessage{ SerialMessageDecoder::decode(receivedSerialMessage_m) };

    applyIfConfigurationMessage(decodedMessage);

    // TODO: Esto limita el mensaje a 255 de longitud, tal vez debería hacer otra codificación que use dos bytes?
    serialToLoRa_m.write(static_cast<uint8_t>(receivedSerialMessage_m.size()));
    serialToLoRa_m.write(receivedSerialMessage_m.data(), receivedSerialMessage_m.size());
    
    messageSent_m = true;
}

bool EasyLoRa_Firmware::isACKMessage(const std::vector<uint8_t>& message) noexcept {
    if (message.size() != ACKMessage.size()) {
        return false;
    }

    return std::equal(message.cbegin(), message.cend(), ACKMessage.cbegin(), ACKMessage.cend());
}