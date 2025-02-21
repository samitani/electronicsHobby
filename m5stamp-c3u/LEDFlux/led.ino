#include <Adafruit_NeoPixel.h>
#define LED_PIN 2

Adafruit_NeoPixel pixels(1, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    pixels.begin();
    Serial.begin(115200);
}

int r = 0;
int g = 0;
int b = 0;

void loop() {
    // r + g -> g
    for (r = 0; r < 126; r++) {
            pixels.setPixelColor(0, pixels.Color(r, g, b));
            pixels.show();
            delay(50);
    }
    for (g = 0; g < 126; g++) {
            pixels.setPixelColor(0, pixels.Color(r, g, b));
            pixels.show();
            delay(50);
    }
    for (r = 125; r > 0; r--) {
            pixels.setPixelColor(0, pixels.Color(r, g, b));
            pixels.show();
            delay(50);
    }
    // g + b -> b
    for (b = 0; b < 126; b++) {
            pixels.setPixelColor(0, pixels.Color(r, g, b));
            pixels.show();
            delay(50);
    }
    for (g = 125; g > 0; g--) {
            pixels.setPixelColor(0, pixels.Color(r, g, b));
            pixels.show();
            delay(50);
    }
    // b + r -> r
    for (r = 0; r < 126; r++) {
            pixels.setPixelColor(0, pixels.Color(r, g, b));
            pixels.show();
            delay(50);
    }
    for (b = 125; b > 0; b--) {
            pixels.setPixelColor(0, pixels.Color(r, g, b));
            pixels.show();
            delay(50);
    }
}