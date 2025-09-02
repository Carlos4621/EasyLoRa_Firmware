#include "E22_400T37S_Configurator.hpp"

E22_400T37S_Configurator::E22_400T37S_Configurator(arduino::HardwareSerial& serialToLoRa, uint8_t m0_pin, uint8_t m1_pin, uint8_t auxPin)
: serialToLoRa_m{ serialToLoRa }
, m0_pin_m{ m0_pin }
, m1_pin_m{ m1_pin }
, auxPin_m{ auxPin }
{
}

void E22_400T37S_Configurator::begin() {
    m0_pin_m.begin();
    m1_pin_m.begin();
    auxPin_m.begin();
    serialToLoRa_m.begin();
}

void E22_400T37S_Configurator::setMode(Modes modeToSet) {
    if (modeToSet == currentMode_m) {
        return;
    }

    switch (modeToSet) {
    case Modes::Transparent:
        setTransparentMode();
        break;
    
    case Modes::WOR:
        setWORMode();
        break;

    case Modes::Configuration:
        setConfigurationMode();
        break;

    case Modes::Sleep:
        setSleepMode();
        break;
    }

    delay(Mode_Switching_Delay_In_Ms);

    currentMode_m = modeToSet;
}

tl::expected<void, std::shared_ptr<std::exception>> E22_400T37S_Configurator::setConfiguration(const ModuleConfiguration& newConfiguration) {
    if (!isValidChannel(newConfiguration.Channel)) {
        return tl::make_unexpected(std::make_shared<AbnormalRegister>("Channel", newConfiguration.Channel));
    }
    
    std::array<uint8_t, 10> messageToSend{ Write_Command_Prefix, Register_Start_Address, Registers_Length };

    messageToSend[High_Address_Byte] = newConfiguration.addressHighByte;
    messageToSend[Low_Address_Byte] = newConfiguration.addressLowByte;
    messageToSend[NETID_Byte] = newConfiguration.NETID;

    setComponentsToREG0(messageToSend[REG0_Byte], newConfiguration);
    setComponentsToREG1(messageToSend[REG1_Byte], newConfiguration);

    messageToSend[Channel_Byte] = newConfiguration.Channel;

    setComponentsToREG3(messageToSend[REG3_Byte], newConfiguration);

    setupForConfiguration();

    const auto status{ serialToLoRa_m.writeCrudeMessage(std::vector<uint8_t>(messageToSend.cbegin(), messageToSend.cend())) };
    if (!status) {
        restorePreviousValues();
        return tl::make_unexpected(std::make_shared<SerialParser::MessageSizeMissmatch>(status.error()));
    }

    const auto response{ serialToLoRa_m.readMessage(Prefix_Length) };

    restorePreviousValues();

    if (!response) {
        return tl::make_unexpected(std::make_shared<SerialParser::MessageSizeMissmatch>(response.error()));
    }

    if (!isValidWriteResponse(std::vector<uint8_t>(messageToSend.cbegin(), messageToSend.cend()), response.value())) {
        return tl::make_unexpected(std::make_shared<AbnormalResponse>(response.value()));
    }

    return {};
}

tl::expected<void, std::shared_ptr<std::exception>> E22_400T37S_Configurator::setDefaultCofiguration() {
    return setConfiguration(Default_Settings);
}

tl::expected<ModuleConfiguration, std::shared_ptr<std::exception>> E22_400T37S_Configurator::getConfiguration() {
    setupForConfiguration();
    
    const auto status{ serialToLoRa_m.writeCrudeMessage(std::vector<uint8_t>(Read_All_Configurations_Command.cbegin(), Read_All_Configurations_Command.cend())) };

    if (!status) {
        restorePreviousValues();
        return tl::make_unexpected(std::make_shared<SerialParser::MessageSizeMissmatch>(status.error()));
    }

    const auto message{ serialToLoRa_m.readMessage(Prefix_Length) };

    restorePreviousValues();

    if (!message) {
        return tl::make_unexpected(std::make_shared<SerialParser::MessageSizeMissmatch>(message.error()));
    }

    const auto& response{ message.value() };

    if (!isValidReadResponse(response)) {
        return tl::make_unexpected(std::make_shared<AbnormalResponse>(response));
    }

    if (!isValidChannel(response[Channel_Byte])) {
        return tl::make_unexpected(std::make_shared<AbnormalRegister>("Channel", response[Channel_Byte]));
    }

    ModuleConfiguration configuration;

    configuration.addressHighByte = response[High_Address_Byte];
    configuration.addressLowByte = response[Low_Address_Byte];
    configuration.NETID = response[NETID_Byte];

    setComponentsFromREG0(configuration, response[REG0_Byte]);

    setComponentsFromREG1(configuration, response[REG1_Byte]);
    
    configuration.Channel = response[Channel_Byte];

    setComponentsFromREG3(configuration, response[REG3_Byte]);

    return configuration;
}

