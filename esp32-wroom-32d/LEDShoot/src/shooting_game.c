#include "shooting_game.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define INITIAL_SPAWN_INTERVAL_MS 2400
#define MIN_SPAWN_INTERVAL_MS     100
#define INTERVAL_DECREASE_STEP    48

#define INITIAL_TARGET_SPEED      6.5f
#define MAX_TARGET_SPEED          33.0f
#define SPEED_INCREASE_STEP       0.58f

#define DEFAULT_BULLET_SPEED      42.0f

static void fire_event(shooting_game_t* game, const char* event_type, int v1, int v2) {
    if (game->event_cb) {
        game->event_cb(event_type, v1, v2);
    }
}

static void clear_entities(shooting_game_t* game) {
    for (int i = 0; i < MAX_SHOOTING_TARGETS; i++) {
        game->targets[i].active = false;
        game->targets[i].position = 0.0f;
        game->targets[i].speed = 0.0f;
        game->targets[i].color = GAME_COLOR_NONE;
    }
    for (int i = 0; i < MAX_SHOOTING_BULLETS; i++) {
        game->bullets[i].active = false;
        game->bullets[i].position = 0.0f;
        game->bullets[i].speed = 0.0f;
        game->bullets[i].color = GAME_COLOR_NONE;
    }
}

static void spawn_target(shooting_game_t* game) {
    for (int i = 0; i < MAX_SHOOTING_TARGETS; i++) {
        if (!game->targets[i].active) {
            game->targets[i].active = true;
            game->targets[i].position = 0.0f;
            game->targets[i].speed = game->target_speed;

            // ランダムに赤・緑・青を選択
            int r = rand() % 3;
            if (r == 0) game->targets[i].color = GAME_COLOR_RED;
            else if (r == 1) game->targets[i].color = GAME_COLOR_GREEN;
            else game->targets[i].color = GAME_COLOR_BLUE;

            game->targets_spawned++;

            // 難易度調整: 落下速度上昇 & 出現間隔短縮
            if (game->spawn_interval_ms > MIN_SPAWN_INTERVAL_MS) {
                game->spawn_interval_ms -= INTERVAL_DECREASE_STEP;
                if (game->spawn_interval_ms < MIN_SPAWN_INTERVAL_MS) {
                    game->spawn_interval_ms = MIN_SPAWN_INTERVAL_MS;
                }
            }
            if (game->target_speed < MAX_TARGET_SPEED) {
                game->target_speed += SPEED_INCREASE_STEP;
                if (game->target_speed > MAX_TARGET_SPEED) {
                    game->target_speed = MAX_TARGET_SPEED;
                }
            }
            break;
        }
    }
}

void shooting_game_init(shooting_game_t* game, int led_count, game_event_cb_t cb) {
    memset(game, 0, sizeof(shooting_game_t));
    game->event_cb = cb;
    game->led_count = (led_count > 0) ? led_count : 60;
    game->state = GAME_STATE_IDLE;
    clear_entities(game);
}

void shooting_game_start(shooting_game_t* game) {
    clear_entities(game);
    game->state = GAME_STATE_PLAYING;
    game->score = 0;
    game->streak = 0;
    game->max_streak = 0;
    game->spawn_interval_ms = INITIAL_SPAWN_INTERVAL_MS;
    game->target_speed = INITIAL_TARGET_SPEED;
    game->bullet_speed = DEFAULT_BULLET_SPEED;
    game->targets_spawned = 0;
    game->last_spawn_time_ms = 0;
    game->effect_hit_timer_ms = 0;
    game->effect_miss_timer_ms = 0;

    fire_event(game, "GAME_STATE", GAME_STATE_PLAYING, 0);
    fire_event(game, "SCORE_UPDATE", game->score, 0);
    fire_event(game, "STREAK_UPDATE", game->streak, game->max_streak);
}

void shooting_game_restart(shooting_game_t* game) {
    shooting_game_start(game);
}

void shooting_game_handle_input(shooting_game_t* game, game_color_t color) {
    if (game->state != GAME_STATE_PLAYING) {
        return;
    }

    // 下端から上向きに弾を発射
    for (int i = 0; i < MAX_SHOOTING_BULLETS; i++) {
        if (!game->bullets[i].active) {
            game->bullets[i].active = true;
            game->bullets[i].position = (float)(game->led_count - 1);
            game->bullets[i].speed = game->bullet_speed;
            game->bullets[i].color = color;
            break;
        }
    }
}

