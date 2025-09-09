#include "EasyLoRa_StatusLED.hpp"

EasyLoRa_SatusLED::EasyLoRa_SatusLED(uint8_t pin)
: led_m{ LED_Number, pin, LED_Type }
{
}

EasyLoRa_SatusLED::~EasyLoRa_SatusLED() {
    shutdowmLED();
}

void EasyLoRa_SatusLED::begin() {
    led_m.begin();
    led_m.setBrightness(Default_Brightness);
    shutdowmLED();
}

void EasyLoRa_SatusLED::setStatus(Status status) {
    switch (status) {
    case Status::OK:
        led_m.setPixelColor(0, OK_Color);
        break;

    case Status::InitializationError:
        led_m.setPixelColor(0, Initialization_Error_Color);
        break;

    case Status::SetConfigurationError:
        led_m.setPixelColor(0, Set_Configuration_Error_Color);
        break;
    
    case Status::SyncConfigurationError:
        led_m.setPixelColor(0, Sync_Cofiguration_Error_Color);
        break;

    case Status::SendLoRaError:
        led_m.setPixelColor(0, Send_LoRa_Error_Color);
        break;

    case Status::InconsistentEnvelopeError:
        led_m.setPixelColor(0, Inconsistent_Envelope_Error_Color);
        break;

    case Status::SerializeError:
        led_m.setPixelColor(0, Serialize_Error_Color);
        break;

    case Status::SendSuccessError:
        led_m.setPixelColor(0, Send_Success_Error_Color);
        break;

    case Status::GetConfigurationError:
        led_m.setPixelColor(0, Get_Configuration_Error_Color);
        break;
    }

    led_m.show();
}

void EasyLoRa_SatusLED::shutdowmLED() {
    led_m.clear();
    led_m.show();
}
