#ifndef ILLUMINATION_H
#define ILLUMINATION_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// 単一LEDのRGBデータ
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_color_t;

// イルミネーションパターン種別
typedef enum {
    PATTERN_CANDY_CANE = 0,        // 赤と白のキャンディケインストライプ
    PATTERN_SNOWFALL = 1,          // 舞い落ちる雪ときらめく夜空
    PATTERN_TREE_TWINKLE = 2,      // モミの木の深緑と光るオーナメント
    PATTERN_AURORA = 3,            // 幻想的なクリスマスオーロラ
    PATTERN_FIREPLACE = 4,         // 温かい暖炉の炎の揺らめき
    PATTERN_RAINBOW = 5,           // ホリデーレインボースクロール
    PATTERN_CHAMPAGNE = 6,         // シャンパンゴールドの上品な瞬き
    PATTERN_METEOR = 7,            // 聖夜の流れ星（ゴールド＆アイスブルー）
    PATTERN_SANTA = 8,             // サンタのソリ（赤鼻のトナカイ＆魔法の星屑）
    PATTERN_COUNT = 9
} illumi_pattern_t;

// 降雪用フレーク構造体
typedef struct {
    float position;
    float speed;
    float brightness;
    bool active;
} snowflake_t;

// 流れ星構造体
typedef struct {
    float position;
    float speed;
    float length;
    uint8_t color_type; // 0: ゴールド, 1: アイスブルー
    bool active;
    uint32_t wait_timer_ms;
} meteor_t;

// サンタのソリ構造体
typedef struct {
    float position;
    float speed;
    bool active;
    uint32_t wait_timer_ms;
} sleigh_t;

#define MAX_SNOWFLAKES 12
#define MAX_METEORS 2
#define MAX_ILLUMI_LEDS 300

// イルミネーション制御コンテキスト
typedef struct {
    illumi_pattern_t pattern;
    bool power_on;
    uint8_t brightness;        // 0 - 255
    float speed_factor;        // 0.5f - 3.0f (標準 1.0f)
    int led_count;

    // 自動巡回 (Auto Cycle)
    bool auto_cycle;
    uint32_t auto_cycle_interval_ms;
    uint32_t last_cycle_time_ms;

    // 内部アニメーション状態
    float anim_phase;          // 累積アニメーション位相
    uint32_t last_update_ms;

    // パターン別状態データ
    snowflake_t snowflakes[MAX_SNOWFLAKES];
    float ornament_phase[MAX_ILLUMI_LEDS];  // ツリー用オーナメント個別位相
    uint8_t ornament_color[MAX_ILLUMI_LEDS];// ツリー用オーナメント色タイプ
    uint8_t fire_heat[MAX_ILLUMI_LEDS];     // 暖炉用熱量バッファ
    float champagne_sparkle[MAX_ILLUMI_LEDS];// シャンパン用明滅強度 (0.0 - 1.0)
    meteor_t meteors[MAX_METEORS];          // 流れ星
    sleigh_t sleigh;                        // サンタのソリ
} illumination_t;

// 関数プロトタイプ
void illumination_init(illumination_t* illumi, int led_count);
void illumination_update(illumination_t* illumi, uint32_t current_time_ms);
void illumination_render(const illumination_t* illumi, rgb_color_t* led_buffer, int led_count);

void illumination_set_pattern(illumination_t* illumi, illumi_pattern_t pattern);
void illumination_set_power(illumination_t* illumi, bool power_on);
void illumination_set_brightness(illumination_t* illumi, uint8_t brightness);
void illumination_set_speed(illumination_t* illumi, float speed_factor);
void illumination_set_auto_cycle(illumination_t* illumi, bool enabled, uint32_t interval_sec);

const char* illumination_get_pattern_name(illumi_pattern_t pattern);

#ifdef __cplusplus
}
#endif

#endif // ILLUMINATION_H
