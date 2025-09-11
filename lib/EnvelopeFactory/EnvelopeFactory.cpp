#include "EnvelopeFactory.hpp"

Envelope EnvelopeFactory::withACK() {
    Envelope result = Envelope_init_zero;

    result.PosibleData.ACK = true;
    result.which_PosibleData = Envelope_ACK_tag;

    return result;
}

Envelope EnvelopeFactory::withModuleConfiguration(const ModuleConfiguration &configuration) {
    Envelope result = Envelope_init_zero;

    result.PosibleData.configuration = configuration;
    result.which_PosibleData = Envelope_configuration_tag;

    return result;
}

Envelope EnvelopeFactory::withError(std::string_view error) {
    Envelope result = Envelope_init_zero;

    setBytesField(result.PosibleData.error, std::vector<uint8_t>(error.cbegin(), error.cend()));

    result.which_PosibleData = Envelope_error_tag;

    return result;
}

Envelope EnvelopeFactory::withData(const std::vector<uint8_t> &data) {
    Envelope result = Envelope_init_zero;

    setBytesField(result.PosibleData.data, data);

    result.which_PosibleData = Envelope_data_tag;

    return result;
}
