#ifndef SHOOTING_GAME_H
#define SHOOTING_GAME_H

#include "game_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// 落下するターゲット光
typedef struct {
    float position;        // 現在位置 (0.0f = 上端)
    float speed;           // 落下速度 (LEDs/秒)
    game_color_t color;    // 色
    bool active;           // 有効フラグ
} shooting_target_t;

// プレイヤーが発射する光（弾）
typedef struct {
    float position;        // 現在位置 (下端から上端へ)
    float speed;           // 上昇速度 (LEDs/秒)
    game_color_t color;    // 色
    bool active;           // 有効フラグ
} shooting_bullet_t;

#define MAX_SHOOTING_TARGETS 24
#define MAX_SHOOTING_BULLETS 16

// カラーシューティング ゲームコンテキスト
typedef struct {
    game_state_t state;
    int score;
    int max_streak;        // 連続撃破数
    int streak;

    // ターゲットおよび弾の管理
    shooting_target_t targets[MAX_SHOOTING_TARGETS];
    shooting_bullet_t bullets[MAX_SHOOTING_BULLETS];

    uint32_t last_spawn_time_ms;
    uint32_t spawn_interval_ms;  // 出現間隔 (ms)
    float target_speed;          // 現在の落下速度 (LEDs/秒)
    float bullet_speed;          // 弾の速度 (LEDs/秒)

    int targets_spawned;
    int led_count;

    // 演出用タイマー
    int effect_hit_timer_ms;
    int effect_hit_pos;
    int effect_miss_timer_ms;

    // イベント通知コールバック
    game_event_cb_t event_cb;
} shooting_game_t;

// 関数プロトタイプ
void shooting_game_init(shooting_game_t* game, int led_count, game_event_cb_t cb);
void shooting_game_start(shooting_game_t* game);
void shooting_game_restart(shooting_game_t* game);
void shooting_game_update(shooting_game_t* game, uint32_t current_time_ms, uint32_t delta_ms);
void shooting_game_handle_input(shooting_game_t* game, game_color_t color);
void shooting_game_render(const shooting_game_t* game, rgb_color_t* led_buffer, int led_count);

#ifdef __cplusplus
}
#endif

#endif // SHOOTING_GAME_H
