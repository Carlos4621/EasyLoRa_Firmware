#include <Arduino.h>
#include "EasyLoRa_Firmware.hpp"
#include "SerialParser.hpp"
#include "E22_400T37S_Configurator.hpp"
#include <span>
#include <Adafruit_NeoPixel.h>

EasyLoRa_Firmware firmware{ Serial, Serial1, D3, D4, D2 };

void setup() {
}

void loop() {
	firmware.begin(); firmware.start();
}

