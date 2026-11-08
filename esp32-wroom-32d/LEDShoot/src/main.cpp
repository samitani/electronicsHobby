#include <Arduino.h>
#include "config.h"
#include "game_types.h"
#include "timing_game.h"
#include "shooting_game.h"
#include "led_controller.h"
#include "web_server.h"

static game_mode_t g_current_mode = GAME_MODE_TIMING;
static timing_game_t g_timing_game;
static shooting_game_t g_shooting_game;
static rgb_color_t g_led_buffer[LED_COUNT];
static uint32_t g_last_update_ms = 0;
static uint32_t g_last_render_ms = 0;

// ゲームイベントが発生した際のコールバック (ESP32 -> スマホブラウザ)
static void on_game_event(const char* event_type, int v1, int v2) {
    web_server_broadcast_event(event_type, v1, v2);
}

// スマホブラウザからWebSocketメッセージを受信した際のコールバック
static void on_web_command(const char* cmd) {
    if (strcmp(cmd, "SET_MODE TIMING") == 0) {
        g_current_mode = GAME_MODE_TIMING;
        timing_game_init(&g_timing_game, on_game_event);
        led_controller_clear();
        Serial.println("[Mode] Switched to TIMING GAME");
    } else if (strcmp(cmd, "SET_MODE SHOOTING") == 0) {
        g_current_mode = GAME_MODE_SHOOTING;
        shooting_game_init(&g_shooting_game, LED_COUNT, on_game_event);
        led_controller_clear();
        Serial.println("[Mode] Switched to COLOR SHOOTING");
    } else if (strcmp(cmd, "START") == 0) {
        if (g_current_mode == GAME_MODE_TIMING) {
            timing_game_start(&g_timing_game);
        } else {
            shooting_game_start(&g_shooting_game);
        }
    } else if (strcmp(cmd, "RESTART") == 0) {
        if (g_current_mode == GAME_MODE_TIMING) {
            timing_game_restart(&g_timing_game);
        } else {
            shooting_game_restart(&g_shooting_game);
        }
    } else if (strcmp(cmd, "PRESS_RED") == 0) {
        if (g_current_mode == GAME_MODE_TIMING) {
            timing_game_handle_input(&g_timing_game, GAME_COLOR_RED);
        } else {
            shooting_game_handle_input(&g_shooting_game, GAME_COLOR_RED);
        }
    } else if (strcmp(cmd, "PRESS_GREEN") == 0) {
        if (g_current_mode == GAME_MODE_TIMING) {
            timing_game_handle_input(&g_timing_game, GAME_COLOR_GREEN);
        } else {
            shooting_game_handle_input(&g_shooting_game, GAME_COLOR_GREEN);
        }
    } else if (strcmp(cmd, "PRESS_BLUE") == 0) {
        if (g_current_mode == GAME_MODE_TIMING) {
            timing_game_handle_input(&g_timing_game, GAME_COLOR_BLUE);
        } else {
            shooting_game_handle_input(&g_shooting_game, GAME_COLOR_BLUE);
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=================================");
    Serial.println("   LEDShoot - Vertical LED Game  ");
    Serial.println("=================================");

    // 1. LEDコントローラー初期化
    Serial.printf("[LED] Initializing %d LEDs on GPIO %d (DIN top: %s)...\n",
                  LED_COUNT, LED_GPIO, LED_DIN_IS_TOP ? "YES" : "NO");
    led_controller_init(LED_GPIO, LED_COUNT, LED_DIN_IS_TOP);

    // 起動時の初期点灯演出（一瞬白色で光って消灯）
    for (int i = 0; i < LED_COUNT; i++) {
        g_led_buffer[i] = (rgb_color_t){10, 10, 10};
    }
    led_controller_apply_buffer(g_led_buffer, LED_COUNT);
    delay(200);
    led_controller_clear();

    // 2. ゲームエンジン初期化
    Serial.println("[Game] Initializing Timing Game & Shooting Game engines...");
    timing_game_init(&g_timing_game, on_game_event);
    shooting_game_init(&g_shooting_game, LED_COUNT, on_game_event);

    // 3. Wi-Fi STA & Web/WebSocketサーバー初期化
    web_server_init(WIFI_SSID, WIFI_PASS, on_web_command);

    g_last_update_ms = millis();
    g_last_render_ms = millis();
    Serial.println("[System] Setup completed. Ready to play!");
}

void loop() {
    // 1. Web/WebSocket通信の処理
    web_server_loop();

    uint32_t now = millis();
    uint32_t delta = now - g_last_update_ms;
    g_last_update_ms = now;

    // 2. 現在のモードのゲームロジック更新
    if (g_current_mode == GAME_MODE_TIMING) {
        timing_game_update(&g_timing_game, now, delta);
    } else if (g_current_mode == GAME_MODE_SHOOTING) {
        shooting_game_update(&g_shooting_game, now, delta);
    }

    // 3. LED描画の更新 (約60 FPS = 16ms周期)
    if (now - g_last_render_ms >= 16) {
        g_last_render_ms = now;
        if (g_current_mode == GAME_MODE_TIMING) {
            timing_game_render(&g_timing_game, g_led_buffer, LED_COUNT);
        } else if (g_current_mode == GAME_MODE_SHOOTING) {
            shooting_game_render(&g_shooting_game, g_led_buffer, LED_COUNT);
        }
        led_controller_apply_buffer(g_led_buffer, LED_COUNT);
    }
}
