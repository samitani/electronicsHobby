#include "led_controller.h"
#include "config.h"
#include <Adafruit_NeoPixel.h>

static Adafruit_NeoPixel* strip = nullptr;
static uint16_t s_num_leds = LED_COUNT;
static bool s_din_is_top = LED_DIN_IS_TOP;

extern "C" {

void led_controller_init(uint8_t pin, uint16_t num_leds, bool din_is_top) {
    s_num_leds = num_leds;
    s_din_is_top = din_is_top;

    if (strip != nullptr) {
        delete strip;
    }

    strip = new Adafruit_NeoPixel(s_num_leds, pin, NEO_GRB + NEO_KHZ800);
    strip->begin();
    strip->setBrightness(255); // 全体輝度はilluminationモジュール側でスケーリング
    strip->show();             // 初期消灯
}

void led_controller_set_brightness(uint8_t brightness) {
    if (strip) {
        strip->setBrightness(brightness);
    }
}

void led_controller_apply_buffer(const rgb_color_t* buffer, int count) {
    if (!strip || !buffer) return;

    int render_count = (count < s_num_leds) ? count : s_num_leds;

    for (int i = 0; i < render_count; i++) {
        // DINが上端の場合はそのまま、下端の場合は物理LEDインデックスを反転
        int physical_idx = s_din_is_top ? i : (s_num_leds - 1 - i);
        strip->setPixelColor(physical_idx, strip->Color(buffer[i].r, buffer[i].g, buffer[i].b));
    }
    strip->show();
}

void led_controller_clear(void) {
    if (strip) {
        strip->clear();
        strip->show();
    }
}

} // extern "C"
