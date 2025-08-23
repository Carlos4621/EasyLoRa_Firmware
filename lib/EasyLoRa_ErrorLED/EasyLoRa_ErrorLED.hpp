#ifndef EASY_LORA_ERROR_LED_HEADER
#define EASY_LORA_ERROR_LED_HEADER

#include  <Adafruit_NeoPixel.h>

/// @brief Clase que representa un LED RGB Neopixel WS812 con varios colores que indican diferentes errores imposibles de enviar al usuario
class EasyLoRa_ErrorLED {
public:

    enum class Error : uint8_t { CantSetConfiguration = 0, CantSendSuccess, CantSendConfiguration, CantSerializeConfiguration, CantSendLoRaMessage };

    explicit EasyLoRa_ErrorLED(uint8_t pin);

    void begin();

private:
    static constexpr uint8_t LED_Number{ 1 };
    static constexpr neoPixelType LED_Type{ NEO_RBG + NEO_KHZ800 };

    Adafruit_NeoPixel led_m;
};

#endif // !EASY_LORA_ERROR_LED_HEADER