#ifndef SERIAL_MESSAGE_DECODER_HEADER
#define SERIAL_MESSAGE_DECODER_HEADER

#include <pb_decode.h>
#include <stdexcept>
#include <Arduino.h>
#include "SerialMessage.pb.h"
#include <vector>

/// @brief Decodificador de SerialMessage
class SerialMessageDecoder {
public:

    /// @brief Decodifica una serie de bytes en un SerialMessage
    /// @param encodedData Data a decodificar
    /// @return Datos decodificados
    [[nodiscard]]
    static SerialMessage decode(const std::vector<uint8_t>& encodedData);
};

#endif // !SERIAL_MESSAGE_DECODER_HEADER