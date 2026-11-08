#include "timing_game.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define INITIAL_SPAWN_INTERVAL_MS 2200
#define MIN_SPAWN_INTERVAL_MS     600
#define INTERVAL_DECREASE_STEP    15

#define INITIAL_SPEED             8.0f
#define MAX_SPEED                 32.0f
#define SPEED_INCREASE_STEP       0.3f

#define INITIAL_LIFE              3
#define DEFAULT_TARGET_CENTER     57
#define DEFAULT_TOLERANCE         1.5f

static void fire_event(timing_game_t* game, const char* event_type, int v1, int v2) {
    if (game->event_cb) {
        game->event_cb(event_type, v1, v2);
    }
}

// ドロップの初期化
static void clear_drops(timing_game_t* game) {
    for (int i = 0; i < MAX_DROPS; i++) {
        game->drops[i].active = false;
        game->drops[i].position = 0.0f;
        game->drops[i].speed = 0.0f;
        game->drops[i].color = GAME_COLOR_NONE;
    }
}

// 新規ドロップを生成
static void spawn_drop(timing_game_t* game) {
    for (int i = 0; i < MAX_DROPS; i++) {
        if (!game->drops[i].active) {
            game->drops[i].active = true;
            game->drops[i].position = 0.0f;
            game->drops[i].speed = game->current_speed;

            // ランダムに赤・緑・青を選択
            int r = rand() % 3;
            if (r == 0) game->drops[i].color = GAME_COLOR_RED;
            else if (r == 1) game->drops[i].color = GAME_COLOR_GREEN;
            else game->drops[i].color = GAME_COLOR_BLUE;

            game->drops_spawned++;

            // 難易度調整: 速度アップと出現間隔短縮
            if (game->spawn_interval_ms > MIN_SPAWN_INTERVAL_MS) {
                game->spawn_interval_ms -= INTERVAL_DECREASE_STEP;
                if (game->spawn_interval_ms < MIN_SPAWN_INTERVAL_MS) {
                    game->spawn_interval_ms = MIN_SPAWN_INTERVAL_MS;
                }
            }
            if (game->current_speed < MAX_SPEED) {
                game->current_speed += SPEED_INCREASE_STEP;
                if (game->current_speed > MAX_SPEED) {
                    game->current_speed = MAX_SPEED;
                }
            }
            break;
        }
    }
}

void timing_game_init(timing_game_t* game, game_event_cb_t cb) {
    memset(game, 0, sizeof(timing_game_t));
    game->event_cb = cb;
    game->state = GAME_STATE_IDLE;
    game->target_center_led = DEFAULT_TARGET_CENTER;
    game->target_tolerance = DEFAULT_TOLERANCE;
    clear_drops(game);
}

void timing_game_start(timing_game_t* game) {
    clear_drops(game);
    game->state = GAME_STATE_PLAYING;
    game->score = 0;
    game->streak = 0;
    game->max_streak = 0;
    game->life = INITIAL_LIFE;
    game->spawn_interval_ms = INITIAL_SPAWN_INTERVAL_MS;
    game->current_speed = INITIAL_SPEED;
    game->drops_spawned = 0;
    game->last_spawn_time_ms = 0;
    game->effect_hit_timer_ms = 0;
    game->effect_miss_timer_ms = 0;

    fire_event(game, "GAME_STATE", GAME_STATE_PLAYING, 0);
    fire_event(game, "SCORE_UPDATE", game->score, 0);
    fire_event(game, "STREAK_UPDATE", game->streak, game->max_streak);
    fire_event(game, "LIFE_UPDATE", game->life, 0);
}

void timing_game_restart(timing_game_t* game) {
    timing_game_start(game);
}

void timing_game_handle_input(timing_game_t* game, game_color_t color) {
    if (game->state != GAME_STATE_PLAYING) {
        return;
    }

    // ターゲット位置周辺にあるドロップを検索（最もターゲット中心に近いものを優先）
    int best_index = -1;
    float best_diff = 999.0f;

    for (int i = 0; i < MAX_DROPS; i++) {
        if (game->drops[i].active) {
            float diff = fabsf(game->drops[i].position - (float)game->target_center_led);
            if (diff <= game->target_tolerance && diff < best_diff) {
                best_diff = diff;
                best_index = i;
            }
        }
    }

    if (best_index >= 0 && game->drops[best_index].color == color) {
        // 色とタイミングが一致 -> 成功 (HIT)
        game->drops[best_index].active = false;
        game->streak++;
        if (game->streak > game->max_streak) {
            game->max_streak = game->streak;
        }
        int points = 100 + (game->streak * 10);
        game->score += points;

        game->effect_hit_timer_ms = 180;
        fire_event(game, "HIT", points, 0);
        fire_event(game, "SCORE_UPDATE", game->score, 0);
        fire_event(game, "STREAK_UPDATE", game->streak, game->max_streak);
        return;
    }

    // 色が不一致、またはターゲット範囲内にドロップがない場合
    // ※光は消去せずそのまま進め、LIFEも減少させない（STREAKリセットとMISS演出のみ）
    game->streak = 0;
    game->effect_miss_timer_ms = 200;

    fire_event(game, "MISS", 0, 0);
    fire_event(game, "STREAK_UPDATE", game->streak, game->max_streak);
}

