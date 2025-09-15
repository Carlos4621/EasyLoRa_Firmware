#include "EasyLoRa_Firmware.hpp"

struct EasyLoRa_Firmware::EnvelopeBundle {
    EnvelopeBundle(const Envelope& envelope_, const std::vector<uint8_t>& encodedEnvelope_)
    : envelope{ envelope_ }
    , encodedEnvelope{ encodedEnvelope_ }
    {
    }

    Envelope envelope;
    std::vector<uint8_t> encodedEnvelope;
};

EasyLoRa_Firmware::EasyLoRa_Firmware(arduino::HardwareSerial &serialUSB, arduino::HardwareSerial &serialToLoRa, uint8_t m0_Pin, uint8_t m1_Pin, uint8_t auxPin)
:   serialToLoRa_m{ serialToLoRa },
    configurator_m{ serialToLoRa_m, m0_Pin, m1_Pin, auxPin },
    serialToAPI_m{ serialUSB },
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
    syncModuleConfiguration();

    statusLED_m.setStatus(StatusLED::Status::OK);
    
    while (true) {
        const auto API_EnvelopeReceived{ receiveEnvelopeFromSerial(serialToAPI_m) };
        if (API_EnvelopeReceived) {
            manageAPIEnvelope(API_EnvelopeReceived.value());
        }

        const auto LoRa_EnvelopeReceived{ receiveEnvelopeFromSerial(serialToLoRa_m) };
        if (LoRa_EnvelopeReceived) {
            manageLoRaEnvelope(LoRa_EnvelopeReceived.value());
        }
    }
}

void EasyLoRa_Firmware::manageAPIEnvelope(const EnvelopeBundle& envelopeBundle) {
    switch (envelopeBundle.envelope.which_PosibleData) {
    case Envelope_data_tag:
        sendToSerial(serialToLoRa_m, envelopeBundle.encodedEnvelope, StatusLED::Status::SendLoRaError);
        break;

    case Envelope_configuration_tag:
        applyConfiguration(envelopeBundle.envelope.PosibleData.configuration);
        break;

    case Envelope_requestConfiguration_tag:
        sendConfigurationToAPI();
        break;

    default:
        putIntoMalfunctionMode("Inconsistent envelope", StatusLED::Status::InconsistentEnvelopeError);
        break;
    }
}

void EasyLoRa_Firmware::manageLoRaEnvelope(const EnvelopeBundle& envelopeBundle) {
    switch (envelopeBundle.envelope.which_PosibleData) {
    case Envelope_data_tag:
        sendToSerial(serialToAPI_m, envelopeBundle.encodedEnvelope, StatusLED::Status::SendSuccessError);
        break;

    case Envelope_configuration_tag:
        // TODO: Aplicar y enviar ACK
        break;

    case Envelope_requestConfiguration_tag:
        // TODO: Solicitar configuración, mandar en modo dataToSend
        break;
    
    default:
        break;
    }
}

void EasyLoRa_Firmware::applyConfiguration(const ModuleConfiguration& configuration) {
    const auto setConfigurationStatus = configurator_m.setConfiguration(configuration);

    if (!setConfigurationStatus) {
        trySendErrorToAPI(setConfigurationStatus.error()->what(), StatusLED::Status::SetConfigurationError);
        return;
    }

    actualConfiguration_m = configuration;
    serialToLoRa_m.setBaudRate(toValueUARTBaudRate(configuration.uartBaudRate));
    
    sendACK(serialToAPI_m);
    // TODO: Reenviar el mensaje de configuración al otro módulo y esperar ACK?
}

void EasyLoRa_Firmware::sendConfigurationToAPI() {
    const auto serializeConfigurationStatus = MessageEncoder<Envelope>::encode(EnvelopeFactory::withModuleConfiguration(actualConfiguration_m));

    if (!serializeConfigurationStatus) {
        trySendErrorToAPI(serializeConfigurationStatus.error()->what(), StatusLED::Status::SerializeError);
        return;
    }
    
    sendToSerial(serialToAPI_m, serializeConfigurationStatus.value(), StatusLED::Status::SendSuccessError);
}

