#include "SerialParser.hpp"

SerialParser::SerialParser(arduino::HardwareSerial& serialUSB, uint32_t baudRate)
    : serial_m{ serialUSB }
    , baudRate_m{ baudRate } {
}

void SerialParser::begin() {
    serial_m.begin(baudRate_m);
}

void SerialParser::setBaudRate(uint32_t baudRate) {
    baudRate_m = baudRate;
    begin();
}

uint32_t SerialParser::getBaudRate() const {
    return baudRate_m;
}

tl::expected<std::vector<uint8_t>, std::shared_ptr<std::exception>> SerialParser::readMessage(uint8_t prefixLength) {
    std::vector<uint8_t> buffer(prefixLength + Message_Length_Byte_Size);
    
    auto readStatus{ tryReadBytes(buffer.begin(), buffer.size(), true) };
    if (!readStatus) {
        return tl::make_unexpected(readStatus.error());
    }

    if (!readStatus.value()) {
        return {};
    }

    const auto incomingMessageSize{ buffer[prefixLength] };
    buffer.resize(prefixLength + Message_Length_Byte_Size + incomingMessageSize);
    
    readStatus = tryReadBytes(buffer.begin() + prefixLength + Message_Length_Byte_Size, incomingMessageSize);
    if (!readStatus) {
        return tl::make_unexpected(readStatus.error());
    }

    return buffer;
}

tl::expected<void, std::shared_ptr<std::exception>> SerialParser::writeMessage(const std::vector<uint8_t>& message, const std::vector<uint8_t>& prefix) {
    if (message.empty() || !serial_m) {
        return {};
    }

    std::vector<uint8_t> buffer;
    buffer.reserve(prefix.size() + Message_Length_Byte_Size + message.size());
    buffer.insert(buffer.cend(), prefix.cbegin(), prefix.cend());
    buffer.push_back(static_cast<uint8_t>(message.size()));
    buffer.insert(buffer.cend(), message.cbegin(), message.cend());

    const auto result{ tryWriteBytes(buffer) };
    if (!result) {
        return tl::make_unexpected(result.error());
    }
    
    return {};
}

tl::expected<void, std::shared_ptr<std::exception>> SerialParser::writeString(std::string_view message, std::string_view prefix) {
    return writeMessage(std::vector<uint8_t>(message.cbegin(), message.cend()), std::vector<uint8_t>(prefix.cbegin(), prefix.cend()));
}

tl::expected<void, std::shared_ptr<std::exception>> SerialParser::writeCrudeMessage(const std::vector<uint8_t> &message) {
    return tryWriteBytes(message);
}

tl::expected<bool, std::shared_ptr<std::exception>> SerialParser::tryReadBytes(std::vector<uint8_t>::iterator buffer, size_t expectedBytes, bool emptyReadIsValid) {
    const auto bytesRead{ serial_m.readBytes(&*buffer, expectedBytes) };

    if (emptyReadIsValid && bytesRead == 0) {
        return false;
    }
    
    if (expectedBytes != bytesRead) {
        return tl::make_unexpected(std::make_shared<MessageSizeMissmatch>(expectedBytes, bytesRead, true));
    }

    return true;
}

tl::expected<void, std::shared_ptr<std::exception>> SerialParser::tryWriteBytes(const std::vector<uint8_t> &buffer) {
    const auto bytesWritten{ serial_m.write(buffer.data(), buffer.size()) };

    if (bytesWritten != buffer.size()) {
        return tl::make_unexpected(std::make_shared<MessageSizeMissmatch>(buffer.size(), bytesWritten, false));
    }

    return {};
}

void SerialParser::flush() {
    serial_m.flush();
}
