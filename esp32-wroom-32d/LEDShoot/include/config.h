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
// 未定義または空文字の場合、ビルドエラーになります
// ==========================================
#if !defined(WIFI_PASS) && defined(WIFI_PASSWORD)
#define WIFI_PASS WIFI_PASSWORD
#endif

#if !defined(WIFI_SSID) || !defined(WIFI_PASS)
#error "WIFI_SSID and WIFI_PASS (or WIFI_PASSWORD) must be set at build time! Example: export WIFI_SSID=\"MySSID\" WIFI_PASS=\"MyPass\""
#endif

#if defined(__cplusplus)
static_assert(sizeof(WIFI_SSID) > 1, "WIFI_SSID is empty! Please provide a valid Wi-Fi SSID.");
static_assert(sizeof(WIFI_PASS) > 1, "WIFI_PASS is empty! Please provide a valid Wi-Fi password.");
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(WIFI_SSID) > 1, "WIFI_SSID is empty! Please provide a valid Wi-Fi SSID.");
_Static_assert(sizeof(WIFI_PASS) > 1, "WIFI_PASS is empty! Please provide a valid Wi-Fi password.");
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
