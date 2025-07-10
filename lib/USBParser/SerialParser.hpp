#ifndef SERIAL_PARSER_HEADER
#define SERIAL_PARSER_HEADER

#include <Arduino.h>
#include <string>
#include <vector>
#include <stdexcept>
#include "tl/expected.hpp"
#include <memory>

/// @brief Clase encargada del formateo de mensajes enviados y recibidos desde un puerto USB
class SerialParser {
public:

    class MessageSizeMissmatch;

    /// @brief Constructor base
    /// @param serialUSB Puerto en donde se enviarán y recibirán paquetes donde el primer byte el tamaño del mensaje. Opcionalmente se puede incluir un prefix
    /// @param baudRate Velocidad de baudios para la comunicación USB, por defecto 115200
    explicit SerialParser(arduino::HardwareSerial& serialUSB, uint32_t baudRate = 115200);

    /// @brief Inicializa el dispositivo, debe ser llamado antes de cualquier otra función.
    void begin();

    /// @brief Cambia el baud rate de la comunicación USB.
    /// @param baudRate Velocidad de baudios para la comunicación USB
    void setBaudRate(uint32_t baudRate);

    /// @brief Obtiene el baud rate
    /// @return El baudRate actual
    [[nodiscard]]
    uint32_t getBaudRate() const;

    /// @brief Intenta leer un mensaje del puerto USB
    /// @param prefixLength Longitud del prefix
    /// @return tl::expected con std::vector<uint8_t> que contiene el mensaje leído si se pudo leer uno,
    /// MessageSizeMissmatch si el byte de longitud del mensaje es incorrecto
    [[nodiscard]]
    tl::expected<std::vector<uint8_t>, MessageSizeMissmatch> readMessage(uint8_t prefixLength = 0);

    /// @brief Escribe mensajes en el puerto SerialUSB.
    /// @param message Mensaje a enviar, el primer byte (si es que no hay prefix) serán el tamaño del mensaje.
    /// @param prefix Prefix a mandar junto al mensaje
    /// @return tl::excepted con MessageSizeMissmatch si el byte de longitud del mensaje es incorrecto
    /// @example 0xAA 0xBB => 0x02 0xAA 0xBB 
    [[nodiscard]]
    tl::expected<void, MessageSizeMissmatch> writeMessage(const std::vector<uint8_t>& message, const std::vector<uint8_t>& prefix = {});

    /// @brief Método de conveniencia para strings, sigue la misma lógica que writeMessage
    /// @param message Mensaje a enviar, el primer byte (si es que no hay prefix) serán el tamaño del mensaje.
    /// @param prefix Prefix a mandar junto al mensaje
    /// @return tl::expected con MessageSizeMissmatch si el byte de longitud del mensaje es incorrecto
    [[nodiscard]]
    tl::expected<void, MessageSizeMissmatch> writeString(std::string_view message, std::string_view prefix = "");

    /// @brief Escribe el mensaje tal como está, no se agrega prefix ni tamaño del mensaje
    /// @param message Mensaje a enviar
    /// @return tl::expected con MessageSizeMissmatch si el byte de longitud del mensaje es incorrecto
    [[nodiscard]]
    tl::expected<void, MessageSizeMissmatch> writeCrudeMessage(const std::vector<uint8_t>& message);

private:

    static constexpr uint8_t MessageLengthByteSize{ 1 };

    arduino::HardwareSerial& USBSerial_m;
    uint32_t baudRate_m;

    [[nodiscard]]
    tl::expected<void, MessageSizeMissmatch> tryReadBytes(std::vector<uint8_t>::iterator buffer, size_t expectedBytes);
    
    [[nodiscard]]
    tl::expected<void, MessageSizeMissmatch> tryWriteBytes(const std::vector<uint8_t>& buffer);
};

class SerialParser::MessageSizeMissmatch : public std::exception {
public:

    MessageSizeMissmatch(size_t sizeExpected, size_t sizeReceived, bool onRead) 
    : errorMessage_m{
        std::string{"Tamaño del mensaje incorrecto. Esperado: "} + 
        std::to_string(sizeExpected) + 
        (onRead ? " Recibido: " : " Escrito: ") + 
        std::to_string(sizeReceived)
    } {}

    [[nodiscard]]
    const char* what() const noexcept override {
        return errorMessage_m.c_str();
    }

private:

    std::string errorMessage_m;
};

#endif // !SERIAL_PARSER_HEADER