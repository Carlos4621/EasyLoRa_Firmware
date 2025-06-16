#ifndef E22_400T37S_CONFIGURATOR_HEADER
#define E22_400T37S_CONFIGURATOR_HEADER

#include "DigitalOutput.hpp"
#include "SerialParser.hpp"
#include "SerialUART.h"
#include "DigitalInput.hpp"
#include "ModuleConfiguration.pb.h"
#include <array>
#include <tuple>

class E22_400T37S_Configurator {
public:
    enum class Modes : uint8_t { Transparent = 0, WOR, Configuration, Sleep };
    
    /// @brief Constructor base
    /// @param serialToLoRa Serial a el módulo LoRa
    /// @param m0_pin Pin a M0
    /// @param m1_pin Pin a M1
    /// @param auxPin Pin a aux
    E22_400T37S_Configurator(arduino::HardwareSerial& serialToLoRa, uint8_t m0_pin, uint8_t m1_pin, uint8_t auxPin);

    /// @brief Inicializa los pines necesarios, llamar antes de cualquier otra función
    void begin();

    /// @brief Cambia al modo deseado
    /// @param modeToSet El modo a cambiar
    /// @note La función espera a que aux esté HIGH, cambia el modo, espera de nuevo a aux y espera otros 2ms, tal como dice la documentación que debe ser
    void setMode(Modes modeToSet);

    void setConfiguration(ModuleConfiguration newConfiguration);

    /// @brief Obtiene la configuración del módulo
    /// @return La configuración del módulo
    /// @note El método se encarga de colocar el módulo en modo configuración y restaura el modo al que estaba originalmente
    [[nodiscard]]
    ModuleConfiguration getConfiguration();

private:

    static constexpr std::array<uint8_t, 2> WriteResponsePrefix{ 0xC1, 0x00 };
    static constexpr std::array<uint8_t, 3> ReadAllConfigurationCommand{ WriteResponsePrefix[0], WriteResponsePrefix[1], 0x07 };

    static constexpr uint16_t ModeSwitchingDelayInMs{ 2 };
    static constexpr uint16_t BaudRateForConfiguration{ 9600 };
    static constexpr uint8_t ExpectedConfigurationResponseSize{ 10 };
    
    static constexpr uint8_t HighAddress_Byte{ 0 };
    static constexpr uint8_t LowAddress_Byte{ 1 };
    static constexpr uint8_t NETID_Byte{ 2 };
    static constexpr uint8_t REG0_Byte{ 3 };
    static constexpr uint8_t REG1_Byte{ 4 };
    static constexpr uint8_t Channel_Byte{ 5 };
    static constexpr uint8_t REG3_Byte{ 6 };

    SerialParser serialToLoRa_m;
    DigitalOutput m0_pin_m;
    DigitalOutput m1_pin_m;
    DigitalInput auxPin_m;
    Modes currentMode_m{ Modes::Transparent };

    void setTransparentMode();
    void setWORMode();
    void setConfigurationMode();
    void setSleepMode();

    void waitForAuxRaising();

    static void setComponentsFromREG0(ModuleConfiguration& configuration, uint8_t REG0);
    static void setComponentsFromREG1(ModuleConfiguration& configuration, uint8_t REG1);
    static void setComponentsFromREG3(ModuleConfiguration& configuration, uint8_t REG3);

    [[nodiscard]]
    static UARTBaudRate getBaudRateFromREG0(uint8_t REG0);

    [[nodiscard]]
    static SerialPortParityByte getParityByteFromREG0(uint8_t REG0);

    [[nodiscard]]
    static AirDataRate getAirDataRateFromREG0(uint8_t REG0);

    [[nodiscard]]
    static SubpacketLenght getSubpacketLenghFromREG1(uint8_t REG1);

    [[nodiscard]]
    static bool getRSSINoiseFromREG1(uint8_t REG1) noexcept;

    [[nodiscard]]
    static bool getAbnormalLogEnabledFromREG1(uint8_t REG1) noexcept;

    [[nodiscard]]
    static bool getRSSIEnabledFromREG3(uint8_t REG3) noexcept;

    [[nodiscard]]
    static bool getTransmissionMethodFromREG3(uint8_t REG3) noexcept;

    [[nodiscard]]
    static bool getRelayFunctionREG3(uint8_t REG3) noexcept;

    [[nodiscard]]
    static bool getLBTEnabledFromREG3(uint8_t REG3) noexcept;

    [[nodiscard]]
    static bool getWORModeFromREG3(uint8_t REG3) noexcept;

    [[nodiscard]]
    static WORCycle getWORCycleFromREG3(uint8_t REG3);
};

class ResponseDontReceived : public std::exception {
public:
    const char* what() const noexcept override {
        return "Response don't received";
    }
};

class AbnormalResponse : public std::exception {
public:
    explicit AbnormalResponse(const std::vector<uint8_t>& response) {
        errorMessage_m = std::string{ "Abnormal response: " } + std::string(response.cbegin(), response.cend());
    }

    const char* what() const noexcept override {
        return errorMessage_m.data();
    }

private:
    std::string errorMessage_m;
};

class AbnormalRegister : public std::exception {
public:
    AbnormalRegister(std::string_view registerName, uint8_t value) {
        errorMessage_m = std::string("Abnormal register: ") + registerName.data() + " with value: " + std::to_string(value);
    }

    const char* what() const noexcept override {
        return errorMessage_m.data();
    }

private:
    std::string errorMessage_m;

};

#endif // E22_400T37S_CONFIGURATOR_HEADER