#ifndef ENVELOPE_DECODER_HEADER
#define ENVELOPE_DECODER_HEADER

#include <pb_decode.h>
#include <stdexcept>
#include <Arduino.h>
#include "Envelope.pb.h"
#include <vector>

/// @brief Decodificador de SerialMessage
class EnvelopeDecoder {
public:

    /// @brief Decodifica una serie de bytes en un SerialMessage
    /// @param encodedData Data a decodificar
    /// @return Datos decodificados
    [[nodiscard]]
    static Envelope decode(const std::vector<uint8_t>& encodedData);
};

class DecodificationError : public std::exception {
public:

    const char* what() const noexcept override {
        return "Unable to decode the data";
    }
};

#endif // !ENVELOPE_DECODER_HEADER