#ifndef MESSAGE_DECODER_HEADER
#define MESSAGE_DECODER_HEADER

#include <pb_decode.h>
#include <stdexcept>
#include <Arduino.h>
#include "Envelope.pb.h"
#include "SuccessStatus.pb.h"
#include <vector>
#include <tl/expected.hpp>
#include <memory>

/// @brief Traits para tipos de mensajes Protocol Buffers
template<typename T>
struct MessageTraits;

/// @brief Especialización para Envelope
template<>
struct MessageTraits<Envelope> {
    static constexpr Envelope init_zero = Envelope_init_zero;
    static constexpr auto fields = Envelope_fields;
};

/// @brief Especialización para SuccessStatus
template<>
struct MessageTraits<SuccessStatus> {
    static constexpr SuccessStatus init_zero = SuccessStatus_init_zero;
    static constexpr auto fields = SuccessStatus_fields;
};

/// @brief Decodificador genérico de mensajes Protocol Buffers
template<typename MessageType>
class MessageDecoder {
public:
    class DecodificationError;

    /// @brief Decodifica una serie de bytes en un mensaje del tipo especificado
    /// @param encodedData Data a decodificar
    /// @return tl::expected con data decodificada, DecodificationError en caso de error
    [[nodiscard]]
    static tl::expected<MessageType, DecodificationError> decode(const std::vector<uint8_t>& encodedData);
};

template<typename MessageType>
class MessageDecoder<MessageType>::DecodificationError : public std::exception {
public:
    [[nodiscard]]
    const char* what() const noexcept override {
        return "Unable to decode the data";
    }
};

template<typename MessageType>
tl::expected<MessageType, typename MessageDecoder<MessageType>::DecodificationError> 
MessageDecoder<MessageType>::decode(const std::vector<uint8_t>& encodedData) {
    MessageType decodedData = MessageTraits<MessageType>::init_zero;

    pb_istream_t decodeStream = pb_istream_from_buffer(encodedData.data(), encodedData.size());

    const auto decodeStatus{ pb_decode(&decodeStream, MessageTraits<MessageType>::fields, &decodedData) };

    if(!decodeStatus) {
        return tl::make_unexpected(DecodificationError{});
    }

    return decodedData;
}

#endif // !MESSAGE_DECODER_HEADER