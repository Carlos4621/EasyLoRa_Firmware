#include "ResponseSender.hpp"
#include <cstring>

ResponseSender::ResponseSender(SerialParser& serialParser)
    : serialParser_m(serialParser) {
}

bool ResponseSender::sendSuccess(const std::string& successMessage) {
    const auto message = createSuccessMessage(successMessage);
    return encodeAndSend(message);
}

bool ResponseSender::sendSuccess(const std::vector<uint8_t>& data) {
    const auto message = createSuccessMessage(data);
    return encodeAndSend(message);
}

bool ResponseSender::sendError(const std::string& errorMessage) {
    const auto message = createErrorMessage(errorMessage);
    return encodeAndSend(message);
}

SuccessStatus ResponseSender::createSuccessMessage(const std::string& message) {
    SuccessStatus toSend = SuccessStatus_init_zero;
    toSend.which_PossibleData = SuccessStatus_data_tag;

    const auto maxSize{ sizeof(toSend.PossibleData.data.bytes) };
    const auto copySize{ std::min(message.length(), maxSize) };

    std::memcpy(toSend.PossibleData.data.bytes, message.c_str(), copySize);
    toSend.PossibleData.data.size = copySize;
    
    return toSend;
}

SuccessStatus ResponseSender::createSuccessMessage(const std::vector<uint8_t>& data) {
    SuccessStatus toSend = SuccessStatus_init_zero;
    toSend.which_PossibleData = SuccessStatus_data_tag;

    const auto maxSize{ sizeof(toSend.PossibleData.data.bytes) };
    const auto copySize{ std::min(data.size(), maxSize) };

    std::memcpy(toSend.PossibleData.data.bytes, data.data(), copySize);
    toSend.PossibleData.data.size = copySize;
    
    return toSend;
}

SuccessStatus ResponseSender::createErrorMessage(const std::string& errorMessage) {
    SuccessStatus toSend = SuccessStatus_init_zero;
    toSend.which_PossibleData = SuccessStatus_error_tag;
    
    const auto maxSize{ sizeof(toSend.PossibleData.error.bytes) };
    const auto copySize{ std::min(errorMessage.length(), maxSize) };
    
    std::memcpy(toSend.PossibleData.error.bytes, errorMessage.c_str(), copySize);
    toSend.PossibleData.error.size = copySize;
    
    return toSend;
}

bool ResponseSender::encodeAndSend(const SuccessStatus& message) {
    const auto encodeStatus{ MessageEncoder<SuccessStatus>::encode(message) };
    if (!encodeStatus) {
        // Log del error de codificación - no podemos usar sendError aquí para evitar recursión
        return false;
    }
    
    const auto writeStatus = serialParser_m.writeCrudeMessage(encodeStatus.value());
    if (!writeStatus) {
        // Log del error de envío - no podemos usar sendError aquí para evitar recursión
        return false;
    }
    
    return true;
}
