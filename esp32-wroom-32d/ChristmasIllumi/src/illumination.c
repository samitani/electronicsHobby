#include "illumination.h"
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PI_VAL 3.14159265f

// HSV (h:0-360, s:0-1, v:0-1) -> RGB (0-255)
static rgb_color_t hsv_to_rgb(float h, float s, float v) {
    while (h < 0.0f) h += 360.0f;
    while (h >= 360.0f) h -= 360.0f;

    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(h / 60.0f, 2.0f) - 1.0f));
    float m = v - c;

    float r1 = 0, g1 = 0, b1 = 0;
    if (h < 60.0f)       { r1 = c; g1 = x; b1 = 0; }
    else if (h < 120.0f) { r1 = x; g1 = c; b1 = 0; }
    else if (h < 180.0f) { r1 = 0; g1 = c; b1 = x; }
    else if (h < 240.0f) { r1 = 0; g1 = x; b1 = c; }
    else if (h < 300.0f) { r1 = x; g1 = 0; b1 = c; }
    else                 { r1 = c; g1 = 0; b1 = x; }

    rgb_color_t rgb;
    rgb.r = (uint8_t)((r1 + m) * 255.0f);
    rgb.g = (uint8_t)((g1 + m) * 255.0f);
    rgb.b = (uint8_t)((b1 + m) * 255.0f);
    return rgb;
}

// 暖炉の炎用カラーパレット (heat: 0 - 255)
static rgb_color_t heat_to_color(uint8_t heat) {
    rgb_color_t c = {0, 0, 0};
    if (heat == 0) return c;

    // 0 - 85: 黒 -> 赤
    if (heat < 85) {
        c.r = heat * 3;
        c.g = 0;
        c.b = 0;
    }
    // 85 - 170: 赤 -> オレンジ/黄
    else if (heat < 170) {
        c.r = 255;
        c.g = (heat - 85) * 3;
        c.b = 0;
    }
    // 170 - 255: 黄 -> 白
    else {
        c.r = 255;
        c.g = 255;
        c.b = (heat - 170) * 3;
    }
    return c;
}

void illumination_init(illumination_t* illumi, int led_count) {
    memset(illumi, 0, sizeof(illumination_t));
    illumi->led_count = (led_count > 0 && led_count <= MAX_ILLUMI_LEDS) ? led_count : 60;
    illumi->pattern = PATTERN_CANDY_CANE;
    illumi->power_on = true;
    illumi->brightness = DEFAULT_BRIGHTNESS; // 初期輝度約30%
    illumi->speed_factor = 1.0f;
    illumi->auto_cycle = false;
    illumi->auto_cycle_interval_ms = 20000; // 20秒
    illumi->last_cycle_time_ms = 0;
    illumi->last_update_ms = 0;
    illumi->anim_phase = 0.0f;

    // 雪片の初期化
    for (int i = 0; i < MAX_SNOWFLAKES; i++) {
        illumi->snowflakes[i].active = true;
        illumi->snowflakes[i].position = (float)(rand() % illumi->led_count);
        illumi->snowflakes[i].speed = 6.0f + (float)(rand() % 14); // 6 - 20 LED/s
        illumi->snowflakes[i].brightness = 0.7f + 0.3f * ((float)rand() / (float)RAND_MAX);
    }

    // ツリーオーナメントの初期化
    for (int i = 0; i < illumi->led_count; i++) {
        illumi->ornament_phase[i] = ((float)rand() / (float)RAND_MAX) * 2.0f * PI_VAL;
        illumi->ornament_color[i] = (uint8_t)(rand() % 4); // 0:金, 1:赤, 2:青, 3:白
    }

    // 暖炉ヒートバッファ初期化
    for (int i = 0; i < illumi->led_count; i++) {
        illumi->fire_heat[i] = 0;
        illumi->champagne_sparkle[i] = 0.0f;
    }

    // 流れ星の初期化
    for (int m = 0; m < MAX_METEORS; m++) {
        illumi->meteors[m].active = false;
        illumi->meteors[m].position = 0.0f;
        illumi->meteors[m].speed = 30.0f;
        illumi->meteors[m].length = 10.0f;
        illumi->meteors[m].color_type = (uint8_t)m;
        illumi->meteors[m].wait_timer_ms = (uint32_t)(m * 800 + 200);
    }

    // サンタのソリの初期化
    illumi->sleigh.active = true;
    illumi->sleigh.position = 0.0f;
    illumi->sleigh.speed = 22.0f;
    illumi->sleigh.wait_timer_ms = 0;
}

