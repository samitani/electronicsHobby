#include <Wire.h>
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
Adafruit_NeoPixel pixels(1, M5LED_PIN, NEO_GRB + NEO_KHZ800);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define CDS_PIN 3

int plateCount = 0;       // カウント数
bool isBlocked = false;   // 遮蔽フラグ
const int threshold = 200; // 遮蔽判定のしきい値（環境に合わせて調整）

// 画面表示の更新
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Bikkura-Pon System");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  display.setTextSize(2);
  display.setCursor(10, 30);
  display.print("PLATES: ");
  display.print(plateCount);
  display.display();
}

// 抽選アニメーションと判定
void runLottery() {
  display.clearDisplay();
  display.setTextSize(2);
  
  // アニメーション（簡易演出）
  for (int i = 0; i < 3; i++) {
    display.clearDisplay();
    display.setCursor(20, 25);
    display.print("GOGO...  ");
    display.display();
    delay(300);
    display.print("!");
    display.display();
    delay(300);
  }

  // 抽選（1/3の確率）
  int result = random(0, 3); 
  display.clearDisplay();
  
  if (result == 0) { // 「0」を当たりとする
    display.setCursor(20, 10);
    display.setTextSize(3);
    display.println("ATARI!!");
    display.setTextSize(1);
    display.println("  CONGRATS!");
    // 簡易的な火花アニメーション
    for(int i=0; i<10; i++){
      display.drawCircle(64, 40, i*3, SSD1306_WHITE);
      display.display();
    }
  } else {
    display.setCursor(20, 25);
    display.setTextSize(2);
    display.println("HAZURE...");
  }
  
  display.display();
  delay(3000); // 結果を3秒表示
}

void setup() {
  Serial.begin(115200);

  pinMode(CDS_PIN, INPUT);

  pixels.setPixelColor(0, COLOR_RED);
  pixels.show();
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;);
  }

  pixels.setPixelColor(0, COLOR_BLUE);
  pixels.show();
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  updateDisplay();
}

void loop() {
  int lightValue = analogRead(CDS_PIN);

/*
  Serial.print("Sensor Value: ");
  Serial.println(lightValue);
*/

  // 光が遮られた（値が小さくなった）時の判定
  if (lightValue < threshold && !isBlocked) {
    plateCount++;
    isBlocked = true;
    updateDisplay();
    delay(200); // チャタリング防止
  } 
  else if (lightValue > threshold + 200) {
    isBlocked = false;
  }

  // 5回溜まったら抽選開始
  if (plateCount >= 5) {
    runLottery();
    plateCount = 0;
    updateDisplay();
  }
}


