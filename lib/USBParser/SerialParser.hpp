#ifndef SERIAL_PARSER_HEADER
#define SERIAL_PARSER_HEADER

#include <Arduino.h>
#include <optional>
#include <string>
#include <vector>
#include <stdexcept>

/// @brief Clase encargada del formateo de mensajes enviados y recibidos desde un puerto USB
class SerialParser {
public:

    /// @brief Constructor base
    /// @param serialUSB Puerto en donde se enviarán y recibirán paquetes donde el primer byte el tamaño del mensaje. Opcionalmente se puede incluir un prefix
    /// @param baudRate Velocidad de baudios para la comunicación USB, por defecto 115200
    explicit SerialParser(arduino::HardwareSerial& serialUSB, uint32_t baudRate = 115200);

    /// @brief Inicializa el dispositivo, debe ser llamado antes de cualquier otra función.
    void begin();

    /// @brief Cambia el baud rate de la comunicación USB.
    /// @param baudRate Velocidad de baudios para la comunicación USB
    void setBaudRate(uint32_t baudRate);

    /// @brief Intenta leer un mensaje del puerto USB
    /// @param prefixLength Longitud del prefix
    /// @return std::optional con std::vector<uint8_t> que contiene el mensaje leído si se pudo leer uno, o std::nullopt si no hay mensajes disponibles.
    [[nodiscard]]
    std::optional<std::vector<uint8_t>> readMessage(uint8_t prefixLength = 0);

    /// @brief Escribe mensajes en el puerto SerialUSB.
    /// @param message Mensaje a enviar, el primer byte (si es que no hay prefix) serán el tamaño del mensaje.
    /// @param prefix Prefix a mandar junto al mensaje
    /// @example 0xAA 0xBB => 0x02 0xAA 0xBB 
    void writeMessage(const std::vector<uint8_t>& message, const std::vector<uint8_t>& prefix = {});

    /// @brief Método de conveniencia para strings, sigue la misma lógica que writeMessage
    /// @param message Mensaje a enviar, el primer byte (si es que no hay prefix) serán el tamaño del mensaje.
    /// @param prefix Prefix a mandar junto al mensaje
    void writeString(std::string_view message, std::string_view prefix = "");

private:

    static constexpr uint8_t MessageLengthByteSize{ 1 };

    arduino::HardwareSerial& USBSerial_m;
    uint32_t baudRate_m;

    void tryReadBytes(std::vector<uint8_t>::iterator buffer, size_t expectedBytes);
    void tryWriteBytes(const std::vector<uint8_t>& buffer);
};

class MessageSizeMissmatch : std::exception {
public:

    MessageSizeMissmatch(size_t sizeExpected, size_t sizeReceived, bool onRead)
    : sizeExpected_m{ sizeExpected }
    , sizeReceived_m{ sizeReceived }
    , onRead_m{ onRead }
    {
        errorMessage_m = (std::string("Tamaño del mensaje incorrecto. Esperado: ") + std::to_string(sizeExpected_m) + (onRead_m ? " Recibido: " : " Escrito: ") 
        + std::to_string(sizeReceived_m));
    }

    const char* what() const noexcept override {
        return errorMessage_m.c_str();
    }

private:

    size_t sizeExpected_m;
    size_t sizeReceived_m;
    bool onRead_m;
    std::string errorMessage_m;
};

class NoSpaceAvailableForWrite : public std::exception {
public:

    const char* what() const noexcept override {
        return "No hay espacio disponible para escribir en el puerto USB.";
    }
};

#endif // !SERIAL_PARSER_HEADER