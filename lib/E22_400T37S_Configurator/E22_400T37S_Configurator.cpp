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
}

void E22_400T37S_Configurator::setMode(Modes modeToSet) {
    if (modeToSet == currentMode_m) {
        return;
    }

    waitForAuxRaising();

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

    waitForAuxRaising();

    delay(ModeSwitchingDelayInMs);

    currentMode_m = modeToSet;
}

void E22_400T37S_Configurator::setConfiguration(ModuleConfiguration newConfiguration) {
    // Pasos: 1- armar mensaje 2- enviar mensaje 3- verificar respuesta

    std::array<uint8_t, 10> messageToSend;
}

tl::expected<ModuleConfiguration, std::shared_ptr<std::exception>> E22_400T37S_Configurator::getConfiguration() {
    setupForConfiguration();

    serialToLoRa_m.writeCrudeMessage(std::vector<uint8_t>(ReadAllConfigurationCommand.cbegin(), ReadAllConfigurationCommand.cend()));
    const auto message{ serialToLoRa_m.readMessage() };

    restorePreviousValues();

    if (!message.has_value()) {
        return tl::make_unexpected(std::make_shared<ResponseDontReceived>());
    }

    const auto& response{ message.value() };

    if (response.size() != ExpectedConfigurationResponseSize) {
        return tl::make_unexpected(std::make_shared<AbnormalResponse>(response));
    }

    if (!isValidChannel(response[Channel_Byte])) {
        return tl::make_unexpected(std::make_shared<AbnormalRegister>("Channel", response[Channel_Byte]));
    }

    ModuleConfiguration configuration;

    configuration.addressHighByte = response[HighAddress_Byte];
    configuration.addressLowByte = response[LowAddress_Byte];
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

void E22_400T37S_Configurator::waitForAuxRaising() {
    while(!auxPin_m.read()) {
        delay(1); // watchdog
    }
}

void E22_400T37S_Configurator::setComponentsFromREG0(ModuleConfiguration &configuration, uint8_t REG0) {
    configuration.uartBaudRate = getBaudRateFromREG0(REG0);
    configuration.serialPortParityByte = getParityByteFromREG0(REG0);
    configuration.airDataRate = getAirDataRateFromREG0(REG0);
}

void E22_400T37S_Configurator::setComponentsFromREG1(ModuleConfiguration &configuration, uint8_t REG1) {
    configuration.subpacketLenght = getSubpacketLenghFromREG1(REG1);
    configuration.enableRSSI = getRSSINoiseFromREG1(REG1);
    configuration.enableAbnormalLog = getAbnormalLogEnabledFromREG1(REG1);
}

void E22_400T37S_Configurator::setComponentsFromREG3(ModuleConfiguration &configuration, uint8_t REG3) {
    configuration.RSSIByte = getRSSIEnabledFromREG3(REG3);
    configuration.enableFixedTransmitionMode = getTransmissionMethodFromREG3(REG3);
    configuration.enableRepeaterMode = getRelayFunctionREG3(REG3);
    configuration.enableLBT = getLBTEnabledFromREG3(REG3);
    configuration.enableWORMode = getWORModeFromREG3(REG3);
    configuration.worCycle = getWORCycleFromREG3(REG3);
}

UARTBaudRate E22_400T37S_Configurator::getBaudRateFromREG0(uint8_t REG0) {
    const auto value{ static_cast<uint8_t>(REG0) >> 5 };

    return static_cast<UARTBaudRate>(value);
}

SerialPortParityByte E22_400T37S_Configurator::getParityByteFromREG0(uint8_t REG0) {
    const auto value{ (static_cast<uint8_t>(REG0) >> 3) & 0b11 };

    return static_cast<SerialPortParityByte>(value);
}

AirDataRate E22_400T37S_Configurator::getAirDataRateFromREG0(uint8_t REG0) {
    const auto value{ static_cast<uint8_t>(REG0) & 0b111 };

    return static_cast<AirDataRate>(value);
}

SubpacketLenght E22_400T37S_Configurator::getSubpacketLenghFromREG1(uint8_t REG1) {
    const auto value{ static_cast<uint8_t>(REG1) >> 6 };

    return static_cast<SubpacketLenght>(value);
}

bool E22_400T37S_Configurator::getRSSINoiseFromREG1(uint8_t REG1) noexcept {
    return (REG1 >> 5) & 1;
}

bool E22_400T37S_Configurator::getAbnormalLogEnabledFromREG1(uint8_t REG1) noexcept {
    return (REG1 >> 2) & 1;
}

bool E22_400T37S_Configurator::getRSSIEnabledFromREG3(uint8_t REG3) noexcept {
    return (REG3 >> 7);
}

bool E22_400T37S_Configurator::getTransmissionMethodFromREG3(uint8_t REG3) noexcept {
    return (REG3 >> 6) & 1;
}

bool E22_400T37S_Configurator::getRelayFunctionREG3(uint8_t REG3) noexcept {
    return (REG3 >> 5) & 1;
}

bool E22_400T37S_Configurator::getLBTEnabledFromREG3(uint8_t REG3) noexcept {
    return (REG3 >> 4) & 1;
}

bool E22_400T37S_Configurator::getWORModeFromREG3(uint8_t REG3) noexcept {
    return (REG3 >> 3) & 1;
}

WORCycle E22_400T37S_Configurator::getWORCycleFromREG3(uint8_t REG3) {
    const auto value{ static_cast<uint8_t>(REG3) & 0b111 };

    return static_cast<WORCycle>(value);
}

void E22_400T37S_Configurator::setupForConfiguration() {
    previousMode_m = currentMode_m;
    setMode(Modes::Configuration);

    previousBaudRate_m = serialToLoRa_m.getBaudRate();
    serialToLoRa_m.setBaudRate(BaudRateForConfiguration);
}

void E22_400T37S_Configurator::restorePreviousValues() {
    setMode(previousMode_m);
    serialToLoRa_m.setBaudRate(previousBaudRate_m);
}

bool E22_400T37S_Configurator::isValidChannel(uint8_t channel) {
    return channel <= MaxChannel;
}
