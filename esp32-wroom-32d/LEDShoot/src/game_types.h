#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ゲームで使用する色の定義
typedef enum {
    GAME_COLOR_NONE = 0,
    GAME_COLOR_RED,
    GAME_COLOR_GREEN,
    GAME_COLOR_BLUE
} game_color_t;

// ゲーム状態
typedef enum {
    GAME_STATE_IDLE = 0,
    GAME_STATE_PLAYING,
    GAME_STATE_GAMEOVER
} game_state_t;

// ゲームモード
typedef enum {
    GAME_MODE_TIMING = 0,
    GAME_MODE_SHOOTING = 1,
    GAME_MODE_BATTLE = 2
} game_mode_t;

// 単一LEDのRGB色データ
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_color_t;

// イベント通知用コールバック型定義
typedef void (*game_event_cb_t)(const char* event_type, int value1, int value2);

#ifdef __cplusplus
}
#endif

#endif // GAME_TYPES_H