void illumination_set_pattern(illumination_t* illumi, illumi_pattern_t pattern) {
    if (pattern < PATTERN_COUNT) {
        illumi->pattern = pattern;
    }
}

void illumination_set_power(illumination_t* illumi, bool power_on) {
    illumi->power_on = power_on;
}

void illumination_set_brightness(illumination_t* illumi, uint8_t brightness) {
    illumi->brightness = brightness;
}

void illumination_set_speed(illumination_t* illumi, float speed_factor) {
    if (speed_factor < 0.2f) speed_factor = 0.2f;
    if (speed_factor > 4.0f) speed_factor = 4.0f;
    illumi->speed_factor = speed_factor;
}

void illumination_set_auto_cycle(illumination_t* illumi, bool enabled, uint32_t interval_sec) {
    illumi->auto_cycle = enabled;
    if (interval_sec < 5) interval_sec = 5;
    illumi->auto_cycle_interval_ms = interval_sec * 1000;
}

const char* illumination_get_pattern_name(illumi_pattern_t pattern) {
    switch (pattern) {
        case PATTERN_CANDY_CANE:   return "Candy Cane";
        case PATTERN_SNOWFALL:     return "Snowfall";
        case PATTERN_TREE_TWINKLE: return "Christmas Tree";
        case PATTERN_AURORA:       return "Silent Night Aurora";
        case PATTERN_FIREPLACE:    return "Cozy Fireplace";
        case PATTERN_RAINBOW:      return "Holiday Rainbow";
        case PATTERN_CHAMPAGNE:    return "Champagne Sparkle";
        case PATTERN_METEOR:       return "Holiday Meteor";
        case PATTERN_SANTA:        return "Santa's Sleigh";
        default:                   return "Unknown";
    }
}