void timing_game_update(timing_game_t* game, uint32_t current_time_ms, uint32_t delta_ms) {
    // 演出タイマーの減算
    if (game->effect_hit_timer_ms > 0) {
        game->effect_hit_timer_ms -= (int)delta_ms;
        if (game->effect_hit_timer_ms < 0) game->effect_hit_timer_ms = 0;
    }
    if (game->effect_miss_timer_ms > 0) {
        game->effect_miss_timer_ms -= (int)delta_ms;
        if (game->effect_miss_timer_ms < 0) game->effect_miss_timer_ms = 0;
    }

    if (game->state != GAME_STATE_PLAYING) {
        return;
    }

    // ドロップの自動出現
    if (current_time_ms - game->last_spawn_time_ms >= game->spawn_interval_ms) {
        game->last_spawn_time_ms = current_time_ms;
        spawn_drop(game);
    }

    // ドロップの移動更新
    float dt_sec = (float)delta_ms / 1000.0f;
    for (int i = 0; i < MAX_DROPS; i++) {
        if (game->drops[i].active) {
            game->drops[i].position += game->drops[i].speed * dt_sec;

            // ターゲット位置を通り過ぎた場合は見逃し (MISS)
            if (game->drops[i].position > (float)game->target_center_led + game->target_tolerance) {
                game->drops[i].active = false;
                game->streak = 0;
                game->life--;
                game->effect_miss_timer_ms = 250;

                fire_event(game, "MISS", 0, 0);
                fire_event(game, "STREAK_UPDATE", game->streak, game->max_streak);
                fire_event(game, "LIFE_UPDATE", game->life, 0);

                if (game->life <= 0) {
                    game->life = 0;
                    game->state = GAME_STATE_GAMEOVER;
                    fire_event(game, "GAME_OVER", game->score, game->max_streak);
                    fire_event(game, "GAME_STATE", GAME_STATE_GAMEOVER, 0);
                    return;
                }
            }
        }
    }
}

void timing_game_render(const timing_game_t* game, rgb_color_t* led_buffer, int led_count) {
    if (!led_buffer || led_count <= 0) return;

    // バッファを黒（消灯）でクリア
    memset(led_buffer, 0, sizeof(rgb_color_t) * led_count);

    if (game->state == GAME_STATE_IDLE) {
        // アイドル待機中: ターゲットエリアを白く呼吸点灯またはゆっくり点滅
        int target = game->target_center_led;
        if (target >= 1 && target < led_count - 1) {
            led_buffer[target - 1] = (rgb_color_t){15, 15, 15};
            led_buffer[target]     = (rgb_color_t){30, 30, 30};
            led_buffer[target + 1] = (rgb_color_t){15, 15, 15};
        }
        return;
    }

    if (game->state == GAME_STATE_GAMEOVER) {
        // ゲームオーバー表示: 全体または下端を赤く点灯
        for (int i = 0; i < led_count; i++) {
            led_buffer[i] = (rgb_color_t){40, 0, 0};
        }
        return;
    }

    // 1. ターゲット位置のガイド描画（通常は薄い白色）
    int target = game->target_center_led;
    if (target >= 1 && target < led_count - 1) {
        if (game->effect_hit_timer_ms > 0) {
            // ヒット演出: 鮮やかな緑色
            led_buffer[target - 1] = (rgb_color_t){0, 180, 0};
            led_buffer[target]     = (rgb_color_t){0, 255, 0};
            led_buffer[target + 1] = (rgb_color_t){0, 180, 0};
        } else if (game->effect_miss_timer_ms > 0) {
            // ミス演出: 赤色フラッシュ
            led_buffer[target - 1] = (rgb_color_t){180, 0, 0};
            led_buffer[target]     = (rgb_color_t){255, 0, 0};
            led_buffer[target + 1] = (rgb_color_t){180, 0, 0};
        } else {
            // 通常ガイド: 薄い白
            led_buffer[target - 1] = (rgb_color_t){12, 12, 12};
            led_buffer[target]     = (rgb_color_t){25, 25, 25};
            led_buffer[target + 1] = (rgb_color_t){12, 12, 12};
        }
    }

    // 2. 落下する光（ドロップ）の描画
    for (int i = 0; i < MAX_DROPS; i++) {
        if (game->drops[i].active) {
            int pos = (int)roundf(game->drops[i].position);
            if (pos >= 0 && pos < led_count) {
                rgb_color_t c = {0, 0, 0};
                switch (game->drops[i].color) {
                    case GAME_COLOR_RED:   c = (rgb_color_t){255, 0, 0}; break;
                    case GAME_COLOR_GREEN: c = (rgb_color_t){0, 255, 0}; break;
                    case GAME_COLOR_BLUE:  c = (rgb_color_t){0, 100, 255}; break;
                    default: break;
                }
                led_buffer[pos] = c;

                // 速度感演出: 1つ後ろのLEDにうっすら残光をつける
                int tail_pos = pos - 1;
                if (tail_pos >= 0 && tail_pos < led_count && tail_pos != target && tail_pos != target-1 && tail_pos != target+1) {
                    led_buffer[tail_pos] = (rgb_color_t){c.r / 4, c.g / 4, c.b / 4};
                }
            }
        }
    }
}
