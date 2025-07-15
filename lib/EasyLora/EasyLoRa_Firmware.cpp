#include "EasyLoRa_Firmware.hpp"

EasyLoRa_Firmware::EasyLoRa_Firmware(arduino::HardwareSerial &serialUSB, arduino::HardwareSerial &serialToLoRa, uint8_t m0_Pin, uint8_t m1_Pin, uint8_t auxPin)
: configurator_m{ serialToLoRa, m0_Pin, m1_Pin, auxPin },
  serialToUSB_m{ serialUSB }
{
}

void EasyLoRa_Firmware::begin() {
    configurator_m.begin();
    serialToUSB_m.begin();
}

void EasyLoRa_Firmware::start() {
    configurator_m.setMode(E22_400T37S_Configurator::Modes::Transparent);

    while (true) {
        const auto envelopeReceived{ tryReceiveEnvelope() };

        if (envelopeReceived) {
            switch (receivedEnvelope_m.which_PosibleData) {
            case Envelope_dataToSend_tag:
                // Simplemente enviar, si se requiere ACK se esperará x segundos, si no se recibe ACK se le informará al usuario
                break;

            case Envelope_configuration_tag:
                // Colocar configuración y reenviar al otro módulo, este siempre debe requerir ACK
                break;
            
            default:
                // No tiene sentido recibir un ACK o un error, simplemente se ignora o tal vez parpadear un led de error
                break;
            }
        }
    }
}

bool EasyLoRa_Firmware::tryReceiveEnvelope() {
    const auto envelope{ serialToUSB_m.readMessage() };

    if (!envelope) {
        // TODO: Hacer que el error sea comunicado al usuario mediante un mensaje enviado por el puerto serial o hacer que se encienda un led de error
        return false;
    }
    
    if (envelope.value().empty()) {
        return false;
    }
    
    const auto decodeStatus{ EnvelopeDecoder::decode(envelope.value()) };

    if (!decodeStatus) {
        // TODO: Hacer que el error sea comunicado al usuario mediante un mensaje enviado por el puerto serial o hacer que se encienda un led de error
        return false;
    }

    receivedEnvelope_m = decodeStatus.value();
    return true;
}

void EasyLoRa_Firmware::applyConfigurationMessage() {
    const auto& newConfiguration{ receivedEnvelope_m.PosibleData.configuration };

    const auto setConfigurationStatus{ configurator_m.setConfiguration(newConfiguration) };

    if (!setConfigurationStatus) {
        // TODO: Hacer que el error sea comunicado al usuario mediante un mensaje enviado por el puerto serial o hacer que se encienda un led de error
        Serial.println("ERROR");
        while(true) {
            delay(10000);
        }
        return;
    }

    // Reenviar el mensaje de configuración al otro módulo
    
}
