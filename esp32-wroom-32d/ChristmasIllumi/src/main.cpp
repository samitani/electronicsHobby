#include <Arduino.h>
#include "config.h"
#include "illumination.h"
#include "led_controller.h"
#include "web_server.h"

static illumination_t g_illumi;
static rgb_color_t g_led_buffer[LED_COUNT];
static uint32_t g_last_render_ms = 0;

static void sync_state_to_clients() {
    web_server_broadcast_state(g_illumi.power_on,
                             (int)g_illumi.pattern,
                             g_illumi.brightness,
                             g_illumi.speed_factor,
                             g_illumi.auto_cycle);
}

// スマホからのWebSocketコマンド受信時
static void on_web_command(const char* cmd) {
    if (strncmp(cmd, "SET_PATTERN ", 12) == 0) {
        int pattern = atoi(cmd + 12);
        illumination_set_pattern(&g_illumi, (illumi_pattern_t)pattern);
        Serial.printf("[Illumi] Pattern: %s\n", illumination_get_pattern_name((illumi_pattern_t)pattern));
        sync_state_to_clients();
    } else if (strncmp(cmd, "SET_POWER ", 10) == 0) {
        int power = atoi(cmd + 10);
        illumination_set_power(&g_illumi, power != 0);
        Serial.printf("[Illumi] Power: %s\n", g_illumi.power_on ? "ON" : "OFF");
        sync_state_to_clients();
    } else if (strncmp(cmd, "SET_BRIGHTNESS ", 15) == 0) {
        int b = atoi(cmd + 15);
        if (b < 0) b = 0;
        if (b > 255) b = 255;
        illumination_set_brightness(&g_illumi, (uint8_t)b);
        sync_state_to_clients();
    } else if (strncmp(cmd, "SET_SPEED ", 10) == 0) {
        float spd = (float)atof(cmd + 10);
        illumination_set_speed(&g_illumi, spd);
        sync_state_to_clients();
    } else if (strncmp(cmd, "SET_AUTO_CYCLE ", 15) == 0) {
        int ac = atoi(cmd + 15);
        illumination_set_auto_cycle(&g_illumi, ac != 0, 20); // 20秒周期
        Serial.printf("[Illumi] Auto Cycle: %s\n", g_illumi.auto_cycle ? "ON" : "OFF");
        sync_state_to_clients();
    } else if (strcmp(cmd, "GET_STATE") == 0) {
        sync_state_to_clients();
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n========================================");
    Serial.println("   🎄 Christmas Illumination 2026 🎄   ");
    Serial.println("========================================");

    // 1. LEDコントローラー初期化
    Serial.printf("[LED] Initializing %d LEDs on GPIO %d (DIN top: %s)...\n",
                  LED_COUNT, LED_GPIO, LED_DIN_IS_TOP ? "YES" : "NO");
    led_controller_init(LED_GPIO, LED_COUNT, LED_DIN_IS_TOP);

    // 起動時テスト点灯 (赤 -> 緑 -> 消灯)
    for (int i = 0; i < LED_COUNT; i++) g_led_buffer[i] = (rgb_color_t){20, 0, 0};
    led_controller_apply_buffer(g_led_buffer, LED_COUNT);
    delay(200);
    for (int i = 0; i < LED_COUNT; i++) g_led_buffer[i] = (rgb_color_t){0, 20, 0};
    led_controller_apply_buffer(g_led_buffer, LED_COUNT);
    delay(200);
    led_controller_clear();

    // 2. イルミネーションエンジン初期化
    Serial.println("[Illumi] Initializing Illumination engine...");
    illumination_init(&g_illumi, LED_COUNT);

    // 3. Wi-Fi STA & Web/WebSocketサーバー初期化
    web_server_init(WIFI_SSID, WIFI_PASS, on_web_command);

    g_last_render_ms = millis();
    Serial.println("[System] Setup completed. Let there be light!");
}

void loop() {
    // 1. Web/WebSocket通信の処理
    web_server_loop();

    uint32_t now = millis();

    // 2. アニメーション更新
    illumination_update(&g_illumi, now);

    // 3. LED描画 (約60 FPS = 16ms周期)
    if (now - g_last_render_ms >= 16) {
        g_last_render_ms = now;
        illumination_render(&g_illumi, g_led_buffer, LED_COUNT);
        led_controller_apply_buffer(g_led_buffer, LED_COUNT);
    }
}
