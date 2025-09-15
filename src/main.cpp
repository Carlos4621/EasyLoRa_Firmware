#include "boards/pico_w.h"
#include "EasyLoRa_Firmware.hpp"

EasyLoRa_Firmware firmware{ Serial, Serial1, D3, D4, D2 };

void setup() {
	delay(200);
	firmware.begin();
}

void loop() {
	firmware.start();
}
