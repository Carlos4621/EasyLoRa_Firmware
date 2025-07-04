#ifndef ENVELOPE_DECODER_HEADER
#define ENVELOPE_DECODER_HEADER

#include <pb_decode.h>
#include <stdexcept>
#include <Arduino.h>
#include "Envelope.pb.h"
#include <vector>
#include <tl/expected.hpp>
#include <memory>

/// @brief Decodificador de SerialMessage
class EnvelopeDecoder {
public:

    /// @brief Decodifica una serie de bytes en un SerialMessage
    /// @param encodedData Data a decodificar
    /// @return tl::expected con data decodificada, DecodificationError en caso de error de decodificación
    [[nodiscard]]
    static tl::expected<Envelope, std::shared_ptr<std::exception>> decode(const std::vector<uint8_t>& encodedData);
};

class DecodificationError : public std::exception {
public:

    const char* what() const noexcept override {
        return "Unable to decode the data";
    }
};

#endif // !ENVELOPE_DECODER_HEADER