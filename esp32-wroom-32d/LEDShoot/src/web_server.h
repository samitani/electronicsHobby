#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// スマホから受信したコマンドのコールバック型
typedef void (*web_cmd_cb_t)(const char* cmd);

void web_server_init(const char* ssid, const char* pass, web_cmd_cb_t cmd_cb);
void web_server_loop(void);
void web_server_broadcast_event(const char* event_type, int val1, int val2);

#ifdef __cplusplus
}
#endif

#endif // WEB_SERVER_H
