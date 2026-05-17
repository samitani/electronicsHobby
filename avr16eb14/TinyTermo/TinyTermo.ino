// TODO
// AHT20 Error and Retry

#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <EEPROM.h>
#include <Wire.h>

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

enum DisplayMode { MODE_OFF, MODE_CURRENT, MODE_LOW, MODE_HIGH, MODE_BATT, MODE_CLEAR };

DisplayMode mode = MODE_OFF;
DisplayMode lastMode = MODE_HIGH;

// --------------------
// AHT20
// --------------------
#define SDA_PIN PIN_PC2
#define SCL_PIN PIN_PC3
#define MEASURE_INTERVAL_MS 60000

AHT20 aht20;

float maxTemp;
float minTemp;

// --------------------
// BUTTON
// --------------------
#define BUTTON_PIN PIN_PD7

bool lastButtonState = HIGH;
unsigned long buttonPressTick = 0;
bool buttonHandled = false;

// interruption
enum TickMode { TICK_SLOW, TICK_FAST };

TickMode tickMode = TICK_SLOW;

volatile uint32_t tickCount = 0;

ISR(RTC_PIT_vect) {
  RTC.PITINTFLAGS = RTC_PI_bm;
  tickCount++;
}

void setRtcPeriodSlow() {
  while (RTC.PITSTATUS & RTC_CTRLBUSY_bm)
    ;
  RTC.PITCTRLA = 0;
  while (RTC.PITSTATUS & RTC_CTRLBUSY_bm)
    ;

  // interval about 8s
  // RTC.PITCTRLA = RTC_PERIOD_CYC8192_gc | RTC_PITEN_bm;

  // interval about 32s
  RTC.PITCTRLA = RTC_PERIOD_CYC32768_gc | RTC_PITEN_bm;
}

void setRtcPeriodFast() {
  // 1. PITのビジー状態（同期中）が終わるのを待つ
  while (RTC.PITSTATUS & RTC_CTRLBUSY_bm)
    ;

  // 2. 一度PITを無効化（PITENを0に）する
  RTC.PITCTRLA = 0;

  // 3. 無効化が内部に反映されるのを待つ
  while (RTC.PITSTATUS & RTC_CTRLBUSY_bm)
    ;

  // 4. 新しい周期を設定し、同時に有効化する
  RTC.PITCTRLA = RTC_PERIOD_CYC128_gc | RTC_PITEN_bm;

  tickMode = TICK_FAST;
}

void RTCInit() {
  while (RTC.STATUS > 0)
    ;
  RTC.CLKSEL = RTC_CLKSEL_INT1K_gc;
  RTC.PITINTCTRL = RTC_PI_bm;
  setRtcPeriodSlow();
}

void wakeUp() {}

unsigned long modeStartTick = 0;


uint16_t getVccMillivolts() {
  // 1. 基準電圧（Reference）を「内部1.024V」に設定
  analogReference(INTERNAL1V024);

  // 2. 入力チャンネルに「VDDの10分の一」を指定してアナログ読み取り
  // ※1MHz駆動のため、DxCoreが自動的に適切なADCクロック（分周）で処理してくれます。
  int reading = analogRead(ADC_VDDDIV10);

  if (reading == 0) return 0;

  // 3. 電圧(mV)の逆算
  uint32_t vcc_mv = ((uint32_t)reading * 10240UL) / 1023UL;

  return (uint16_t)vcc_mv;
}

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
    // data[3] = display.encodeDigit((value / 10) % 10) | SEG_DP;
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

  display.setBrightness(1, true);
  display.setSegments(data);
}

void showBatteryLevel(int value) {
  /*
  uint8_t data[] = {SEG_C | SEG_D | SEG_E | SEG_F | SEG_G, // b
                    CHAR_BAR, CHAR_BAR, CHAR_NONE};
  */
  uint8_t data[4];
  data[0] = display.encodeDigit((value / 1000) % 10);
  data[1] = display.encodeDigit((value / 100) % 10);
  data[2] = display.encodeDigit((value / 10) % 10);
  data[3] = display.encodeDigit((value) % 10);

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

void clearTempMinMax() {
  minTemp = 100;
  maxTemp = -100;
  EEPROM.put(0, minTemp);
  EEPROM.put(4, maxTemp);
}

void updateTempMinMax(float current) {
  bool updated = false;

  if (current > maxTemp) {
    maxTemp = current;
    updated = true;
  }

  if (current < minTemp) {
    minTemp = current;
    updated = true;
  }

  if (updated) {
    EEPROM.put(0, minTemp);
    EEPROM.put(4, maxTemp);
  }
}

void setup() {
  // Serial.begin(115200);

#ifdef DEBUG
  pinMode(PIN_LED, OUTPUT);
#endif

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

  RTCInit();
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), wakeUp, FALLING);
}

void loop() {
  // --- 1ティックにつき1回だけ実行するためのガード ---
  noInterrupts();
  uint32_t currentTick = tickCount;
  interrupts();

  if (mode == MODE_OFF && currentTick % 2 == 0) {
    updateTempMinMax(aht20.getTemperature());
  }

  bool buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW) {
    // beggining of pressed button, only react during OFF
    if (lastButtonState == HIGH && mode == MODE_OFF) {
      setRtcPeriodFast();
      buttonPressTick = currentTick;
      buttonHandled = false;
    }

    // long press
    if (!buttonHandled && (currentTick - buttonPressTick >= 40)) {
      mode = MODE_CLEAR;
      buttonHandled = true;
    }
  } else { // HIGH
    // released button
    if (!buttonHandled && lastButtonState == LOW) {
      mode = MODE_CURRENT;
      buttonHandled = true;
    }
  }

  lastButtonState = buttonState;

  switch (mode) {
  case MODE_CURRENT:
    if (mode != lastMode) {
      lastMode = mode;
      modeStartTick = currentTick;
      float currentTemp = aht20.getTemperature();
      updateTempMinMax(currentTemp);
      showTemperature(CHAR_NONE, currentTemp);
      setRtcPeriodFast();
    }

    if (currentTick - modeStartTick >= 24) {
      mode = MODE_LOW;
    }
    break;
  case MODE_LOW:
    if (mode != lastMode) {
      lastMode = mode;
      modeStartTick = currentTick;
      showTemperature(CHAR_L, minTemp);
    }
    if (currentTick - modeStartTick >= 24) {
      mode = MODE_HIGH;
    }
    break;
  case MODE_HIGH:
    if (mode != lastMode) {
      lastMode = mode;
      modeStartTick = currentTick;
      showTemperature(CHAR_H, maxTemp);
    }
    if (currentTick - modeStartTick >= 24) {
      mode = MODE_BATT;
    }
    break;
  case MODE_BATT:
    if (mode != lastMode) {
      lastMode = mode;
      modeStartTick = currentTick;
      showBatteryLevel(getVccMillivolts());
    }
    if (currentTick - modeStartTick >= 24) {
      mode = MODE_OFF;
    }
    break;
  case MODE_OFF:
    if (mode != lastMode) {
      lastMode = mode;
      modeStartTick = currentTick;
      display.setBrightness(0, false);
      display.clear();
      setRtcPeriodSlow();
    }
    break;
  case MODE_CLEAR:
    if (mode != lastMode) {
      lastMode = mode;
      modeStartTick = currentTick;
      clearTempMinMax();
      showClear();
      setRtcPeriodFast();
    }
    if (currentTick - modeStartTick >= 48) {
      mode = MODE_OFF;
    }
  }

  set_sleep_mode(SLEEP_MODE_STANDBY);
  sleep_mode();
}
