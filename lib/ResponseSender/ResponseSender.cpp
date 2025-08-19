#include "ResponseSender.hpp"
#include <cstring>

ResponseSender::ResponseSender(SerialParser& serialParser)
    : serialParser_m(serialParser) {
}

bool ResponseSender::sendSuccess(const std::vector<uint8_t>& data) {
    const auto message{ createSuccessMessage(data) };
    return encodeAndSend(message);
}

bool ResponseSender::sendError(std::string_view errorMessage) {
    const auto message{ createErrorMessage(errorMessage) };
    return encodeAndSend(message);
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

// TODO: Se puede simplificar esto con el método de arriba?
SuccessStatus ResponseSender::createErrorMessage(std::string_view errorMessage) {
    SuccessStatus toSend = SuccessStatus_init_zero;
    toSend.which_PossibleData = SuccessStatus_error_tag; //
    
    const auto maxSize{ sizeof(toSend.PossibleData.error.bytes) }; //
    const auto copySize{ std::min(errorMessage.size(), maxSize) };
    
    std::memcpy(toSend.PossibleData.error.bytes, errorMessage.data(), copySize); //
    toSend.PossibleData.error.size = copySize; //
    
    return toSend;
}

bool ResponseSender::encodeAndSend(const SuccessStatus& message) {
    const auto encodeStatus{ MessageEncoder<SuccessStatus>::encode(message) };
    if (!encodeStatus) {
        // TODO: Log del error de codificación - no podemos usar sendError aquí para evitar recursión
        return false;
    }
    
    const auto writeStatus{ serialParser_m.writeMessage(encodeStatus.value()) };
    if (!writeStatus) {
        // TODO: Log del error de envío - no podemos usar sendError aquí para evitar recursión
        return false;
    }
    
    return true;
}
