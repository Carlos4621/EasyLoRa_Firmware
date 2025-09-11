#ifndef SERIAL_PARSER_HEADER
#define SERIAL_PARSER_HEADER

#include <Arduino.h>
#include <string>
#include <vector>
#include <stdexcept>
#include "tl/expected.hpp"
#include <memory>
#include "EasyLoRa_Exceptions.hpp"

/*
    TODO:
        - el size byte no es eliminado en readMessage, debo eliminar la posibilidad de añadir prefix y quitar ese byte antes de devolver el mensaje
*/

/// @brief Clase encargada del formateo de mensajes enviados y recibidos desde un puerto serial
class SerialParser {
public:

    /// @brief Constructor base
    /// @param serial Puerto en donde se enviarán y recibirán paquetes donde el primer byte el tamaño del mensaje. Opcionalmente se puede incluir un prefix
    /// @param baudRate Velocidad de baudios para la comunicación serial, por defecto 115200
    explicit SerialParser(arduino::HardwareSerial& serial, uint32_t baudRate = 115200);

    /// @brief Inicializa el dispositivo, debe ser llamado antes de cualquier otra función.
    void begin();

    /// @brief Cambia el baud rate de la comunicación serial
    /// @param baudRate Velocidad de baudios para la comunicación serial
    void setBaudRate(uint32_t baudRate);

    /// @brief Obtiene el baudrate
    /// @return El baudRate actual
    [[nodiscard]]
    uint32_t getBaudRate() const;

    /// @brief Intenta leer un mensaje del puerto serial
    /// @param prefixLength Longitud del prefix
    /// @return tl::expected con std::vector<uint8_t> que contiene el mensaje leído si se pudo leer uno,
    /// MessageSizeMissmatch si el byte de longitud del mensaje es incorrecto
    [[nodiscard]]
    tl::expected<std::vector<uint8_t>, std::shared_ptr<std::exception>> readMessage(uint8_t prefixLength = 0);

    /// @brief Escribe mensajes en el puerto serial
    /// @param message Mensaje a enviar, el primer byte (si es que no hay prefix) serán el tamaño del mensaje.
    /// @param prefix Prefix a mandar junto al mensaje
    /// @return tl::excepted con MessageSizeMissmatch si el byte de longitud del mensaje es incorrecto
    /// @example 0xAA 0xBB => 0x02 0xAA 0xBB 
    [[nodiscard]]
    tl::expected<void, std::shared_ptr<std::exception>> writeMessage(const std::vector<uint8_t>& message, const std::vector<uint8_t>& prefix = {});

    /// @brief Método de conveniencia para strings, sigue la misma lógica que writeMessage
    /// @param message Mensaje a enviar, el primer byte (si es que no hay prefix) serán el tamaño del mensaje.
    /// @param prefix Prefix a mandar junto al mensaje
    /// @return tl::expected con MessageSizeMissmatch si el byte de longitud del mensaje es incorrecto
    [[nodiscard]]
    tl::expected<void, std::shared_ptr<std::exception>> writeString(std::string_view message, std::string_view prefix = "");

    /// @brief Escribe el mensaje tal como está, no se agrega prefix ni tamaño del mensaje
    /// @param message Mensaje a enviar
    /// @return tl::expected con MessageSizeMissmatch si el byte de longitud del mensaje es incorrecto
    [[nodiscard]]
    tl::expected<void, std::shared_ptr<std::exception>> writeCrudeMessage(const std::vector<uint8_t>& message);

private:

    static constexpr uint8_t Message_Length_Byte_Size{ 1 };

    arduino::HardwareSerial& serial_m;
    uint32_t baudRate_m;

    /// @brief Intenta leer un mensaje del puerto serial
    /// @param buffer iterador a buffer en donde se escribirán los datos
    /// @param expectedBytes bytes esperados
    /// @param emptyReadIsValid Permite devolver false en caso de no haber leído ningún dato en vez de lanzar MessageSizeMissmatch
    /// @return tl::expected con true si se han leídos los expectedBytes.
    /// false si (emptyReadIsValid && bytesLeidos == 0).
    /// MessageSizeMissmatch si (!emptyReadIsValid && bytesLeídos != expectedBytes)
    [[nodiscard]]
    tl::expected<bool, std::shared_ptr<std::exception>> tryReadBytes(std::vector<uint8_t>::iterator buffer, size_t expectedBytes, bool emptyReadIsValid = false);
    
    [[nodiscard]]
    tl::expected<void, std::shared_ptr<std::exception>> tryWriteBytes(const std::vector<uint8_t>& buffer);
};

#endif // !SERIAL_PARSER_HEADER