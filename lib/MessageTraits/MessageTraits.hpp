#ifndef MESSAGE_TRAITS_HPP
#define MESSAGE_TRAITS_HPP

#include "Envelope.pb.h"
#include "SuccessStatus.pb.h"
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

/// @brief Especialización para SuccessStatus
template<>
struct MessageTraits<SuccessStatus> {
    static constexpr SuccessStatus init_zero = SuccessStatus_init_zero;
    static constexpr auto fields = SuccessStatus_fields;
    static constexpr size_t max_size = SuccessStatus_size;
};

/// @brief Especialización para ModuleConfiguration
template<>
struct MessageTraits<ModuleConfiguration> {
    static constexpr ModuleConfiguration init_zero = ModuleConfiguration_init_zero;
    static constexpr auto fields = ModuleConfiguration_fields;
    static constexpr size_t max_size = ModuleConfiguration_size;
};

#endif // !MESSAGE_TRAITS_HPP