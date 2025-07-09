#include <Arduino.h>
#include "EasyLoRa_Firmware.hpp"
#include "SerialParser.hpp"
#include "E22_400T37S_Configurator.hpp"
#include <span>

E22_400T37S_Configurator test{ Serial1, D0, D1, D2 };
SerialParser par{ Serial1, 9600 };

void setup() {
	Serial.begin();

	pinMode(D0, OUTPUT);
	pinMode(D1, OUTPUT);

	digitalWrite(D0, LOW);
	digitalWrite(D1, HIGH);

	par.begin();

	delay(15000);

	Serial.println("setup");
}

void loop() {

	const auto status{ par.writeCrudeMessage({0xC1, 0x05, 0x01}) };
	if (!status) {
		Serial.println(status.error().what());
		while(true) delay(100);
	}
	
	const auto response{ par.readMessage(2) };
	if (!response) {
		Serial.println(response.error().what());
		while(true) delay(100);
	}
	
	for (const auto& i : response.value()) {
		Serial.print(i, HEX);
		Serial.print(" ");
	}

	Serial.println();

	Serial.println("Final");

	while(true) delay(1000);
}
