#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*web_cmd_cb_t)(const char* cmd);

void web_server_init(const char* ssid, const char* pass, web_cmd_cb_t cmd_cb);
void web_server_loop(void);
void web_server_broadcast_state(bool power, int pattern, uint8_t brightness, float speed, bool auto_cycle);

#ifdef __cplusplus
}
#endif

#endif // WEB_SERVER_H
