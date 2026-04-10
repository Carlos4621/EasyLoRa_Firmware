#include "EasyLoRa_Firmware.hpp"

EasyLoRa_Firmware firmware{ Serial, Serial1, D3, D4, D2, 0x01, D0, 50, 40 };

void setup() {
	delay(200);
	firmware.begin();
}

void loop() {
	firmware.start();
}
