#ifndef MESSAGE_ENCODER_HPP
#define MESSAGE_ENCODER_HPP

#include <pb_encode.h>
#include <stdexcept>
#include <Arduino.h>
#include "Envelope.pb.h"
#include "SuccessStatus.pb.h"
#include <vector>
#include <tl/expected.hpp>
#include <memory>
#include "MessageTraits.hpp"

/// @brief Codificador genérico de mensajes Protocol Buffers
template<typename MessageType>
class MessageEncoder {
public:
    class EncodificationError;

    /// @brief Codifica un mensaje del tipo especificado en una serie de bytes
    /// @param message Mensaje a codificar
    /// @return tl::expected con data codificada, EncodificationError en caso de error
    [[nodiscard]]
    static tl::expected<std::vector<uint8_t>, EncodificationError> encode(const MessageType& message);
};

template<typename MessageType>
class MessageEncoder<MessageType>::EncodificationError : public std::exception {
public:
    [[nodiscard]]
    const char* what() const noexcept override {
        return "Unable to encode the data";
    }
};

template<typename MessageType>
tl::expected<std::vector<uint8_t>, typename MessageEncoder<MessageType>::EncodificationError> 
MessageEncoder<MessageType>::encode(const MessageType& message) {

    std::vector<uint8_t> encodedData(MessageTraits<MessageType>::max_size);

    pb_ostream_t encodeStream = pb_ostream_from_buffer(encodedData.data(), encodedData.size());

    const auto encodeStatus = pb_encode(&encodeStream, MessageTraits<MessageType>::fields, &message);
    
    if (!encodeStatus) {
        return tl::make_unexpected(EncodificationError{});
    }

    encodedData.resize(encodeStream.bytes_written);
    
    return encodedData;
}

#endif // !MESSAGE_ENCODER_HPP