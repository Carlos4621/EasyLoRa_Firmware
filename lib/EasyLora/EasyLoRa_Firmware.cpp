#include "EasyLoRa_Firmware.hpp"

EasyLoRa_Firmware::EasyLoRa_Firmware(arduino::HardwareSerial &serialUSB, arduino::HardwareSerial &serialToLoRa, uint8_t m0_Pin, uint8_t m1_Pin, uint8_t auxPin)
:   serialToLoRa_m{ serialToLoRa },
    configurator_m{ serialToLoRa, m0_Pin, m1_Pin, auxPin },
    serialToAPI_m{ serialUSB },
    responseSender_m{ serialToAPI_m },
    auxPin_m{ auxPin }
{
}

void EasyLoRa_Firmware::begin() {
    serialToLoRa_m.begin();
    configurator_m.begin();
    serialToAPI_m.begin();
    statusLED_m.begin();
}

void EasyLoRa_Firmware::start() {
    configurator_m.setMode(E22_400T37S_Configurator::Modes::Transparent);
    syncBaudRateWithModule();

    statusLED_m.setStatus(EasyLoRa_SatusLED::Status::OK);
    
    while (true) {
        const auto API_EnvelopeReceived{ tryReceive_API_Envelope() };
        if (API_EnvelopeReceived) {
            manageAPIEnvelope();
        }

        const auto LoRa_EnvelopeReceived{ tryReceive_LoRa_Envelope() };
        if (LoRa_EnvelopeReceived) {
            manageLoRaEnvelope();
        }
    }
}

bool EasyLoRa_Firmware::tryReceive_API_Envelope() {
    return tryReceiveEnvelopeFromSerial(serialToAPI_m, received_API_Envelope_m, received_API_SerializedData_m);
}

bool EasyLoRa_Firmware::tryReceive_LoRa_Envelope() {
    return tryReceiveEnvelopeFromSerial(serialToLoRa_m, received_LoRa_Envelope_m, received_LoRa_SerializedData_m);
}

void EasyLoRa_Firmware::applyConfigurationMessage() {
    const auto& newConfiguration{ received_API_Envelope_m.PosibleData.configuration };

    const auto setConfigurationStatus{ configurator_m.setConfiguration(newConfiguration) };

    if (!setConfigurationStatus) {
        trySendErrorToAPI(setConfigurationStatus.error()->what(), EasyLoRa_SatusLED::Status::SetConfigurationError);
        return;
    }

    const auto successSent{ responseSender_m.sendSuccess({}) };
    if (!successSent) {
        trySendErrorToAPI(successSent.error()->what(), EasyLoRa_SatusLED::Status::SendSuccessError);
        return;
    }

    serialToLoRa_m.setBaudRate(toValueBaudRate(newConfiguration.uartBaudRate));
    // TODO: Reenviar el mensaje de configuración al otro módulo y esperar ACK?
}

void EasyLoRa_Firmware::sendConfigurationToAPI() {
    const auto getConfigurationStatus{ configurator_m.getConfiguration() };
    if (!getConfigurationStatus) {
        trySendErrorToAPI(getConfigurationStatus.error()->what(), EasyLoRa_SatusLED::Status::GetConfigurationError);
        return;
    }

    const auto serializeConfigurationStatus{ MessageEncoder<ModuleConfiguration>::encode(getConfigurationStatus.value()) };

    if (!serializeConfigurationStatus) {
        trySendErrorToAPI(serializeConfigurationStatus.error().what(), EasyLoRa_SatusLED::Status::SerializeError);
        return;
    }
    
    sendToAPI(serializeConfigurationStatus.value());
}

void EasyLoRa_Firmware::sendToAPI(const std::vector<uint8_t> &dataToSend) {
    const auto succesSentSatus{ responseSender_m.sendSuccess(dataToSend) };
    if (!succesSentSatus) {
        trySendErrorToAPI(succesSentSatus.error()->what(), EasyLoRa_SatusLED::Status::SendSuccessError);
    }
}

void EasyLoRa_Firmware::sendReceivedDataToAPI() {
    sendToAPI(received_LoRa_SerializedData_m);
}
    
