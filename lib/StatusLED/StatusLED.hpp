#ifndef STATUS_LED_HEADER
#define STATUS_LED_HEADER

#include  <Adafruit_NeoPixel.h>

/// @brief Clase que permite mostrar el status del dispositivo EasyLoRa mediante colores
class StatusLED {
public:

    /// @brief Status posibles del módulo
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

    /// @brief Constructor base
    /// @param pin Pin del LED NeoPixel a usar
    explicit StatusLED(uint8_t pin);

    ~StatusLED();

    /// @brief Inicializa el LED, usar antes de cualquier otro método
    void begin();

    /// @brief Establece el color del status en el LED
    /// @param status status a colocar
    void setStatus(Status status);

    /// @brief Apaga el LED
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

#endif // !Status_LED_HEADER