#include <Arduino.h>

#define LED_PIN 38
#define BRIGHTNESS 40

void setup() {
  Serial.begin(115200);
  delay(1000);
}

void loop() {
  neopixelWrite(LED_PIN, BRIGHTNESS, 0, 0);     // Red
  delay(500);

  neopixelWrite(LED_PIN, 0, BRIGHTNESS, 0);     // Green
  delay(500);

  neopixelWrite(LED_PIN, 0, 0, BRIGHTNESS);     // Blue
  delay(500);

  neopixelWrite(LED_PIN, 0, 0, 0);              // Off
  delay(500);
}
