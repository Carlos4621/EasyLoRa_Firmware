#ifndef MESSAGE_TRAITS_HEADER
#define MESSAGE_TRAITS_HEADER

#include "Envelope.pb.h"
#include "ModuleConfiguration.pb.h"

/// @brief Traits para tipos de mensajes Protocol Buffers
template<typename T>
struct MessageTraits;

/// @brief Especialización para Envelope
template<>
struct MessageTraits<Envelope> {
    static constexpr Envelope init_zero = Envelope_init_zero;
    static constexpr auto fields = Envelope_fields;
    static constexpr size_t max_size = Envelope_size;
};

/// @brief Especialización para ModuleConfiguration
template<>
struct MessageTraits<ModuleConfiguration> {
    static constexpr ModuleConfiguration init_zero = ModuleConfiguration_init_zero;
    static constexpr auto fields = ModuleConfiguration_fields;
    static constexpr size_t max_size = ModuleConfiguration_size;
};

#endif // !MESSAGE_TRAITS_HPP