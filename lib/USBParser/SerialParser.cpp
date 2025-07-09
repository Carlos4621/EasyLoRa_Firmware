#include "SerialParser.hpp"

SerialParser::SerialParser(arduino::HardwareSerial& serialUSB, uint32_t baudRate)
    : USBSerial_m{ serialUSB }
    , baudRate_m{ baudRate } {
}

void SerialParser::begin() {
    USBSerial_m.begin(baudRate_m);
}

void SerialParser::setBaudRate(uint32_t baudRate) {
    baudRate_m = baudRate;
    USBSerial_m.begin(baudRate_m);
}

uint32_t SerialParser::getBaudRate() const noexcept {
    return baudRate_m;
}

tl::expected<std::vector<uint8_t>, SerialParser::MessageSizeMissmatch> SerialParser::readMessage(uint8_t prefixLength) {
    std::vector<uint8_t> buffer(prefixLength + MessageLengthByteSize);
    
    auto result{ tryReadBytes(buffer.begin(), buffer.size()) };
    if (!result) {
        return tl::make_unexpected(result.error());
    }

    const auto incomingMessageSize{ buffer[prefixLength] };
    buffer.resize(prefixLength + MessageLengthByteSize + incomingMessageSize);
    
    result = tryReadBytes(buffer.begin() + prefixLength + MessageLengthByteSize, incomingMessageSize);
    if (!result) {
        return tl::make_unexpected(result.error());
    }

    return buffer;
}

tl::expected<void, SerialParser::MessageSizeMissmatch> SerialParser::writeMessage(const std::vector<uint8_t>& message, const std::vector<uint8_t>& prefix) {
    if (message.empty()) {
        return {};
    }

    std::vector<uint8_t> buffer;
    buffer.reserve(prefix.size() + MessageLengthByteSize + message.size());
    buffer.insert(buffer.cbegin(), prefix.cbegin(), prefix.cend());
    buffer.push_back(message.size());
    buffer.insert(buffer.cend(), message.cbegin(), message.cend());

    const auto result{ tryWriteBytes(buffer) };
    if (!result) {
        return tl::make_unexpected(result.error());
    }
    
    return {};
}

tl::expected<void, SerialParser::MessageSizeMissmatch> SerialParser::writeString(std::string_view message, std::string_view prefix) {
    return writeMessage(std::vector<uint8_t>(message.cbegin(), message.cend()), std::vector<uint8_t>(prefix.cbegin(), prefix.cend()));
}

tl::expected<void, SerialParser::MessageSizeMissmatch> SerialParser::writeCrudeMessage(const std::vector<uint8_t> &message) {
    return tryWriteBytes(message);
}

tl::expected<void, SerialParser::MessageSizeMissmatch> SerialParser::tryReadBytes(std::vector<uint8_t>::iterator buffer, size_t expectedBytes) {
    const auto bytesRead{ USBSerial_m.readBytes(&*buffer, expectedBytes) };

    if (expectedBytes != bytesRead) {
        return tl::make_unexpected(MessageSizeMissmatch{expectedBytes, bytesRead, true});
    }

    return {};
}

tl::expected<void, SerialParser::MessageSizeMissmatch> SerialParser::tryWriteBytes(const std::vector<uint8_t> &buffer) {
    const auto bytesWritten{ USBSerial_m.write(buffer.data(), buffer.size()) };

    if (bytesWritten != buffer.size()) {
        return tl::make_unexpected(MessageSizeMissmatch{buffer.size(), bytesWritten, false});
    }

    return {};
}
