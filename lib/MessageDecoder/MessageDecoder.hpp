#ifndef MESSAGE_DECODER_HEADER
#define MESSAGE_DECODER_HEADER

#include <pb_decode.h>
#include <tl/expected.hpp>
#include "EasyLoRa_Exceptions.hpp"
#include "MessageTraits.hpp"
#include <memory>

/// @brief Decodificador genérico de mensajes Protocol Buffers
template<typename MessageType>
class MessageDecoder {
public:

    /// @brief Decodifica una serie de bytes en un mensaje del tipo especificado
    /// @param encodedData Data a decodificar
    /// @return tl::expected con data decodificada, DecodificationError en caso de error
    [[nodiscard]]
    static tl::expected<MessageType, std::shared_ptr<std::exception>> decode(const std::vector<uint8_t>& encodedData);
};

template<typename MessageType>
tl::expected<MessageType, std::shared_ptr<std::exception>> MessageDecoder<MessageType>::decode(const std::vector<uint8_t>& encodedData) {
    MessageType decodedData{ MessageTraits<MessageType>::init_zero };

    pb_istream_t decodeStream{ pb_istream_from_buffer(encodedData.data(), encodedData.size()) };

    const auto decodeStatus{ pb_decode(&decodeStream, MessageTraits<MessageType>::fields, &decodedData) };

    if(!decodeStatus) {
        return tl::make_unexpected(std::make_shared<DecodificationError>());
    }

    return decodedData;
}

#endif // !MESSAGE_DECODER_HEADER