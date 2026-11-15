#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "illumination.h"

int main(void) {
    printf("Starting Christmas Illumination unit tests...\n");

    illumination_t illumi;
    illumination_init(&illumi, 60);

    assert(illumi.power_on == true);
    assert(illumi.pattern == PATTERN_CANDY_CANE);

    rgb_color_t buffer[60];

    // 1. レンダリングテスト (Candy Cane)
    illumination_render(&illumi, buffer, 60);
    // 最初のピクセルは赤のはず
    assert(buffer[0].r > 0);

    // 2. パターン変更テスト
    for (int p = 0; p < PATTERN_COUNT; p++) {
        illumination_set_pattern(&illumi, (illumi_pattern_t)p);
        assert(illumi.pattern == (illumi_pattern_t)p);

        // 時間経過更新
        illumination_update(&illumi, (uint32_t)(p * 1000 + 100));
        illumination_render(&illumi, buffer, 60);

        printf("Pattern %d (%s) render OK!\n", p, illumination_get_pattern_name((illumi_pattern_t)p));
    }

    // 3. 電源OFFテスト
    illumination_set_power(&illumi, false);
    illumination_render(&illumi, buffer, 60);
    for (int i = 0; i < 60; i++) {
        assert(buffer[i].r == 0 && buffer[i].g == 0 && buffer[i].b == 0);
    }
    printf("Power OFF (blackout) test passed!\n");

    // 4. Auto Cycle テスト
    illumination_set_power(&illumi, true);
    illumination_set_auto_cycle(&illumi, true, 10); // 10秒周期
    illumi.pattern = PATTERN_CANDY_CANE;
    illumi.last_cycle_time_ms = 1000;

    // 11秒後 (12000ms) に更新
    illumination_update(&illumi, 12000);
    assert(illumi.pattern == PATTERN_SNOWFALL);
    printf("Auto Cycle switch test passed!\n");

    printf("All illumination unit tests passed successfully!\n");
    return 0;
}
