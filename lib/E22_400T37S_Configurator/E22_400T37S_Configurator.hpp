#ifndef E22_400T37S_CONFIGURATOR_HEADER
#define E22_400T37S_CONFIGURATOR_HEADER

#include "DigitalOutput.hpp"
#include "SerialParser.hpp"
#include "SerialUART.h"
#include "DigitalInput.hpp"
#include <pb.h>
#include "ModuleConfiguration.pb.h"
#include <array>
#include <tuple>
#include "tl/expected.hpp"
#include <memory>
#include <sstream>
#include <bitset>

class E22_400T37S_Configurator {
public:

    class ResponseDontReceived;
    class AbnormalResponse;
    class AbnormalRegister;

    enum class Modes : uint8_t { Transparent = 0, WOR, Configuration, Sleep };

    /// @brief Valores dados por documentación del módulo
    /// @note WORCycle no tiene un valor por defecto en la documentación, se presume ser 0x00 en este caso
    static constexpr ModuleConfiguration Default_Settings{
        .addressHighByte = 0,
        .addressLowByte = 0,
        .NETID = 0,
        .Channel = 0x17,
        .enableRSSI = false,
        .RSSIByte = false,
        .enableAbnormalLog = false,
        .enableFixedTransmitionMode = false,
        .enableRepeaterMode = false,
        .enableLBT = false,
        .enableWORMode = false,
        .airDataRate = AirDataRate_AirRate_2400_BPS,
        .subpacketLenght = SubpacketLenght_Bytes_Lenght_240,
        .serialPortParityByte = SerialPortParityByte_Byte_8N1,
        .worCycle = WORCycle_WORCycle_500_ms,
        .uartBaudRate = UARTBaudRate_UART_9600_BPS
    };

    /// @brief Constructor base
    /// @param serialToLoRa Serial a el módulo LoRa
    /// @param m0_pin Pin a M0
    /// @param m1_pin Pin a M1
    /// @param auxPin Pin a aux
    E22_400T37S_Configurator(arduino::HardwareSerial& serialToLoRa, uint8_t m0_pin, uint8_t m1_pin, uint8_t auxPin);

    /// @brief Inicializa los pines necesarios, llamar antes de cualquier otra función
    /// @note Esto NO inicializa el puerto serial debido a que la clase no lo usa necesariamente, esto no es problema ya que cuando se cambia la configuración se establece al
    /// baud rate necesario
    void begin();

    /// @brief Cambia al modo deseado
    /// @param modeToSet El modo a cambiar
    /// @note La función espera a que aux esté HIGH, cambia el modo, espera de nuevo a aux y espera otros 2ms, tal como dice la documentación que debe ser
    void setMode(Modes modeToSet);

    /// @brief Establece la configuración enviada
    /// @param newConfiguration Nueva configuración a establecer
    /// @return tl::expected con AbnormalRegister en caso de que algún valor sea anómalo, ResponseDontReceived en caso de que no se reciba respuesta,
    /// MessageSizeMissmatch en caso de que el tamaño del mensaje no coincida con lo esperado
    [[nodiscard]]
    tl::expected<void, std::shared_ptr<std::exception>> setConfiguration(const ModuleConfiguration& newConfiguration);

    /// @brief Coloca la configuración default
    /// @return tl::expected con AbnormalRegister en caso de que algún valor sea anómalo, ResponseDontReceived en caso de que no se reciba respuesta,
    /// MessageSizeMissmatch en caso de que el tamaño del mensaje no coincida con lo esperado
    [[nodiscard]]
    tl::expected<void, std::shared_ptr<std::exception>> setDefaultCofiguration();

    /// @brief Obtiene la configuración del módulo
    /// @return tl::expected con la configuración del módulo, AbnormalRegister en caso de un valor anómalo en un registro, ResponseDontReceived en caso de que no haya respuesta
    /// @note El método se encarga de colocar el módulo en modo configuración y restaura el modo al que estaba originalmente
    [[nodiscard]]
    tl::expected<ModuleConfiguration, std::shared_ptr<std::exception>> getConfiguration();

private:

