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

std::optional<std::vector<uint8_t>> SerialParser::readMessage(uint8_t prefixLength) {
    if (!USBSerial_m.available()) {
        return std::nullopt;
    }

    std::vector<uint8_t> buffer(prefixLength + MessageLengthByteSize);
    tryReadBytes(buffer.begin(), buffer.size());

    const auto incomingMessageSize{ buffer[prefixLength] };
    buffer.resize(prefixLength + MessageLengthByteSize + incomingMessageSize);
    tryReadBytes(buffer.begin() + prefixLength + MessageLengthByteSize, incomingMessageSize);

    return buffer;
}

void SerialParser::writeMessage(const std::vector<uint8_t>& message, const std::vector<uint8_t>& prefix = {}) {
    if (message.empty()) {
        return;
    }

    if (!USBSerial_m.availableForWrite()) {
        throw NoSpaceAvailableForWrite{};
    }

    std::vector<uint8_t> buffer;
    buffer.reserve(prefix.size() + MessageLengthByteSize + message.size());
    buffer.insert(buffer.cbegin(), prefix.cbegin(), prefix.cend());
    buffer.push_back(message.size());
    buffer.insert(buffer.cend(), message.cbegin(), message.cend());
    tryWriteBytes(buffer);
}

void SerialParser::writeString(std::string_view message, std::string_view prefix) {
    writeMessage(std::vector<uint8_t>(message.cbegin(), message.cend()), std::vector<uint8_t>(prefix.cbegin(), prefix.cend()));
}

void SerialParser::tryReadBytes(std::vector<uint8_t>::iterator buffer, size_t expectedBytes) {
    const auto bytesRead{ USBSerial_m.readBytes(&*buffer, expectedBytes) };

    if (expectedBytes != bytesRead) {
        throw MessageSizeMissmatch{ expectedBytes, bytesRead, true };
    }
}

void SerialParser::tryWriteBytes(const std::vector<uint8_t> &buffer) {
    const auto bytesWritten{ USBSerial_m.write(buffer.data(), buffer.size()) };

    if (bytesWritten != buffer.size()) {
        throw MessageSizeMissmatch(buffer.size(), bytesWritten, false);
    }
}