void EasyLoRa_Firmware::sendToSerial(SerialParser &serial, const std::vector<uint8_t>& data, StatusLED::Status errorIfFails) {
    const auto sendStatus{ serial.writeMessage(data) };
    
    if (!sendStatus) {
        if (&serial == &serialToAPI_m) {
            putIntoMalfunctionMode(sendStatus.error()->what(), errorIfFails);
        }
        else {
            trySendErrorToAPI(sendStatus.error()->what(), errorIfFails);
        }
    }
}

void EasyLoRa_Firmware::syncModuleConfiguration() {
    const auto getConfigurationStatus = configurator_m.getConfiguration();
    if (!getConfigurationStatus) {
        trySendErrorToAPI(getConfigurationStatus.error()->what(), StatusLED::Status::InitializationError);
        return;
    }

    actualConfiguration_m = getConfigurationStatus.value();
    serialToLoRa_m.setBaudRate(toValueUARTBaudRate(actualConfiguration_m.uartBaudRate));
}

std::optional<EasyLoRa_Firmware::EnvelopeBundle> EasyLoRa_Firmware::receiveEnvelopeFromSerial(SerialParser &serial) {
    auto envelopeStatus{ serial.readMessage() };

    if (!envelopeStatus) {
        // TODO: Cambiar a error correcto
        trySendErrorToAPI(envelopeStatus.error()->what(), StatusLED::Status::InconsistentEnvelopeError);
        return std::nullopt;
    }

    if (envelopeStatus.value().empty()) {
        return std::nullopt;
    }

    envelopeStatus.value().erase(envelopeStatus.value().begin()); // Quitar el size byte

    const auto decodeStatus{ MessageDecoder<Envelope>::decode(envelopeStatus.value()) };

    if (!decodeStatus) {
        trySendErrorToAPI(decodeStatus.error()->what(), StatusLED::Status::SerializeError);
        return std::nullopt;
    }

    return std::make_optional<EnvelopeBundle>(decodeStatus.value(), envelopeStatus.value());
}

// Error irrecuperable, para debug
void EasyLoRa_Firmware::putIntoMalfunctionMode(std::string_view errorMessage, StatusLED::Status status) {
    statusLED_m.setStatus(status);
    Serial.begin();
    while (true) {
        Serial.print("El modulo no funciona correctamente, por favor reiniciar. Error: ");
        Serial.println(errorMessage.data());

        delay(1000);
    }
}

void EasyLoRa_Firmware::trySendErrorToAPI(std::string_view errorMessage, StatusLED::Status errorStatus) {
    const auto encodeStatus = MessageEncoder<Envelope>::encode(EnvelopeFactory::withError(errorMessage));
    if (!encodeStatus) {
        putIntoMalfunctionMode(encodeStatus.error()->what(), StatusLED::Status::SerializeError);
        return;
    }
    
    sendToSerial(serialToAPI_m, encodeStatus.value(), errorStatus);
}

void EasyLoRa_Firmware::sendACK(SerialParser &serial) {
    const auto encodeStatus = MessageEncoder<Envelope>::encode(EnvelopeFactory::withACK());
    if (!encodeStatus) {
        trySendErrorToAPI(encodeStatus.error()->what(), StatusLED::Status::SerializeError);
        return;
    }
    
    sendToSerial(serial, encodeStatus.value(), StatusLED::Status::SendSuccessError);
}

uint32_t EasyLoRa_Firmware::toValueUARTBaudRate(UARTBaudRate enumedBaudRate) {
    switch (enumedBaudRate) {
    case UARTBaudRate_UART_1200_BPS: return 1200;
    case UARTBaudRate_UART_2400_BPS: return 2400;
    case UARTBaudRate_UART_4800_BPS: return 4800;
    case UARTBaudRate_UART_9600_BPS: return 9600;
    case UARTBaudRate_UART_19200_BPS: return 19200;
    case UARTBaudRate_UART_38400_BPS: return 38400;
    case UARTBaudRate_UART_57600_BPS: return 57600;
    case UARTBaudRate_UART_115200_BPS: return 115200;
    }

    __unreachable();
}
