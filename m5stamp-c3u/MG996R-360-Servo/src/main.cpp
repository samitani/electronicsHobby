#include <ESP32Servo.h>

#define SERVO_PIN 5
#define M5STAMP_BUTTON 9

Servo mg996r360;
int buttonValue;

void setup() {
  pinMode(M5STAMP_BUTTON, INPUT_PULLUP);

  mg996r360.attach(SERVO_PIN);
}

void loop() {
  buttonValue = digitalRead(M5STAMP_BUTTON);

  if (buttonValue) {
    mg996r360.write(0);
  } else {
    mg996r360.write(180);
  }
  delay(1000);
}
