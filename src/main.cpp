#include <Arduino.h>
#include "EasyLoRa_Firmware.hpp"

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}