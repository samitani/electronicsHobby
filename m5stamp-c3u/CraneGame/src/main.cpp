#include <math.h>
#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>

#define M5LED_PIN 2
#define SERVO_PIN 8

#define JOYSTICK_PUSH_PIN 10
#define JOYSTICK_X_PIN 0
#define JOYSTICK_Y_PIN 1

#define TB6647PG_1_S_PIN 3
#define TB6647PG_2_S_PIN 4
#define TB6647PG_A_PIN 5
#define TB6647PG_B_PIN 6
#define MOTOR_DELAY_MS 3

Adafruit_NeoPixel pixels(1, M5LED_PIN, NEO_GRB + NEO_KHZ800);
Servo motor_servo;
int valueX, valueY, valueBtn, isArmOpen;

void setup() {
  Serial.begin(9600);
  pinMode(JOYSTICK_PUSH_PIN, INPUT_PULLUP);

  pinMode(TB6647PG_1_S_PIN,OUTPUT);
  pinMode(TB6647PG_2_S_PIN,OUTPUT);
  pinMode(TB6647PG_A_PIN,OUTPUT);
  pinMode(TB6647PG_B_PIN,OUTPUT);

  pixels.begin();
  motor_servo.attach(SERVO_PIN);

  isArmOpen = 0;
}

void motor_backword() {
    digitalWrite(TB6647PG_A_PIN, HIGH);
    digitalWrite(TB6647PG_B_PIN, HIGH);
    delay(MOTOR_DELAY_MS);

    digitalWrite(TB6647PG_A_PIN, HIGH);
    digitalWrite(TB6647PG_B_PIN, LOW);
    delay(MOTOR_DELAY_MS);

    digitalWrite(TB6647PG_A_PIN, LOW);
    digitalWrite(TB6647PG_B_PIN, LOW);
    delay(MOTOR_DELAY_MS);

    digitalWrite(TB6647PG_A_PIN, LOW);
    digitalWrite(TB6647PG_B_PIN, HIGH);
    delay(MOTOR_DELAY_MS);
}

void motor_forword() {
    digitalWrite(TB6647PG_A_PIN, HIGH);
    digitalWrite(TB6647PG_B_PIN, HIGH);
    delay(MOTOR_DELAY_MS);

    digitalWrite(TB6647PG_A_PIN, LOW);
    digitalWrite(TB6647PG_B_PIN, HIGH);
    delay(MOTOR_DELAY_MS);

    digitalWrite(TB6647PG_A_PIN, LOW);
    digitalWrite(TB6647PG_B_PIN, LOW);
    delay(MOTOR_DELAY_MS);

    digitalWrite(TB6647PG_A_PIN, HIGH);
    digitalWrite(TB6647PG_B_PIN, LOW);
    delay(MOTOR_DELAY_MS);
}

void loop() {
  valueX = analogRead(JOYSTICK_X_PIN);
  valueY = analogRead(JOYSTICK_Y_PIN);
  valueBtn = digitalRead(JOYSTICK_PUSH_PIN);
 
  if (valueX > 4050) { // UP
    pixels.setPixelColor(0, pixels.Color(128, 0, 0));
    pixels.show();

    digitalWrite(TB6647PG_1_S_PIN, HIGH);
    digitalWrite(TB6647PG_2_S_PIN, LOW);
    motor_backword();
  } else if (valueX < 300) { // DOWN
    pixels.setPixelColor(0, pixels.Color(0, 0, 128));
    pixels.show();

    digitalWrite(TB6647PG_1_S_PIN, HIGH);
    digitalWrite(TB6647PG_2_S_PIN, LOW);
    motor_forword();
  } else if (valueY > 4050) { // RIGHT
    pixels.setPixelColor(0, pixels.Color(128, 128, 128));
    pixels.show();

    digitalWrite(TB6647PG_1_S_PIN, LOW);
    digitalWrite(TB6647PG_2_S_PIN, HIGH);
    motor_backword();
  } else if (valueY < 700) { // LEFT
    pixels.setPixelColor(0, pixels.Color(0, 128, 0));
    pixels.show();

    digitalWrite(TB6647PG_1_S_PIN, LOW);
    digitalWrite(TB6647PG_2_S_PIN, HIGH);
    motor_forword();
  } else if (!valueBtn) {
    pixels.setPixelColor(0, pixels.Color(128, 128, 0));
    pixels.show();

    if (isArmOpen) {
      motor_servo.write(0);
    } else {
      motor_servo.write(180);
    }
    isArmOpen = !isArmOpen;
    delay(1000);
  } else {
    digitalWrite(TB6647PG_1_S_PIN, LOW);
    digitalWrite(TB6647PG_2_S_PIN, LOW);

    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    pixels.show();
    delay(300);
  }


}
