#ifndef ENVELOPE_FACTORY_HEADER
#define ENVELOPE_FACTORY_HEADER

#include <pb.h>
#include "Envelope.pb.h"
#include <string_view>
#include <utility>
#include <vector>

class EnvelopeFactory {
public:

    [[nodiscard]]
    static Envelope withACK();

    [[nodiscard]]
    static Envelope withModuleConfiguration(const ModuleConfiguration& configuration);

    [[nodiscard]]
    static Envelope withError(std::string_view error);

    [[nodiscard]]
    static Envelope withData(const std::vector<uint8_t>& data);

private:

    template<typename T>
    static void setBytesField(T& bytesField, const std::vector<uint8_t>& data);
};

template <typename T>
inline void EnvelopeFactory::setBytesField(T &bytesField, const std::vector<uint8_t> &data) {
    const auto copySize{ std::min(sizeof(bytesField.bytes), data.size()) };
    memcpy(reinterpret_cast<uint8_t*>(bytesField.bytes), data.data(), copySize);

    bytesField.size = copySize;
}

#endif // !ENVELOPE_FACTORY_HEADER