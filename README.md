# EasyLoRa Firmware

Firmware para el dispositivo EasyLoRa. Corre en un microcontrolador **RP2350** (Waveshare RP2040 Zero) y actúa como puente de comunicación LoRa entre una aplicación host ([EasyLoRa](https://github.com/Carlos4621/EasyLoRa)) y un módulo de radio **EByte E22-400T37S**.

---

## Arquitectura

| Componente | Rol |
|---|---|
| **`EasyLoRa_Firmware`** | Clase principal. Orquesta el flujo: recibe datos vía USB serial desde la app host, los envía por LoRa, y viceversa. |
| **`SerialParser`** | Gestiona la comunicación serial con un protocolo de longitud prefijada (primer byte = tamaño del mensaje), tanto hacia el host como hacia el módulo LoRa. |
| **`E22_400T37S_Configurator`** | Configura el módulo LoRa mediante sus pines M0/M1/AUX y UART (modos: transparente, WOR, configuración, sleep). Permite leer y escribir todos los registros del chip. |
| **`EnvelopeFactory`** | Fabrica mensajes Protocol Buffers (`Envelope`) que encapsulan datos, configuración del módulo, ACKs y errores. |
| **`MessageEncoder/Decoder`** | Serializa y deserializa los mensajes Protobuf. |
| **`Cooler`** | Gestión térmica: lee un sensor **MCP9808** (I2C) y activa/desactiva un disipador según umbrales configurables. |

## Flujo de operación

1. **`setup()`** — inicializa todos los periféricos.
2. **`loop()`** — ejecuta un bucle infinito que:
   - Escucha mensajes del host por USB serial y los reenvía por LoRa.
   - Escucha mensajes entrantes por LoRa y los reenvía al host.
   - Gestiona comandos de configuración (sincronización de parámetros del módulo).
   - Envía ACKs y errores encapsulados en Protobuf.
   - Llama al `Cooler` para regulación térmica continua.

## Protocolo de comunicación

Los mensajes se serializan con **Nanopb** (Protocol Buffers ligero para embebidos). El esquema está definido en el submódulo `EasyLoRa_Proto`. Los `Envelope` pueden contener:
- Datos raw
- Configuración del módulo
- ACK
- Mensajes de error

## Dependencias

| Librería | Uso |
|---|---|
| [Nanopb](https://github.com/nanopb/nanopb) | Serialización Protocol Buffers |
| [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) | LED de estado |
| [Adafruit MCP9808](https://github.com/adafruit/Adafruit_MCP9808_Library) | Sensor de temperatura |
| [tl::expected](https://github.com/TartanLlama/expected) | Manejo de errores sin excepciones |

## Uso conjunto

Este firmware está diseñado para usarse junto con el repositorio [EasyLoRa](https://github.com/Carlos4621/EasyLoRa), que actúa como la aplicación host encargada de enviar y recibir los mensajes a través del puerto USB serial.
