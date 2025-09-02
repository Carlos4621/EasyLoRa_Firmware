#include <Arduino.h>
#include "EasyLoRa_Firmware.hpp"
#include "SerialParser.hpp"
#include "E22_400T37S_Configurator.hpp"
#include <span>
#include <Adafruit_NeoPixel.h>

EasyLoRa_Firmware firmware{ Serial, Serial1, D3, D4, D2 };
SerialParser par{ Serial1, 9600 };

Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

[[noreturn]]
void sendAlive() {
	while (true) {
		const auto status{ par.writeString("Hola") };

		if (!status) {
			while (true) {
				Serial.println(status.error().what());
				delay(1000);
			}
		}

		delay(1000);
	}
}

[[noreturn]]
void getAlive() {
	while (true) {
		const auto status{ par.readMessage() };

		if (!status) {
			while (true) {
				Serial.println(status.error().what());
				delay(1000);
			}
		}

		for (const char& i : status.value()) {
			Serial.print(i);
		}

		Serial.println();

		delay(1000);
	}
}

void setup() {
	Serial.begin();
	
	par.begin();
}

void loop() {
	
	firmware.begin(); firmware.start();

	//sendAlive();

	//getAlive();
}

