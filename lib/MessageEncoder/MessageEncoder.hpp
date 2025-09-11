#ifndef MESSAGE_ENCODER_HEADER
#define MESSAGE_ENCODER_HEADER

#include <pb_encode.h>
#include <vector>
#include <tl/expected.hpp>
#include "EasyLoRa_Exceptions.hpp"
#include "MessageTraits.hpp"

/// @brief Codificador genérico de mensajes Protocol Buffers
template<typename MessageType>
class MessageEncoder {
public:

    /// @brief Codifica un mensaje del tipo especificado en una serie de bytes
    /// @param message Mensaje a codificar
    /// @return tl::expected con data codificada, EncodificationError en caso de error
    [[nodiscard]]
    static tl::expected<std::vector<uint8_t>, std::shared_ptr<std::exception>> encode(const MessageType& message);
};

template<typename MessageType>
tl::expected<std::vector<uint8_t>, std::shared_ptr<std::exception>> MessageEncoder<MessageType>::encode(const MessageType& message) {
    std::vector<uint8_t> encodedData(MessageTraits<MessageType>::max_size);

    pb_ostream_t encodeStream{ pb_ostream_from_buffer(encodedData.data(), encodedData.size()) };

    const auto encodeStatus{ pb_encode(&encodeStream, MessageTraits<MessageType>::fields, &message) };
    
    if (!encodeStatus) {
        return tl::make_unexpected(std::make_shared<EncodificationError>());
    }

    encodedData.resize(encodeStream.bytes_written);
    
    return encodedData;
}

#endif // !MESSAGE_ENCODER_HEADER