void illumination_update(illumination_t* illumi, uint32_t current_time_ms) {
    if (illumi->last_update_ms == 0) {
        illumi->last_update_ms = current_time_ms;
        illumi->last_cycle_time_ms = current_time_ms;
        return;
    }

    uint32_t delta_ms = current_time_ms - illumi->last_update_ms;
    illumi->last_update_ms = current_time_ms;
    float dt = (float)delta_ms / 1000.0f;

    // 自動巡回 (Auto Cycle) の判定
    if (illumi->auto_cycle && illumi->power_on) {
        if (current_time_ms - illumi->last_cycle_time_ms >= illumi->auto_cycle_interval_ms) {
            illumi->last_cycle_time_ms = current_time_ms;
            illumi->pattern = (illumi_pattern_t)((illumi->pattern + 1) % PATTERN_COUNT);
        }
    }

    // アニメーション位相の進行
    illumi->anim_phase += dt * illumi->speed_factor;

    // パターン個別更新
    if (illumi->pattern == PATTERN_SNOWFALL) {
        for (int i = 0; i < MAX_SNOWFLAKES; i++) {
            illumi->snowflakes[i].position += illumi->snowflakes[i].speed * dt * illumi->speed_factor;
            if (illumi->snowflakes[i].position >= (float)illumi->led_count) {
                illumi->snowflakes[i].position = 0.0f;
                illumi->snowflakes[i].speed = 6.0f + (float)(rand() % 14);
                illumi->snowflakes[i].brightness = 0.7f + 0.3f * ((float)rand() / (float)RAND_MAX);
            }
        }
    } else if (illumi->pattern == PATTERN_FIREPLACE) {
        // 暖炉アルゴリズム: 冷却 & 拡散
        int n = illumi->led_count;
        for (int i = 0; i < n; i++) {
            int cooldown = rand() % 8;
            if (illumi->fire_heat[i] > cooldown) {
                illumi->fire_heat[i] -= cooldown;
            } else {
                illumi->fire_heat[i] = 0;
            }
        }
        for (int i = n - 1; i >= 2; i--) {
            illumi->fire_heat[i] = (uint8_t)(((int)illumi->fire_heat[i - 1] + (int)illumi->fire_heat[i - 2] + (int)illumi->fire_heat[i - 2]) / 3);
        }
        // 下端に火種を投入
        if (rand() % 100 < 80) {
            int spark = rand() % 3;
            int heat = 180 + rand() % 75;
            if (spark < n) {
                illumi->fire_heat[spark] = (uint8_t)heat;
            }
        }
    } else if (illumi->pattern == PATTERN_CHAMPAGNE) {
        // シャンパンスパークルの減衰と泡の生成
        for (int i = 0; i < illumi->led_count; i++) {
            if (illumi->champagne_sparkle[i] > 0.0f) {
                illumi->champagne_sparkle[i] -= dt * 2.8f * illumi->speed_factor;
                if (illumi->champagne_sparkle[i] < 0.0f) illumi->champagne_sparkle[i] = 0.0f;
            }
        }
        if (rand() % 100 < 35) {
            int idx = rand() % illumi->led_count;
            illumi->champagne_sparkle[idx] = 1.0f;
        }
    } else if (illumi->pattern == PATTERN_METEOR) {
        // 流れ星の移動と再出現
        for (int m = 0; m < MAX_METEORS; m++) {
            if (illumi->meteors[m].active) {
                illumi->meteors[m].position += illumi->meteors[m].speed * dt * illumi->speed_factor;
                if (illumi->meteors[m].position - illumi->meteors[m].length >= (float)illumi->led_count) {
                    illumi->meteors[m].active = false;
                    illumi->meteors[m].wait_timer_ms = 400 + (rand() % 1200);
                }
            } else {
                if (illumi->meteors[m].wait_timer_ms > delta_ms) {
                    illumi->meteors[m].wait_timer_ms -= delta_ms;
                } else {
                    illumi->meteors[m].active = true;
                    illumi->meteors[m].position = 0.0f;
                    illumi->meteors[m].speed = 28.0f + (float)(rand() % 16);
                    illumi->meteors[m].length = 8.0f + (float)(rand() % 6);
                    illumi->meteors[m].color_type = (uint8_t)(rand() % 2);
                }
            }
        }
    } else if (illumi->pattern == PATTERN_SANTA) {
        // サンタのソリの飛行と再出現
        if (illumi->sleigh.active) {
            illumi->sleigh.position += illumi->sleigh.speed * dt * illumi->speed_factor;
            if (illumi->sleigh.position - 18.0f >= (float)illumi->led_count) {
                illumi->sleigh.active = false;
                illumi->sleigh.wait_timer_ms = 1800; // 1.8秒のインターバル
            }
        } else {
            if (illumi->sleigh.wait_timer_ms > delta_ms) {
                illumi->sleigh.wait_timer_ms -= delta_ms;
            } else {
                illumi->sleigh.active = true;
                illumi->sleigh.position = 0.0f;
                illumi->sleigh.speed = 20.0f + (float)(rand() % 8);
            }
        }
    }
}