void shooting_game_update(shooting_game_t* game, uint32_t current_time_ms, uint32_t delta_ms) {
    // 演出タイマー減算
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

    // ターゲットの自動出現
    if (current_time_ms - game->last_spawn_time_ms >= game->spawn_interval_ms) {
        game->last_spawn_time_ms = current_time_ms;
        spawn_target(game);
    }

    float dt_sec = (float)delta_ms / 1000.0f;

    // 1. 弾の位置更新（下から上へ移動）
    for (int b = 0; b < MAX_SHOOTING_BULLETS; b++) {
        if (game->bullets[b].active) {
            game->bullets[b].position -= game->bullets[b].speed * dt_sec;
            if (game->bullets[b].position < 0.0f) {
                game->bullets[b].active = false;
            }
        }
    }

    // 2. ターゲットの位置更新（上から下へ移動）
    for (int t = 0; t < MAX_SHOOTING_TARGETS; t++) {
        if (game->targets[t].active) {
            game->targets[t].position += game->targets[t].speed * dt_sec;

            // 落下ターゲットが下端に到達した場合は即ゲームオーバー！
            if (game->targets[t].position >= (float)(game->led_count - 1)) {
                game->targets[t].position = (float)(game->led_count - 1);
                game->state = GAME_STATE_GAMEOVER;
                fire_event(game, "GAME_OVER", game->score, game->max_streak);
                fire_event(game, "GAME_STATE", GAME_STATE_GAMEOVER, 0);
                return;
            }
        }
    }

    // 3. 衝突判定 (弾 vs 落下ターゲット)
    // 複数同士のすれ違い判定を行う
    for (int b = 0; b < MAX_SHOOTING_BULLETS; b++) {
        if (!game->bullets[b].active) continue;

        for (int t = 0; t < MAX_SHOOTING_TARGETS; t++) {
            if (!game->targets[t].active) continue;

            // 弾がターゲットの位置まで到達（すれ違い・接触）したか判定
            if (game->bullets[b].position <= game->targets[t].position + 0.8f) {
                if (game->bullets[b].color == game->targets[t].color) {
                    // 同色: 撃破！両方消滅
                    game->targets[t].active = false;
                    game->bullets[b].active = false;

                    game->streak++;
                    if (game->streak > game->max_streak) {
                        game->max_streak = game->streak;
                    }
                    int points = 100 + (game->streak * 10);
                    game->score += points;

                    game->effect_hit_timer_ms = 180;
                    game->effect_hit_pos = (int)roundf(game->targets[t].position);

                    fire_event(game, "HIT", points, 0);
                    fire_event(game, "SCORE_UPDATE", game->score, 0);
                    fire_event(game, "STREAK_UPDATE", game->streak, game->max_streak);
                } else {
                    // 異色: 弾のみ消滅、落下ターゲットはそのまま継続！
                    game->bullets[b].active = false;
                    game->streak = 0;
                    game->effect_miss_timer_ms = 150;

                    fire_event(game, "MISS", 0, 0);
                    fire_event(game, "STREAK_UPDATE", game->streak, game->max_streak);
                }
                break; // この弾は消えたので次の弾へ
            }
        }
    }
}

void shooting_game_render(const shooting_game_t* game, rgb_color_t* led_buffer, int led_count) {
    if (!led_buffer || led_count <= 0) return;

    memset(led_buffer, 0, sizeof(rgb_color_t) * led_count);

    if (game->state == GAME_STATE_IDLE) {
        // アイドル待機中: 下端の砲台をうっすら水色で点灯
        if (led_count > 0) {
            led_buffer[led_count - 1] = (rgb_color_t){0, 30, 40};
        }
        return;
    }

    if (game->state == GAME_STATE_GAMEOVER) {
        // ゲームオーバー時: 全体赤色表示
        for (int i = 0; i < led_count; i++) {
            led_buffer[i] = (rgb_color_t){45, 0, 0};
        }
        return;
    }

    // 1. 下端の自機（砲台）を表示
    led_buffer[led_count - 1] = (rgb_color_t){25, 25, 25};

    // 2. 落下ターゲットの描画
    for (int t = 0; t < MAX_SHOOTING_TARGETS; t++) {
        if (game->targets[t].active) {
            int pos = (int)roundf(game->targets[t].position);
            if (pos >= 0 && pos < led_count) {
                rgb_color_t c = {0, 0, 0};
                switch (game->targets[t].color) {
                    case GAME_COLOR_RED:   c = (rgb_color_t){255, 0, 0}; break;
                    case GAME_COLOR_GREEN: c = (rgb_color_t){0, 255, 0}; break;
                    case GAME_COLOR_BLUE:  c = (rgb_color_t){0, 100, 255}; break;
                    default: break;
                }
                led_buffer[pos] = c;

                // 落下トレイル（後ろに薄い残光）
                int tail = pos - 1;
                if (tail >= 0 && tail < led_count) {
                    led_buffer[tail] = (rgb_color_t){c.r / 4, c.g / 4, c.b / 4};
                }
            }
        }
    }

    // 3. 上昇する弾の描画
    for (int b = 0; b < MAX_SHOOTING_BULLETS; b++) {
        if (game->bullets[b].active) {
            int pos = (int)roundf(game->bullets[b].position);
            if (pos >= 0 && pos < led_count) {
                rgb_color_t c = {0, 0, 0};
                switch (game->bullets[b].color) {
                    case GAME_COLOR_RED:   c = (rgb_color_t){255, 50, 50}; break;
                    case GAME_COLOR_GREEN: c = (rgb_color_t){50, 255, 50}; break;
                    case GAME_COLOR_BLUE:  c = (rgb_color_t){80, 180, 255}; break;
                    default: break;
                }
                // すでにターゲット等が重なっている場合は加算または上書き
                led_buffer[pos] = c;

                // 上昇トレイル（1つ下に薄い残光）
                int tail = pos + 1;
                if (tail < led_count) {
                    led_buffer[tail] = (rgb_color_t){c.r / 3, c.g / 3, c.b / 3};
                }
            }
        }
    }

    // 4. ヒット演出（撃破エフェクト）
    if (game->effect_hit_timer_ms > 0) {
        int pos = game->effect_hit_pos;
        if (pos >= 0 && pos < led_count) {
            led_buffer[pos] = (rgb_color_t){255, 255, 255}; // 白フラッシュ
            if (pos > 0) led_buffer[pos - 1] = (rgb_color_t){120, 255, 120};
            if (pos < led_count - 1) led_buffer[pos + 1] = (rgb_color_t){120, 255, 120};
        }
    }
}
