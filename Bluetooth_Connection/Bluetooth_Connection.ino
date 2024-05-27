#include <SoftwareSerial.h>
SoftwareSerial hc05(4,3);
void setup() {
  // Serial.begin(38400);
  hc05.begin(38400);
  hc05.begin(9600);
}

void loop() {
  hc05.print(5);
  delay(100);
}