void illumination_render(const illumination_t* illumi, rgb_color_t* led_buffer, int led_count) {
    if (!led_buffer || led_count <= 0) return;

    // 電源OFFの場合は全消灯
    if (!illumi->power_on) {
        memset(led_buffer, 0, sizeof(rgb_color_t) * led_count);
        return;
    }

    int n = (led_count < illumi->led_count) ? led_count : illumi->led_count;

    switch (illumi->pattern) {
        case PATTERN_CANDY_CANE: {
            // 赤と白のストライプが流れる
            float offset = illumi->anim_phase * 8.0f;
            for (int i = 0; i < n; i++) {
                int stripe = (int)(i + offset) % 10;
                if (stripe < 0) stripe += 10;

                if (stripe < 5) {
                    led_buffer[i] = (rgb_color_t){255, 0, 0};      // 赤
                } else if (stripe < 9) {
                    led_buffer[i] = (rgb_color_t){255, 255, 255};  // 白
                } else {
                    led_buffer[i] = (rgb_color_t){0, 180, 25};     // アクセント緑
                }
            }
            break;
        }

        case PATTERN_SNOWFALL: {
            // 背景: 静かな夜空の深い青
            for (int i = 0; i < n; i++) {
                led_buffer[i] = (rgb_color_t){0, 1, 6};
            }
            // 雪片の描画
            for (int s = 0; s < MAX_SNOWFLAKES; s++) {
                int pos = (int)roundf(illumi->snowflakes[s].position);
                if (pos >= 0 && pos < n) {
                    uint8_t b = (uint8_t)(255.0f * illumi->snowflakes[s].brightness);
                    led_buffer[pos] = (rgb_color_t){b, b, b};
                    // 上下にうっすら残光
                    if (pos > 0) {
                        led_buffer[pos - 1] = (rgb_color_t){(uint8_t)(b / 4), (uint8_t)(b / 4), (uint8_t)(b / 3)};
                    }
                    if (pos < n - 1) {
                        led_buffer[pos + 1] = (rgb_color_t){(uint8_t)(b / 4), (uint8_t)(b / 4), (uint8_t)(b / 3)};
                    }
                }
            }
            break;
        }

        case PATTERN_TREE_TWINKLE: {
            // モミの木の深緑と光るカラフルオーナメント
            for (int i = 0; i < n; i++) {
                if (i % 4 == 0) {
                    // オーナメントLED (呼吸点灯)
                    float wave = sinf(illumi->anim_phase * 2.5f + illumi->ornament_phase[i]);
                    float factor = 0.25f + 0.75f * ((wave + 1.0f) * 0.5f); // 0.25 - 1.0

                    rgb_color_t oc = {0, 0, 0};
                    switch (illumi->ornament_color[i]) {
                        case 0: oc = (rgb_color_t){255, 200, 30};  break; // ゴールド
                        case 1: oc = (rgb_color_t){255, 20, 30};   break; // ルビーレッド
                        case 2: oc = (rgb_color_t){0, 140, 255};   break; // サファイアブルー
                        case 3: oc = (rgb_color_t){255, 255, 255}; break; // クリスタルホワイト
                    }
                    led_buffer[i] = (rgb_color_t){
                        (uint8_t)(oc.r * factor),
                        (uint8_t)(oc.g * factor),
                        (uint8_t)(oc.b * factor)
                    };
                } else {
                    // ツリーの深緑 (かすかに揺らぐ)
                    float g_wave = 0.8f + 0.2f * sinf(illumi->anim_phase * 1.0f + (float)i * 0.3f);
                    led_buffer[i] = (rgb_color_t){0, (uint8_t)(45.0f * g_wave), (uint8_t)(10.0f * g_wave)};
                }
            }
            break;
        }

        case PATTERN_AURORA: {
            // 幻想的なクリスマスオーロラ
            for (int i = 0; i < n; i++) {
                float wave1 = sinf((float)i * 0.12f + illumi->anim_phase * 1.2f);
                float wave2 = cosf((float)i * 0.08f - illumi->anim_phase * 0.7f);
                float hue = 140.0f + (wave1 + wave2) * 55.0f; // 緑〜シアン〜青〜紫
                float val = 0.35f + 0.65f * ((wave1 + 1.0f) * 0.5f);
                led_buffer[i] = hsv_to_rgb(hue, 0.95f, val);
            }
            break;
        }

        case PATTERN_FIREPLACE: {
            // 暖炉の炎
            for (int i = 0; i < n; i++) {
                led_buffer[i] = heat_to_color(illumi->fire_heat[i]);
            }
            break;
        }

        case PATTERN_RAINBOW: {
            // ホリデーレインボー
            for (int i = 0; i < n; i++) {
                float hue = fmodf((float)i * 6.0f + illumi->anim_phase * 50.0f, 360.0f);
                led_buffer[i] = hsv_to_rgb(hue, 1.0f, 0.9f);
            }
            break;
        }

        case PATTERN_CHAMPAGNE: {
            // シャンパンゴールドと気泡のスパークル
            for (int i = 0; i < n; i++) {
                float base_wave = 0.7f + 0.3f * sinf(illumi->anim_phase * 1.5f + (float)i * 0.4f);
                float spk = illumi->champagne_sparkle[i]; // 0.0 - 1.0

                // ベース: 穏やかなウォームゴールド (32, 24, 7)
                // スパークル: きらめく白金 (255, 220, 110)
                uint8_t r = (uint8_t)((32.0f * base_wave) + (223.0f * spk));
                uint8_t g = (uint8_t)((24.0f * base_wave) + (196.0f * spk));
                uint8_t b = (uint8_t)((7.0f  * base_wave) + (103.0f * spk));
                led_buffer[i] = (rgb_color_t){r, g, b};
            }
            break;
        }

        case PATTERN_METEOR: {
            // 静寂な冬の夜空 (濃紺)
            for (int i = 0; i < n; i++) {
                led_buffer[i] = (rgb_color_t){0, 1, 4};
            }
            // 流れ星の描画 (先頭は白、尾はゴールド/アイスブルー)
            for (int m = 0; m < MAX_METEORS; m++) {
                if (illumi->meteors[m].active) {
                    float head = illumi->meteors[m].position;
                    float len = illumi->meteors[m].length;
                    uint8_t ctype = illumi->meteors[m].color_type;

                    for (int i = 0; i < n; i++) {
                        float dist = head - (float)i;
                        if (dist >= 0.0f && dist <= len) {
                            float decay = 1.0f - (dist / len);
                            decay = decay * decay; // 2乗減衰

                            uint8_t mr, mg, mb;
                            if (dist < 1.0f) {
                                mr = 255; mg = 255; mb = 255; // 先頭は純白
                            } else if (ctype == 0) {
                                // ゴールド流星
                                mr = (uint8_t)(255 * decay);
                                mg = (uint8_t)(190 * decay);
                                mb = (uint8_t)(40 * decay);
                            } else {
                                // アイスブルー流星
                                mr = (uint8_t)(120 * decay);
                                mg = (uint8_t)(220 * decay);
                                mb = (uint8_t)(255 * decay);
                            }
                            // 加算合成
                            int tr = led_buffer[i].r + mr;
                            int tg = led_buffer[i].g + mg;
                            int tb = led_buffer[i].b + mb;
                            led_buffer[i].r = (tr > 255) ? 255 : (uint8_t)tr;
                            led_buffer[i].g = (tg > 255) ? 255 : (uint8_t)tg;
                            led_buffer[i].b = (tb > 255) ? 255 : (uint8_t)tb;
                        }
                    }
                }
            }
            break;
        }

        case PATTERN_SANTA: {
            // 夜空のベース
            for (int i = 0; i < n; i++) {
                led_buffer[i] = (rgb_color_t){0, 2, 8};
            }
            if (illumi->sleigh.active) {
                float p = illumi->sleigh.position;
                for (int i = 0; i < n; i++) {
                    float dist = p - (float)i;
                    if (dist >= 0.0f && dist < 1.0f) {
                        // 先頭: 赤鼻のルドルフ (ピカピカ点滅)
                        float blink = 0.75f + 0.25f * sinf(illumi->anim_phase * 20.0f);
                        led_buffer[i] = (rgb_color_t){(uint8_t)(255 * blink), 0, 0};
                    } else if (dist >= 1.0f && dist < 3.0f) {
                        // トナカイの鈴 (ゴールド)
                        led_buffer[i] = (rgb_color_t){255, 190, 20};
                    } else if (dist >= 3.0f && dist < 5.0f) {
                        // サンタのソリ (真紅)
                        led_buffer[i] = (rgb_color_t){240, 15, 20};
                    } else if (dist >= 5.0f && dist < 18.0f) {
                        // 魔法の星屑トレイル (エメラルド＆ゴールドのきらめき)
                        float decay = 1.0f - ((dist - 5.0f) / 13.0f);
                        float spk = 0.5f + 0.5f * sinf((float)i * 3.5f + illumi->anim_phase * 15.0f);
                        float intensity = decay * spk;

                        if (i % 2 == 0) {
                            led_buffer[i] = (rgb_color_t){(uint8_t)(255 * intensity), (uint8_t)(210 * intensity), (uint8_t)(40 * intensity)};
                        } else {
                            led_buffer[i] = (rgb_color_t){(uint8_t)(20 * intensity), (uint8_t)(255 * intensity), (uint8_t)(100 * intensity)};
                        }
                    }
                }
            }
            break;
        }

        default:
            memset(led_buffer, 0, sizeof(rgb_color_t) * led_count);
            break;
    }

    // 全体輝度（brightness）のスケーリング適用
    if (illumi->brightness < 255) {
        float b_scale = (float)illumi->brightness / 255.0f;
        for (int i = 0; i < n; i++) {
            led_buffer[i].r = (uint8_t)((float)led_buffer[i].r * b_scale);
            led_buffer[i].g = (uint8_t)((float)led_buffer[i].g * b_scale);
            led_buffer[i].b = (uint8_t)((float)led_buffer[i].b * b_scale);
        }
    }
}