void E22_400T37S_Configurator::setTransparentMode() {
    m0_pin_m.write(false);
    m1_pin_m.write(false);
}

void E22_400T37S_Configurator::setWORMode() {
    m0_pin_m.write(true);
    m1_pin_m.write(false);
}

void E22_400T37S_Configurator::setConfigurationMode() {
    m0_pin_m.write(false);
    m1_pin_m.write(true);
}

void E22_400T37S_Configurator::setSleepMode() {
    m0_pin_m.write(true);
    m1_pin_m.write(true);
}

void E22_400T37S_Configurator::setComponentsFromREG0(ModuleConfiguration &configuration, uint8_t REG0) {
    configuration.uartBaudRate = getBaudRateFromREG0(REG0);
    configuration.serialPortParityByte = getParityByteFromREG0(REG0);
    configuration.airDataRate = getAirDataRateFromREG0(REG0);
}

void E22_400T37S_Configurator::setComponentsFromREG1(ModuleConfiguration &configuration, uint8_t REG1) {
    configuration.subpacketLenght = getSubpacketLenghFromREG1(REG1);
    configuration.RSSIByte = getRSSINoiseFromREG1(REG1);
    configuration.enableAbnormalLog = getAbnormalLogEnabledFromREG1(REG1);
}

void E22_400T37S_Configurator::setComponentsFromREG3(ModuleConfiguration &configuration, uint8_t REG3) {
    configuration.enableRSSI = getRSSIEnabledFromREG3(REG3);
    configuration.enableFixedTransmitionMode = getTransmissionMethodFromREG3(REG3);
    configuration.enableRepeaterMode = getRelayFunctionREG3(REG3);
    configuration.enableLBT = getLBTEnabledFromREG3(REG3);
    configuration.enableWORMode = getWORModeFromREG3(REG3);
    configuration.worCycle = getWORCycleFromREG3(REG3);
}

void E22_400T37S_Configurator::setComponentsToREG0(uint8_t &REG0, const ModuleConfiguration &configuration) {
    REG0 = 0;
    REG0 |= static_cast<uint8_t>(configuration.uartBaudRate) << UART_Baud_Rate_Shift;
    REG0 |= static_cast<uint8_t>(configuration.serialPortParityByte) << Parity_Byte_Shift;
    REG0 |= static_cast<uint8_t>(configuration.airDataRate) & Air_Data_Rate_Mask;
}

void E22_400T37S_Configurator::setComponentsToREG1(uint8_t &REG1, const ModuleConfiguration &configuration) {
    REG1 = 0;
    REG1 |= static_cast<uint8_t>(configuration.subpacketLenght) << Subpacket_Length_Shift;
    REG1 |= configuration.RSSIByte << RSSI_Byte_Shift;
    REG1 |= configuration.enableAbnormalLog << Abnormal_Log_Shift;
}

void E22_400T37S_Configurator::setComponentsToREG3(uint8_t &REG3, const ModuleConfiguration &configuration) {
    REG3 = 0;
    REG3 |= configuration.enableRSSI << RSSI_Enabled_Shift;
    REG3 |= configuration.enableFixedTransmitionMode << Transmission_Method_Shift;
    REG3 |= configuration.enableRepeaterMode << Relay_Mode_Shift;
    REG3 |= configuration.enableLBT << LBT_Enabled_Shift;
    REG3 |= configuration.enableWORMode << WOR_Enabled_Shift;
    REG3 |= static_cast<uint8_t>(configuration.worCycle) & WOR_Cycle_Mask;
}