    static constexpr uint8_t Write_Command_Prefix{ 0xC0 };
    static constexpr uint8_t Write_Read_Response_Prefix{ 0xC1 };
    static constexpr uint8_t Register_Start_Address{ 0x00 };
    static constexpr uint8_t Registers_Length{ 0x07 };

    static constexpr std::array Read_All_Configurations_Command{ Write_Read_Response_Prefix, Register_Start_Address, Registers_Length };
 
    static constexpr uint16_t Mode_Switching_Delay_In_Ms{ 2 };
    static constexpr uint16_t Baud_Rate_For_Configuration{ 9600 };
    static constexpr uint8_t Expected_Read_Configuration_Response_Size{ 10 };
    
    static constexpr uint8_t Prefix_Length{ 2 };
    static constexpr uint8_t Prefix_Displacement{ 3 };
    static constexpr uint8_t High_Address_Byte{ 0 + Prefix_Displacement };
    static constexpr uint8_t Low_Address_Byte{ 1 + Prefix_Displacement };
    static constexpr uint8_t NETID_Byte{ 2 + Prefix_Displacement };
    static constexpr uint8_t REG0_Byte{ 3 + Prefix_Displacement };
    static constexpr uint8_t REG1_Byte{ 4 + Prefix_Displacement };
    static constexpr uint8_t Channel_Byte{ 5 + Prefix_Displacement };
    static constexpr uint8_t REG3_Byte{ 6 + Prefix_Displacement };

    static constexpr uint8_t Max_Channel{ 83 };

    static constexpr uint8_t UART_Baud_Rate_Shift{ 5 };
    static constexpr uint8_t Parity_Byte_Shift{ 3 };
    static constexpr uint8_t Parity_Byte_Mask{ 0b11 };
    static constexpr uint8_t Air_Data_Rate_Mask{ 0b111 };

    static constexpr uint8_t Subpacket_Length_Shift{ 6 };
    static constexpr uint8_t RSSI_Byte_Shift{ 5 };
    static constexpr uint8_t Abnormal_Log_Shift{ 2 };
    static constexpr uint8_t Single_Bit_Mask{ 1 };

    static constexpr uint8_t RSSI_Enabled_Shift{ 7 };
    static constexpr uint8_t Transmission_Method_Shift{ 6 };
    static constexpr uint8_t Relay_Mode_Shift{ 5 };
    static constexpr uint8_t LBT_Enabled_Shift{ 4 };
    static constexpr uint8_t WOR_Enabled_Shift{ 3 };
    static constexpr uint8_t WOR_Cycle_Mask{ 0b111 };

    static constexpr uint8_t Watchdog_Delay_Ms{ 1 };

    SerialParser serialToLoRa_m;
    DigitalOutput m0_pin_m;
    DigitalOutput m1_pin_m;
    DigitalInput auxPin_m;
    Modes currentMode_m{ Modes::Transparent };

    Modes previousMode_m{ Modes::Transparent };
    uint64_t previousBaudRate_m{ 0 };

    void setTransparentMode();
    void setWORMode();
    void setConfigurationMode();
    void setSleepMode();

    void waitForAuxRaising();

    static void setComponentsFromREG0(ModuleConfiguration& configuration, uint8_t REG0);
    static void setComponentsFromREG1(ModuleConfiguration& configuration, uint8_t REG1);
    static void setComponentsFromREG3(ModuleConfiguration& configuration, uint8_t REG3);

    static void setComponentsToREG0(uint8_t& REG0, const ModuleConfiguration& configuration);
    static void setComponentsToREG1(uint8_t& REG1, const ModuleConfiguration& configuration);
    static void setComponentsToREG3(uint8_t& REG3, const ModuleConfiguration& configuration);

    [[nodiscard]]
    static UARTBaudRate getBaudRateFromREG0(uint8_t REG0);

    [[nodiscard]]
    static SerialPortParityByte getParityByteFromREG0(uint8_t REG0);

    [[nodiscard]]
    static AirDataRate getAirDataRateFromREG0(uint8_t REG0);

