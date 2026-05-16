// TODO
// Sleep
// BatteryADC
// EPPROM
// AHT20 Error and Retry

#include <avr/sleep.h>
#include <avr/interrupt.h>

#include <Wire.h>
#include <EEPROM.h>

#include <AHT20.h>
#include <TM1637Display.h>

#define PIN_LED PIN_PD6
// --------------------
// TM1637
// --------------------
#define PIN_CLK PIN_PC0
#define PIN_DIO PIN_PC1

#define CHAR_NONE 0x00
#define CHAR_L SEG_D | SEG_E | SEG_F
#define CHAR_H SEG_B | SEG_C | SEG_E | SEG_F | SEG_G
#define CHAR_BAR SEG_G

TM1637Display display(PIN_CLK, PIN_DIO);

enum DisplayMode { MODE_OFF, MODE_CURRENT, MODE_LOW, MODE_HIGH, MODE_BATT};

DisplayMode mode = MODE_OFF;
DisplayMode lastMode = MODE_HIGH;

// --------------------
// AHT20
// --------------------
//#define SDA_PIN PIN_PA0
//#define SCL_PIN PIN_PA1
#define SDA_PIN PIN_PC2
#define SCL_PIN PIN_PC3
#define MEASURE_INTERVAL_MS 60000

AHT20 aht20;
float currentTemp;
unsigned long lastMeasureTime = 0;

float maxTemp;
float minTemp;

// --------------------
// BUTTON
// --------------------
#define BUTTON_PIN PIN_PD7

bool lastButtonState = HIGH;
unsigned long buttonPressTime = 0;
bool buttonHandled = false;

unsigned long modeStartTime = 0;

void showTemperature(uint8_t chara, float temp) {
  int value;
  uint8_t data[4];

  data[0] = chara; // L or H

  if (temp < 0) {
    // -12.3 -> -12
    value = (int)(-temp + 0.5f);
    data[1] = CHAR_BAR;
    data[2] = display.encodeDigit((value / 10) % 10);
    data[3] = display.encodeDigit(value % 10);
  } else {
    // 23.4 -> 234
    value = (int)(temp * 10);
    data[1] = display.encodeDigit((value / 100) % 10);
    data[2] = display.encodeDigit((value / 10) % 10);
    //data[3] = display.encodeDigit((value / 10) % 10) | SEG_DP;
    data[3] = display.encodeDigit(value % 10);
  }


  display.setBrightness(1, true);
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

  display.setBrightness(1, true);
  display.setSegments(data);
}

void loadTempMinMax() {
  EEPROM.get(0, minTemp);
  EEPROM.get(4, maxTemp);

  // if no value in EEPROM
  if (isnan(minTemp) || isnan(maxTemp)) {
    minTemp = 100;
    maxTemp = -100;
  }
}

void saveTempMinMax() {
  EEPROM.put(0, minTemp);
  EEPROM.put(4, maxTemp);
}

void setup() {
  pinMode(PIN_LED, OUTPUT);
  //Serial.begin(115200);

  // Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Begin I2C for AHT20
  Wire.swap(2);
  Wire.begin();

  // AHT20初期化
  do {
    delay(1000);
  } while (aht20.begin() != true);

  display.clear();
  display.setBrightness(0, false);

  loadTempMinMax();
}

void loop() {
  bool buttonState = digitalRead(BUTTON_PIN);

  if (lastMeasureTime == 0 || millis() - lastMeasureTime >= (unsigned long)(MEASURE_INTERVAL_MS)) {
    lastMeasureTime = millis();
    currentTemp = aht20.getTemperature();

    if (currentTemp > maxTemp) {
      maxTemp = currentTemp;
      saveTempMinMax();
    }
    if (currentTemp < minTemp) {
      minTemp = currentTemp;
      saveTempMinMax();
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
      display.clear();
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
      display.setBrightness(0, false);
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

  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_mode();
}
