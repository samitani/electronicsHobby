#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ==========================================
// ハードウェア設定
// ==========================================
#ifndef LED_GPIO
#define LED_GPIO 25
#endif

#ifndef LED_COUNT
#define LED_COUNT 60
#endif

// LEDの方向設定:
// true: DIN側が上端（光が出現する側、インデックス 0）
// false: 反転（DIN側が下端、インデックス 59）
#define LED_DIN_IS_TOP false

// ==========================================
// Wi-Fi設定 (STAモードのみ)
// build_flags (-D WIFI_SSID="..." -D WIFI_PASS="...") で上書き可能
// ==========================================
#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID"
#endif

#ifndef WIFI_PASS
#define WIFI_PASS "YOUR_WIFI_PASSWORD"
#endif

// ==========================================
// サーバー設定
// ==========================================
#define HTTP_PORT 80
#define WS_PORT   81

#ifdef __cplusplus
}
#endif

#endif // CONFIG_H
