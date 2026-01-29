#include <Wire.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

#define COLOR_NONE pixels.Color(0, 0, 0)
#define COLOR_RED pixels.Color(255, 0, 0)
#define COLOR_BLUE pixels.Color(0, 0, 255)
#define COLOR_GREEN pixels.Color(0, 255, 0)
#define COLOR_YELLOW pixels.Color(255, 255, 0)
#define COLOR_WHITE pixels.Color(255, 255, 255)

#define M5LED_PIN 2
#define M5LED_BUTTON 9

#define BUZZER_PIN 0

#define SERVO_PIN 5
#define SERVO_CLOCKWISE_SLOW 80
#define SERVO_CLOCKWISE_FAST 40
#define SERVO_STOP 90
#define SERVO_ONELAP_DELAY 3145 // Time in ms for one full rotation at SERVO_CLOCKWISE_FAST speed

Adafruit_NeoPixel pixels(1, M5LED_PIN, NEO_GRB + NEO_KHZ800);

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_SDA 8
#define OLED_SCL 7
#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

#define CDS_PIN 3

// Number plates for starting lottery
#define LOTTERY_PLATE_COUNT 5
#define LOTTERY_PROBABILITY 2

bool isBlocked = false;   // 遮蔽フラグ
int plateCount = 0;
const int threshold = 400; // 遮蔽判定のしきい値（環境に合わせて調整）

Servo mg996r360;

// 画面表示の更新
void updateDisplay(int cdsValue) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Bikkura-Pon ");
  display.println(cdsValue);
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  display.setTextSize(2);
  display.setCursor(10, 30);
  display.print("PLATES: ");
  display.print(plateCount);
  display.display();
}

// 当たり音
void playWinSound() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(150);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }

  delay(200);

  // 長めに鳴らして締め
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
}

// 抽選アニメーションと判定
void runLottery() {
  display.clearDisplay();
  display.setTextSize(3);
  
  // アニメーション（簡易演出）
  for (int i = 0; i < 3; i++) {
    display.clearDisplay();
    display.setCursor(0, 15);
    display.print("GOGO..");
    display.display();
    delay(300);
    display.print("!");
    display.display();
    delay(300);
  }

  // 抽選（ 1/LOTTERY_PROBABILITY の確率）
  int result = random(0, LOTTERY_PROBABILITY);
  display.clearDisplay();
  
  if (result == 0) { // 「0」を当たりとする

    display.setCursor(0, 10);
    display.setTextSize(3);
    display.println("\\(^-^)/");
    display.display();

    playWinSound();
    delay(500);


    mg996r360.write(SERVO_CLOCKWISE_FAST);
    delay(SERVO_ONELAP_DELAY);
    mg996r360.write(SERVO_STOP);

  } else {
    display.setCursor(0, 10);
    display.setTextSize(3);
    display.println("(T ^ T)");
    display.display();
    delay(3000); // 結果を3秒表示
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(M5LED_BUTTON, INPUT_PULLUP);
  pinMode(CDS_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  mg996r360.attach(SERVO_PIN);

  pixels.setPixelColor(0, COLOR_RED);
  pixels.show();
  
  Wire.begin(OLED_SDA, OLED_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;);
  }

  pixels.setPixelColor(0, COLOR_BLUE);
  pixels.show();
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  updateDisplay(0);
}

void loop() {
  int buttonValue = digitalRead(M5LED_BUTTON);

  if (!buttonValue) {
      mg996r360.write(SERVO_CLOCKWISE_SLOW);
      delay(200);
  } else {
      mg996r360.write(SERVO_STOP);
  }

  int cdsValue = analogRead(CDS_PIN);

  // Serial.print("Sensor Value: ");
  // Serial.println(lightValue);

  // 光が遮られた（値が小さくなった）時の判定
  updateDisplay(cdsValue);

  if (cdsValue < threshold && !isBlocked) {
    plateCount++;
    isBlocked = true;
    updateDisplay(cdsValue);
    delay(200); // チャタリング防止
  } 
  else if (cdsValue > threshold + 200) {
    isBlocked = false;
  }

  // LOTTERY_PLATE_COUNT 回溜まったら抽選開始
  if (plateCount >= LOTTERY_PLATE_COUNT) {
    runLottery();
    plateCount = 0;
    updateDisplay(cdsValue);
  }
}