UARTBaudRate E22_400T37S_Configurator::getBaudRateFromREG0(uint8_t REG0) {
    const auto value{ static_cast<uint8_t>(REG0) >> UART_Baud_Rate_Shift };

    return static_cast<UARTBaudRate>(value);
}

SerialPortParityByte E22_400T37S_Configurator::getParityByteFromREG0(uint8_t REG0) {
    const auto value{ (static_cast<uint8_t>(REG0) >> Parity_Byte_Shift) & Parity_Byte_Mask };

    return static_cast<SerialPortParityByte>(value);
}

AirDataRate E22_400T37S_Configurator::getAirDataRateFromREG0(uint8_t REG0) {
    const auto value{ static_cast<uint8_t>(REG0) & Air_Data_Rate_Mask };

    return static_cast<AirDataRate>(value);
}

SubpacketLenght E22_400T37S_Configurator::getSubpacketLenghFromREG1(uint8_t REG1) {
    const auto value{ static_cast<uint8_t>(REG1) >> Subpacket_Length_Shift };

    return static_cast<SubpacketLenght>(value);
}

bool E22_400T37S_Configurator::getRSSINoiseFromREG1(uint8_t REG1) {
    return (REG1 >> RSSI_Byte_Shift) & Single_Bit_Mask;
}

bool E22_400T37S_Configurator::getAbnormalLogEnabledFromREG1(uint8_t REG1) {
    return (REG1 >> Abnormal_Log_Shift) & Single_Bit_Mask;
}

bool E22_400T37S_Configurator::getRSSIEnabledFromREG3(uint8_t REG3) {
    return (REG3 >> RSSI_Enabled_Shift) & Single_Bit_Mask;
}

bool E22_400T37S_Configurator::getTransmissionMethodFromREG3(uint8_t REG3) {
    return (REG3 >> Transmission_Method_Shift) & Single_Bit_Mask;
}

bool E22_400T37S_Configurator::getRelayFunctionREG3(uint8_t REG3) {
    return (REG3 >> Relay_Mode_Shift) & Single_Bit_Mask;
}

bool E22_400T37S_Configurator::getLBTEnabledFromREG3(uint8_t REG3) {
    return (REG3 >> LBT_Enabled_Shift) & Single_Bit_Mask;
}

bool E22_400T37S_Configurator::getWORModeFromREG3(uint8_t REG3) {
    return (REG3 >> WOR_Enabled_Shift) & Single_Bit_Mask;
}

WORCycle E22_400T37S_Configurator::getWORCycleFromREG3(uint8_t REG3) {
    const auto value{ static_cast<uint8_t>(REG3) & WOR_Cycle_Mask };

    return static_cast<WORCycle>(value);
}

void E22_400T37S_Configurator::setupForConfiguration() {
    previousMode_m = currentMode_m;
    setMode(Modes::Configuration);

    previousBaudRate_m = serialToLoRa_m.getBaudRate();
    serialToLoRa_m.setBaudRate(Baud_Rate_For_Configuration);
}

void E22_400T37S_Configurator::restorePreviousValues() {
    setMode(previousMode_m);
    serialToLoRa_m.setBaudRate(previousBaudRate_m);
}

bool E22_400T37S_Configurator::isValidChannel(uint8_t channel) {
    return channel <= Max_Channel;
}

bool E22_400T37S_Configurator::isValidWriteResponse(const std::vector<uint8_t> &messageSent, const std::vector<uint8_t> &responseReceived) {
    return (responseReceived.size() == messageSent.size()) && (responseReceived[0] == Write_Read_Response_Prefix) &&
        std::equal(responseReceived.cbegin() + 1, responseReceived.cend(), messageSent.cbegin() + 1);
}

bool E22_400T37S_Configurator::isValidReadResponse(const std::vector<uint8_t> &responseReceived) {
    return (responseReceived.size() == Expected_Read_Configuration_Response_Size) && std::equal(Read_All_Configurations_Command.cbegin(), Read_All_Configurations_Command.cend(), responseReceived.cbegin());
}
