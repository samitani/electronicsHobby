#include <avr/sleep.h>
#include <avr/interrupt.h>

#include <AHT20.h>
#include <TM1637Display.h>
#include <Wire.h>

// --------------------
// TM1637
// --------------------
#define CLK 8
#define DIO 10

#define CHAR_NONE 0x00
#define CHAR_L SEG_D | SEG_E | SEG_F
#define CHAR_H SEG_B | SEG_C | SEG_E | SEG_F | SEG_G
#define CHAR_BAR SEG_G

TM1637Display display(CLK, DIO);

enum DisplayMode { MODE_OFF, MODE_CURRENT, MODE_LOW, MODE_HIGH, MODE_BATT};

DisplayMode mode = MODE_OFF;
DisplayMode lastMode = MODE_HIGH;

// --------------------
// AHT20
// --------------------
#define SDA_PIN 4
#define SCL_PIN 3
#define MEASURE_INTERVAL_SECOND 60

AHT20 aht20;
float currentTemp;
unsigned long lastMeasureTime = 0;

float maxTemp = -100;
float minTemp = 100;

// --------------------
// BUTTON
// --------------------
#define BUTTON_PIN 9

bool lastButtonState = HIGH;
unsigned long buttonPressTime = 0;
bool buttonHandled = false;

unsigned long modeStartTime = 0;

void showTemperature(uint8_t chara, float temp) {
  // 23.4 -> 234
  int value = (int)(temp * 10);

  uint8_t data[] = {chara, display.encodeDigit((value / 100) % 10),
                    display.encodeDigit((value / 10) % 10),
                    // display.encodeDigit((value / 10) % 10) | SEG_DP,
                    display.encodeDigit(value % 10)};

  display.setSegments(data);
}

void showClear() {
  uint8_t data[] = {SEG_A | SEG_D | SEG_E | SEG_F, // C
                    SEG_D | SEG_E | SEG_F,         // L
                    SEG_E | SEG_G,                 // r
                    CHAR_NONE};

  display.setSegments(data);
}

void showBatteryLevel(int voltage) {
  uint8_t data[] = {SEG_C | SEG_D | SEG_E | SEG_F | SEG_G, // b
                    CHAR_BAR,
                    CHAR_BAR,
                    CHAR_NONE};

  display.setSegments(data);
}

void setup() {
  Serial.begin(9600);
  //Serial.begin(115200);

  // Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // I2C開始
  // Wire.begin(SDA_PIN, SCL_PIN);


  // AHT20初期化
  do {
    delay(1000);
  } while (aht20.begin() != true);

  display.clear();
  display.setBrightness(1);
}

void loop() {
  bool buttonState = digitalRead(BUTTON_PIN);

  if (lastMeasureTime == 0 || millis() - lastMeasureTime >= (unsigned long)(MEASURE_INTERVAL_SECOND * 1000)) {
    lastMeasureTime = millis();
    currentTemp = aht20.getTemperature();

    // TODO: EPPROM min/max 更新
    if (currentTemp > maxTemp) {
      maxTemp = currentTemp;
    }
    if (currentTemp < minTemp) {
      minTemp = currentTemp;
    }
  }

  if (buttonState == LOW) {
    // beggining of pressed button
    if (lastButtonState == HIGH) {
      buttonPressTime = millis();
      buttonHandled = false;
    }
    // keep pressed
    if (!buttonHandled && millis() - buttonPressTime >= 5000) {
      buttonHandled = true;

      // TODO: EEPROMクリア
      showClear();
      delay(3000);
      mode = MODE_OFF;
    }
  } else { // HIGH
    // released button
    if (lastButtonState == LOW) {
      if (!buttonHandled) {
        currentTemp = aht20.getTemperature();
        mode = MODE_CURRENT;
        modeStartTime = millis();
      }
    }
  }

  lastButtonState = buttonState;

  switch (mode) {
  case MODE_OFF:
    if (mode != lastMode) {
      display.clear();
    }
    lastMode = mode;
    break;
  case MODE_CURRENT:
    showTemperature(CHAR_NONE, currentTemp);
    // use millis() instead of delay() to lower battery consumption
    if (millis() - modeStartTime >= 3000) {
      lastMode = mode;
      mode = MODE_LOW;
      modeStartTime = millis();
    }
    break;
  case MODE_LOW:
    showTemperature(CHAR_L, minTemp);
    if (millis() - modeStartTime >= 3000) {
      lastMode = mode;
      mode = MODE_HIGH;
      modeStartTime = millis();
    }
    break;
  case MODE_HIGH:
    showTemperature(CHAR_H, maxTemp);
    if (millis() - modeStartTime >= 3000) {
      lastMode = mode;
      mode = MODE_BATT;
      modeStartTime = millis();
    }
    break;
  case MODE_BATT:
    showBatteryLevel(100);
    if (millis() - modeStartTime >= 3000) {
      lastMode = mode;
      mode = MODE_OFF;
      modeStartTime = millis();
    }
    break;
  }
}
