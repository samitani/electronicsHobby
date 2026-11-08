#ifndef TIMING_GAME_H
#define TIMING_GAME_H

#include "game_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// 落下する光（ドロップ）
typedef struct {
    float position;        // 現在位置 (0.0f = 上端)
    float speed;           // 落下速度 (LEDs/秒)
    game_color_t color;    // 色
    bool active;           // 有効フラグ
} drop_t;

// 最大同時落下ドロップ数
#define MAX_DROPS 12

// ゲームコンテキスト構造体
typedef struct {
    game_state_t state;
    int score;
    int streak;
    int max_streak;
    int life;

    // ドロップ管理
    drop_t drops[MAX_DROPS];
    uint32_t last_spawn_time_ms;
    uint32_t spawn_interval_ms;  // 現在の出現間隔 (ms)
    float current_speed;         // 現在の落下速度 (LEDs/秒)

    // 難易度調整パラメータ
    int drops_spawned;

    // 演出タイマー (ms)
    int effect_hit_timer_ms;
    int effect_miss_timer_ms;

    // 判定設定
    int target_center_led;       // ターゲット中心 (例: 57)
    float target_tolerance;      // 許容幅 (例: 1.5f => ±1 LED)

    // イベント通知コールバック
    game_event_cb_t event_cb;
} timing_game_t;

// 関数プロトタイプ
void timing_game_init(timing_game_t* game, game_event_cb_t cb);
void timing_game_start(timing_game_t* game);
void timing_game_restart(timing_game_t* game);
void timing_game_update(timing_game_t* game, uint32_t current_time_ms, uint32_t delta_ms);
void timing_game_handle_input(timing_game_t* game, game_color_t color);

// LEDフレームバッファへの描画関数
void timing_game_render(const timing_game_t* game, rgb_color_t* led_buffer, int led_count);

#ifdef __cplusplus
}
#endif

#endif // TIMING_GAME_H
