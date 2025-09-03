#ifndef RESPONSE_SENDER_HEADER
#define RESPONSE_SENDER_HEADER

#include <string_view>
#include <vector>
#include "SerialParser.hpp"
#include "MessageEncoder.hpp"
#include "SuccessStatus.pb.h"
#include <tl/expected.hpp>

/// @brief Clase responsable del envío de respuestas de éxito y error por puerto serial
class ResponseSender {
public:
    /// @brief Constructor
    /// @param serialParser Referencia al parser serial para envío de mensajes
    explicit ResponseSender(SerialParser& serialParser);

    /// @brief Envía un mensaje de éxito con datos binarios
    /// @param data Datos binarios a enviar
    /// @return tl::expected con EncodificationError en caso de error de codificación, MessageSizeMissmatch en caso de no poder escribir el mensaje completo
    [[nodiscard]]
    tl::expected<void, std::shared_ptr<std::exception>> sendSuccess(const std::vector<uint8_t>& data);

    /// @brief Envía un mensaje de error
    /// @param errorMessage Mensaje de error a enviar
    /// @return tl::expected con EncodificationError en caso de error de codificación, MessageSizeMissmatch en caso de no poder escribir el mensaje completo
    [[nodiscard]]
    tl::expected<void, std::shared_ptr<std::exception>> sendError(std::string_view errorMessage);

private:
    SerialParser& serialParser_m;

    [[nodiscard]]
    SuccessStatus createSuccessMessage(const std::vector<uint8_t>& data);

    [[nodiscard]]
    SuccessStatus createErrorMessage(std::string_view errorMessage);

    [[nodiscard]]
    tl::expected<void, std::shared_ptr<std::exception>> encodeAndSend(const SuccessStatus& message);
};

#endif // !RESPONSE_SENDER_HEADER