    [[nodiscard]]
    static SubpacketLenght getSubpacketLenghFromREG1(uint8_t REG1);

    [[nodiscard]]
    static bool getRSSINoiseFromREG1(uint8_t REG1);

    [[nodiscard]]
    static bool getAbnormalLogEnabledFromREG1(uint8_t REG1);

    [[nodiscard]]
    static bool getRSSIEnabledFromREG3(uint8_t REG3);

    [[nodiscard]]
    static bool getTransmissionMethodFromREG3(uint8_t REG3);

    [[nodiscard]]
    static bool getRelayFunctionREG3(uint8_t REG3);

    [[nodiscard]]
    static bool getLBTEnabledFromREG3(uint8_t REG3);

    [[nodiscard]]
    static bool getWORModeFromREG3(uint8_t REG3);

    [[nodiscard]]
    static WORCycle getWORCycleFromREG3(uint8_t REG3);

    void setupForConfiguration();
    void restorePreviousValues();

    [[nodiscard]]
    static bool isValidChannel(uint8_t channel);

    [[nodiscard]]
    static bool isValidWriteResponse(const std::vector<uint8_t>& messageSent, const std::vector<uint8_t>& responseReceived);

    [[nodiscard]]
    static bool isValidReadResponse(const std::vector<uint8_t>& responseReceived);
};

class E22_400T37S_Configurator::ResponseDontReceived : public std::exception {
public:

    [[nodiscard]]
    const char* what() const noexcept override {
        return "Response don't received";
    }
};

class E22_400T37S_Configurator::AbnormalResponse : public std::exception {
public:
    explicit AbnormalResponse(const std::vector<uint8_t>& response)
    : errorMessage_m{ std::string{ "Abnormal response: " } + std::string(response.cbegin(), response.cend()) }
    {}

    [[nodiscard]]
    const char* what() const noexcept override {
        return errorMessage_m.data();
    }

private:
    std::string errorMessage_m;
};

class E22_400T37S_Configurator::AbnormalRegister : public std::exception {
public:
    AbnormalRegister(std::string_view registerName, uint8_t value) 
    : errorMessage_m{ std::string("Abnormal register: ") + registerName.data() + " with value: " + std::to_string(value) }
    {}

    [[nodiscard]]
    const char* what() const noexcept override {
        return errorMessage_m.data();
    }

private:
    std::string errorMessage_m;

};

/// @brief Método de conveniecia para obtener una string con formato que muestra las configuraciones enviadas
/// @return std::string con las configuraciones
/// @attention El uso de esta función aumenta DEMASIADO el peso del binario, usar con suma precaución
[[nodiscard]]
static std::string stringConfigurations(const ModuleConfiguration& configs) {
    std::ostringstream output;

    output <<
    "High address: " << std::hex << configs.addressHighByte << '\n' <<
    "Low address: " << configs.addressLowByte << '\n' <<
    "NETID: " << std::dec << configs.NETID << '\n' <<
    "UART bps: " << std::bitset<3>(configs.uartBaudRate) << '\n' <<
    "Parity byte: " << std::bitset<2>(configs.serialPortParityByte) << '\n' <<
    "Air rate: " << std::bitset<3>(configs.airDataRate) << '\n' <<
    "Subpacket: " << std::bitset<2>(configs.subpacketLenght) << '\n' <<
    "RSSI noise: " << configs.RSSIByte << '\n' <<
    "Abnormal register: " << configs.enableAbnormalLog << '\n' <<
    "Channel: " << std::hex << configs.Channel << '\n' <<
    "Enable RSSI: " << configs.enableRSSI << '\n' <<
    "Transmition method: " << configs.enableFixedTransmitionMode << '\n' <<
    "Relay Mode: " << configs.enableRepeaterMode << '\n' <<
    "LBT enabled: " << configs.enableLBT << '\n' <<
    "WOR mode: " << configs.enableWORMode << '\n' <<
    "WOR cycle: " << std::bitset<3>(configs.worCycle);

    return output.str();
};

#endif // E22_400T37S_CONFIGURATOR_HEADER