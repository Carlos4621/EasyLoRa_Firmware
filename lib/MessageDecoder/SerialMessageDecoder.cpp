#include "SerialMessageDecoder.hpp"

SerialMessage SerialMessageDecoder::decode(const std::vector<uint8_t>& encodedData) {
    SerialMessage decodedData = SerialMessage_init_zero;

    pb_istream_t decodeStream = pb_istream_from_buffer(encodedData.data(), encodedData.size());

    const bool decodeStatus{ pb_decode(&decodeStream, SerialMessage_fields, &decodedData) };

    if(!decodeStatus) {
        throw std::runtime_error{ "Unable to decode the data" };
    }

    return decodedData;
}