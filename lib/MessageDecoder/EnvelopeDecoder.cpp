#include "EnvelopeDecoder.hpp"

tl::expected<Envelope, EnvelopeDecoder::DecodificationError> EnvelopeDecoder::decode(const std::vector<uint8_t>& encodedData) {
    Envelope decodedData = Envelope_init_zero;

    pb_istream_t decodeStream = pb_istream_from_buffer(encodedData.data(), encodedData.size());

    const bool decodeStatus{ pb_decode(&decodeStream, Envelope_fields, &decodedData) };

    if(!decodeStatus) {
        return tl::make_unexpected(DecodificationError{});
    }

    return decodedData;
}