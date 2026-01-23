#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>
#include <math.h>

#define M5LED_PIN 2
#define SERVO_PIN 8

#define PUSH_SWITCH_PIN 10
#define JOYSTICK_X_PIN 0
#define JOYSTICK_Y_PIN 1

#define JOYSTICK_THRESHOLD_DOWN 400
#define JOYSTICK_THRESHOLD_UP 4050
#define JOYSTICK_THRESHOLD_LEFT 400
#define JOYSTICK_THRESHOLD_RIGHT 4050

#define TB6647PG_1_S_PIN 3
#define TB6647PG_2_S_PIN 4
#define TB6647PG_A_PIN 5
#define TB6647PG_B_PIN 6
#define MOTOR_DELAY_MS 3

#ifdef ENABLE_GRIPPER_LOTTERY
#define GRIP_DETECT_PIN 7
#define GRIPPER_LOTTERY_THRESHOLD_MIN 400
#define GRIPPER_LOTTERY_THRESHOLD_MAX 1000
#endif

#define COLOR_NONE pixels.Color(0, 0, 0)
#define COLOR_RED pixels.Color(255, 0, 0)
#define COLOR_BLUE pixels.Color(0, 0, 255)
#define COLOR_GREEN pixels.Color(0, 255, 0)
#define COLOR_YELLOW pixels.Color(255, 255, 0)
#define COLOR_WHITE pixels.Color(255, 255, 255)

Adafruit_NeoPixel pixels(1, M5LED_PIN, NEO_GRB + NEO_KHZ800);
Servo motor_servo;
int joyStickX, joyStickY, valueBtn, isGripperOpen;
#ifdef ENABLE_GRIPPER_LOTTERY
int gripperBtn, moveCounter, lotteryThreshold, lotteryPercentage;
#endif

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

void gripper_open() {
  motor_servo.write(0);
  isGripperOpen = 1;
  delay(1000);
}

void gripper_close() {
  motor_servo.write(180);
  isGripperOpen = 0;
  delay(1000);
}

void gripper_lottery() {
  if (moveCounter == 0) {
    lotteryThreshold =
        random(GRIPPER_LOTTERY_THRESHOLD_MIN, GRIPPER_LOTTERY_THRESHOLD_MAX);
  }
  moveCounter++;
  pixels.setPixelColor(
      0, pixels.Color(255 - (((float)moveCounter / lotteryThreshold) * 255), 0,
                      255));
  if (moveCounter == lotteryThreshold) {
    pixels.setPixelColor(0, COLOR_RED);
    if (random(100) < lotteryPercentage) { // 0 ~ 99
      gripper_open();
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(PUSH_SWITCH_PIN, INPUT_PULLDOWN);

  pinMode(TB6647PG_1_S_PIN, OUTPUT);
  pinMode(TB6647PG_2_S_PIN, OUTPUT);
  pinMode(TB6647PG_A_PIN, OUTPUT);
  pinMode(TB6647PG_B_PIN, OUTPUT);
#ifdef ENABLE_GRIPPER_LOTTERY
  pinMode(GRIP_DETECT_PIN, INPUT_PULLDOWN);
#endif

  motor_servo.attach(SERVO_PIN);
  pixels.begin();

  gripper_open();

#ifdef ENABLE_GRIPPER_LOTTERY
  joyStickY = analogRead(JOYSTICK_Y_PIN);

  if (joyStickY < JOYSTICK_THRESHOLD_DOWN) { // DOWN
    pixels.setPixelColor(0, COLOR_BLUE);
    pixels.show();
    lotteryPercentage = 0; // easy mode
    delay(1000);
  } else if (joyStickY > JOYSTICK_THRESHOLD_UP) { // UP
    pixels.setPixelColor(0, COLOR_RED);
    pixels.show();
    lotteryPercentage = 80; // hard mode
    delay(1000);
  } else {
    lotteryPercentage = 30; // normal mode
  }
#endif
}

void loop() {
  joyStickX = analogRead(JOYSTICK_X_PIN);
  joyStickY = analogRead(JOYSTICK_Y_PIN);
  valueBtn = digitalRead(PUSH_SWITCH_PIN);

#ifdef ENABLE_GRIPPER_LOTTERY
  gripperBtn = digitalRead(GRIP_DETECT_PIN);
#endif

  if (joyStickX > JOYSTICK_THRESHOLD_RIGHT) { // RIGHT
    pixels.setPixelColor(0, COLOR_YELLOW);
#ifdef ENABLE_GRIPPER_LOTTERY
    if (gripperBtn == LOW) {
      gripper_lottery();
    }
#endif
    pixels.show();

    digitalWrite(TB6647PG_1_S_PIN, LOW);
    digitalWrite(TB6647PG_2_S_PIN, HIGH);
    motor_backword();
  } else if (joyStickX < JOYSTICK_THRESHOLD_LEFT) { // LEFT
    pixels.setPixelColor(0, COLOR_GREEN);
    pixels.show();

    digitalWrite(TB6647PG_1_S_PIN, LOW);
    digitalWrite(TB6647PG_2_S_PIN, HIGH);
    motor_forword();
  } else if (joyStickY > JOYSTICK_THRESHOLD_UP) { // UP
    pixels.setPixelColor(0, COLOR_BLUE);
#ifdef ENABLE_GRIPPER_LOTTERY
    if (gripperBtn == LOW) {
      gripper_lottery();
    }
#endif
    pixels.show();

    digitalWrite(TB6647PG_1_S_PIN, HIGH);
    digitalWrite(TB6647PG_2_S_PIN, LOW);
    motor_forword();
  } else if (joyStickY < JOYSTICK_THRESHOLD_DOWN) { // DOWN
    pixels.setPixelColor(0, COLOR_RED);
    pixels.show();

    digitalWrite(TB6647PG_1_S_PIN, HIGH);
    digitalWrite(TB6647PG_2_S_PIN, LOW);
    motor_backword();
  } else if (valueBtn) {
    pixels.setPixelColor(0, COLOR_WHITE);
    pixels.show();

    isGripperOpen ? gripper_close() : gripper_open();
  } else {
    digitalWrite(TB6647PG_1_S_PIN, LOW);
    digitalWrite(TB6647PG_2_S_PIN, LOW);

    pixels.setPixelColor(0, COLOR_NONE);
#ifdef ENABLE_GRIPPER_LOTTERY
    if (gripperBtn == HIGH && isGripperOpen) {
      moveCounter = 0;
      pixels.setPixelColor(0, COLOR_WHITE);
    }
#endif
    pixels.show();
    delay(200);
  }
}
