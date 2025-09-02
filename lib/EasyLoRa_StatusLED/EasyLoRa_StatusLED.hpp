#ifndef EASY_LORA_STATUS_LED_HEADER
#define EASY_LORA_STATUS_LED_HEADER

#include  <Adafruit_NeoPixel.h>

/// @brief Clase que permite mostrar el status del dispositivo EasyLoRa mediante colores
class EasyLoRa_SatusLED {
public:

    enum class Status : uint8_t { 
        OK = 0,
        InitializationError,
        SetConfigurationError,
        SendSuccessError,
        SyncConfigurationError,
        GetConfigurationError,
        SendLoRaError,
        InconsistentEnvelopeError,
        SerializeError,
    };

    explicit EasyLoRa_SatusLED(uint8_t pin);

    ~EasyLoRa_SatusLED();

    void begin();

    void setStatus(Status status);

    void shutdowmLED();

private:
    static constexpr uint8_t LED_Number{ 1 };
    static constexpr neoPixelType LED_Type{ NEO_RBG + NEO_KHZ800 };
    static constexpr uint8_t Default_Brightness{ 50 };

    static constexpr uint32_t OK_Color{ 0x000000FF }; // Verde
    static constexpr uint32_t Initialization_Error_Color{ 0x00FF0000 }; // Rojo
    static constexpr uint32_t Set_Configuration_Error_Color{ 0x00FF00A5 }; // Naranja
    static constexpr uint32_t Send_Success_Error_Color{ 0x0000FF00 }; // Azul
    static constexpr uint32_t Sync_Cofiguration_Error_Color{ 0x0000FFFF }; // Cian
    static constexpr uint32_t Get_Configuration_Error_Color{ 0x00FF00FF }; // Morado
    static constexpr uint32_t Send_LoRa_Error_Color{ 0x00FF0080 }; // Rosa
    static constexpr uint32_t Inconsistent_Envelope_Error_Color{ 0x00FFFFFF }; // Blanco
    static constexpr uint32_t Serialize_Error_Color{ 0x00FFD700 }; // Dorado

    Adafruit_NeoPixel led_m;
};

#endif // !EASY_LORA_Status_LED_HEADER