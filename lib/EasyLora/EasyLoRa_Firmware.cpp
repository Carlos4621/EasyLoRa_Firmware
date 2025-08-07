#include "EasyLoRa_Firmware.hpp"

EasyLoRa_Firmware::EasyLoRa_Firmware(arduino::HardwareSerial &serialUSB, arduino::HardwareSerial &serialToLoRa, uint8_t m0_Pin, uint8_t m1_Pin, uint8_t auxPin)
:   serialToLoRa_m{ serialToLoRa },
    configurator_m{ serialToLoRa, m0_Pin, m1_Pin, auxPin },
    serialToUSB_m{ serialUSB },
    responseSender_m{ serialToUSB_m }
{
}

void EasyLoRa_Firmware::begin() {
    serialToLoRa_m.begin();
    configurator_m.begin();
    serialToUSB_m.begin();
}

void EasyLoRa_Firmware::start() {
    configurator_m.setMode(E22_400T37S_Configurator::Modes::Transparent);
    syncBaudRateWithModule();

    while (true) {
        const auto envelopeReceived{ tryReceiveEnvelope() };

        if (envelopeReceived) {
            switch (received_API_Envelope_m.which_PosibleData) {
            case Envelope_dataToSend_tag:
                // Simplemente enviar, si se requiere ACK se esperará x segundos, si no se recibe ACK se le informará al usuario
                Serial.println("Uknow package");
                break;

            case Envelope_configuration_tag:
                applyConfigurationMessage();
                break;
            
            case Envelope_requestConfiguration_tag:
                sendConfigurationToAPI();
                break;

            default:
                // No tiene sentido recibir un ACK o un error, simplemente se ignora o tal vez parpadear un led de error
                Serial.println("Uknow package");
                break;
            }
        }
    }
}

bool EasyLoRa_Firmware::tryReceiveEnvelope() {
    auto envelope{ serialToUSB_m.readMessage() };

    if (!envelope) {
        // TODO: Hacer que se encienda un led de error
        return false;
    }
    
    if (envelope.value().empty()) {
        return false;
    }

    envelope.value().erase(envelope.value().begin());

    const auto decodeStatus{ MessageDecoder<Envelope>::decode(envelope.value()) };

    if (!decodeStatus) {
        // TODO: Hacer que se encienda un led de error
        return false;
    }

    received_API_Envelope_m = decodeStatus.value();
    return true;
}

void EasyLoRa_Firmware::applyConfigurationMessage() {
    const auto& newConfiguration{ received_API_Envelope_m.PosibleData.configuration };

    const auto setConfigurationStatus{ configurator_m.setConfiguration(newConfiguration) };

    if (!setConfigurationStatus) {
        const auto errorSent{ responseSender_m.sendError(setConfigurationStatus.error()->what()) };
        if (!errorSent) {
            // TODO: Manejar el caso donde no se pudo enviar el error (ej: encender LED de error)
        }
        return;
    }

    const bool successSent{ responseSender_m.sendSuccess() };
    if (!successSent) {
        // TODO: Manejar el caso donde no se pudo enviar el mensaje de éxito (ej: encender LED de error)
    }

    serialToLoRa_m.setBaudRate(toValueBaudRate(newConfiguration.uartBaudRate));
    // TODO: Reenviar el mensaje de configuración al otro módulo
}

void EasyLoRa_Firmware::sendConfigurationToAPI() {
    const auto getConfigurationStatus{ configurator_m.getConfiguration() };
    if (!getConfigurationStatus) {
        const auto errorSent{ responseSender_m.sendError(getConfigurationStatus.error()->what()) };
        if (!errorSent) {
            // TODO: Manejar el caso donde no se pudo enviar el error (ej: encender LED de error)
        }
        return;
    }

    const auto serializeConfigurationStatus{ MessageEncoder<ModuleConfiguration>::encode(getConfigurationStatus.value()) };

    if (!serializeConfigurationStatus) {
        const auto errorSent{ responseSender_m.sendError(serializeConfigurationStatus.error().what()) };
        if (!errorSent) {
            // TODO: Manejar el caso donde no se pudo enviar el error (ej: encender LED de error)
        }
        return;
    }
    
    const auto& toSend{ serializeConfigurationStatus.value() };
    const auto succesSentSatus{ responseSender_m.sendSuccess(toSend) };
    if (!succesSentSatus) {
        // TODO: Led de error
    }

}

void EasyLoRa_Firmware::syncBaudRateWithModule() {
    const auto getConfigurationStatus{ configurator_m.getConfiguration() };
    if (!getConfigurationStatus) {
        const auto errorSent{ responseSender_m.sendError(getConfigurationStatus.error()->what()) };
        if (!errorSent) {
            // TODO: Manejar el caso donde no se pudo enviar el error (ej: encender LED de error)
        }
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