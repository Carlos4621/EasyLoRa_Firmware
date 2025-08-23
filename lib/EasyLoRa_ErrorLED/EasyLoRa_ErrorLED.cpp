#include "EasyLoRa_ErrorLED.hpp"

EasyLoRa_ErrorLED::EasyLoRa_ErrorLED(uint8_t pin)
: led_m{ LED_Number, pin, LED_Type }
{
}

void EasyLoRa_ErrorLED::begin() {
    led_m.begin();
}