void EasyLoRa_Firmware::sendReceivedDataToLoRa() {
    configurator_m.setMode(E22_400T37S_Configurator::Modes::Transparent);

    const auto sendStatus{ serialToLoRa_m.writeMessage(received_API_SerializedData_m) };
    if (!sendStatus) {
        trySendErrorToAPI(sendStatus.error().what(), EasyLoRa_SatusLED::Status::SendLoRaError);
        return;
    }

    //TODO: Implementar recibimiento de ACK
}

void EasyLoRa_Firmware::syncBaudRateWithModule() {
    const auto getConfigurationStatus{ configurator_m.getConfiguration() };
    if (!getConfigurationStatus) {
        trySendErrorToAPI(getConfigurationStatus.error()->what(), EasyLoRa_SatusLED::Status::InitializationError);
        return;
    }

    serialToLoRa_m.setBaudRate(toValueBaudRate(getConfigurationStatus.value().uartBaudRate));
}

uint32_t EasyLoRa_Firmware::toValueBaudRate(UARTBaudRate enumedBaudRate) {
    switch (enumedBaudRate) {
    case UARTBaudRate_UART_1200_BPS:
        return 1200;
    
    case UARTBaudRate_UART_2400_BPS:
        return 2400;

    case UARTBaudRate_UART_4800_BPS:
        return 4800;
    
    case UARTBaudRate_UART_9600_BPS:
        return 9600;

    case UARTBaudRate_UART_19200_BPS:
        return 19200;

    case UARTBaudRate_UART_38400_BPS:
        return 38400;

    case UARTBaudRate_UART_57600_BPS:
        return 57600;

    case UARTBaudRate_UART_115200_BPS:
        return 115200;
    }

    return 0;
}

void EasyLoRa_Firmware::manageAPIEnvelope() {
    switch (received_API_Envelope_m.which_PosibleData) {
    case Envelope_dataToSend_tag:
        // Simplemente enviar, si se requiere ACK se esperará x segundos, si no se recibe ACK se le informará al usuario
        sendReceivedDataToLoRa();
        break;

    case Envelope_configuration_tag:
        applyConfigurationMessage();
        break;

    case Envelope_requestConfiguration_tag:
        sendConfigurationToAPI();
        break;

    default:
        putIntoMalfunctionMode("Inconsistent envelope", EasyLoRa_SatusLED::Status::InconsistentEnvelopeError);
        break;
    }
}

void EasyLoRa_Firmware::manageLoRaEnvelope() {
    switch (received_LoRa_Envelope_m.which_PosibleData) {
    case Envelope_dataToSend_tag:
        sendReceivedDataToAPI();
        break;

    case Envelope_configuration_tag:
        // Configuración a colocar, una vez hecho mandar ACK
        break;

    case Envelope_requestConfiguration_tag:
        // Solicitar configuración, mandar en modo dataToSend
        break;
    
    default:
        break;
    }
}

bool EasyLoRa_Firmware::tryReceiveEnvelopeFromSerial(SerialParser &serial, Envelope &receivedEnvelope, std::vector<uint8_t>& receivedCrudeData) {
    auto envelopeStatus{ serial.readMessage() };

    if (!envelopeStatus) {
        // TODO: No se ha podido leer un mensaje, Error de conexión a la API o al módulo
        return false;
    }

    if (envelopeStatus.value().empty()) {
        return false;
    }

    envelopeStatus.value().erase(envelopeStatus.value().begin());

    const auto decodeStatus{ MessageDecoder<Envelope>::decode(envelopeStatus.value()) };

    if (!decodeStatus) {
        return false;
    }

    receivedEnvelope = decodeStatus.value();
    receivedCrudeData = envelopeStatus.value();

    return true;
}

// Error irrecuperable, para debug
void EasyLoRa_Firmware::putIntoMalfunctionMode(std::string_view errorMessage, EasyLoRa_SatusLED::Status status) {
    statusLED_m.setStatus(status);
    Serial.begin();
    while (true) {
        Serial.print("El modulo no funciona correctamente, por favor reiniciar. Error: ");
        Serial.println(errorMessage.data());

        delay(1000);
    }
}

void EasyLoRa_Firmware::trySendErrorToAPI(std::string_view errorMessage, EasyLoRa_SatusLED::Status errorStatus) {
    const auto errorSent{ responseSender_m.sendError(errorMessage) };
    if (!errorSent) {
        putIntoMalfunctionMode(errorMessage, errorStatus);
    }
}
