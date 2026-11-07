#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "timing_game.h"

static int g_hit_count = 0;
static int g_miss_count = 0;
static int g_last_score = 0;
static int g_game_over_score = -1;

static void test_event_cb(const char* event_type, int v1, int v2) {
    printf("[EVENT] %s: val1=%d, val2=%d\n", event_type, v1, v2);
    if (strcmp(event_type, "HIT") == 0) {
        g_hit_count++;
    } else if (strcmp(event_type, "MISS") == 0) {
        g_miss_count++;
    } else if (strcmp(event_type, "SCORE_UPDATE") == 0) {
        g_last_score = v1;
    } else if (strcmp(event_type, "GAME_OVER") == 0) {
        g_game_over_score = v1;
    }
}

int main(void) {
    printf("Starting Timing Game logic tests...\n");

    timing_game_t game;
    timing_game_init(&game, test_event_cb);
    assert(game.state == GAME_STATE_IDLE);

    // 1. ゲームスタート
    timing_game_start(&game);
    assert(game.state == GAME_STATE_PLAYING);
    assert(game.life == 3);
    assert(game.score == 0);

    // 2. ドロップ生成と移動シミュレーション (出現間隔2200ms以上)
    timing_game_update(&game, 2500, 2500);

    int test_drop_idx = -1;
    for (int i = 0; i < MAX_DROPS; i++) {
        if (game.drops[i].active) {
            test_drop_idx = i;
            game.drops[i].position = (float)game.target_center_led;
            game.drops[i].color = GAME_COLOR_RED;
            break;
        }
    }
    assert(test_drop_idx >= 0);

    // 3. 違う色のボタン入力 -> STREAKはリセットされるが、LIFEは減らない！光も残る！
    timing_game_handle_input(&game, GAME_COLOR_BLUE);
    assert(g_miss_count == 1);
    assert(game.life == 3); // LIFEは減らない
    assert(game.streak == 0);
    assert(game.drops[test_drop_idx].active == true); // 光も消えない

    // 4. 正しい色のボタン入力 -> HIT
    timing_game_handle_input(&game, GAME_COLOR_RED);
    assert(g_hit_count == 1);
    assert(game.score > 0);
    assert(game.streak == 1);
    assert(game.drops[test_drop_idx].active == false); // 消える

    // 5. 光が消えずに最後まで到達した場合 -> LIFEが減る
    // 新しいドロップを生成し、ターゲット通過させる
    timing_game_update(&game, 5000, 2500);
    for (int i = 0; i < MAX_DROPS; i++) {
        if (game.drops[i].active) {
            test_drop_idx = i;
            // ターゲットより先に強制移動
            game.drops[i].position = (float)game.target_center_led + game.target_tolerance + 1.0f;
            break;
        }
    }
    // updateを呼んで見逃し判定を処理
    timing_game_update(&game, 5010, 10);
    assert(game.life == 2); // ここで初めてLIFEが減る！

    // 残り2回通過させてゲームオーバー確認
    for (int k = 0; k < 2; k++) {
        timing_game_update(&game, 8000 + k * 3000, 2500);
        for (int i = 0; i < MAX_DROPS; i++) {
            if (game.drops[i].active) {
                game.drops[i].position = (float)game.target_center_led + game.target_tolerance + 1.0f;
                break;
            }
        }
        timing_game_update(&game, 8010 + k * 3000, 10);
    }
    assert(game.life == 0);
    assert(game.state == GAME_STATE_GAMEOVER);
    assert(g_game_over_score >= 0);

    // 6. LED描画テスト
    rgb_color_t buffer[60];
    timing_game_render(&game, buffer, 60);
    assert(buffer[0].r == 40 && buffer[0].g == 0 && buffer[0].b == 0);

    printf("All timing_game unit tests passed successfully!\n");
    return 0;
}
