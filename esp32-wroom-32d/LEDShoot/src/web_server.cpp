#include "web_server.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "web_page.h"

static WebServer server(80);
static WebSocketsServer wsServer(81);
static web_cmd_cb_t s_cmd_cb = nullptr;

static void handleRoot() {
    server.send_P(200, "text/html", INDEX_HTML);
}

static void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WS] Client #%u Disconnected\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = wsServer.remoteIP(num);
            Serial.printf("[WS] Client #%u Connected from %s\n", num, ip.toString().c_str());
            break;
        }
        case WStype_TEXT: {
            // 受信文字列をnull終端
            char msg[64];
            size_t copy_len = (length < sizeof(msg) - 1) ? length : sizeof(msg) - 1;
            memcpy(msg, payload, copy_len);
            msg[copy_len] = '\0';

            Serial.printf("[WS] Received: %s\n", msg);
            if (s_cmd_cb != nullptr) {
                s_cmd_cb(msg);
            }
            break;
        }
        default:
            break;
    }
}

extern "C" {

void web_server_init(const char* ssid, const char* pass, web_cmd_cb_t cmd_cb) {
    s_cmd_cb = cmd_cb;

    Serial.println("\n[WiFi] Connecting to WiFi in STA mode...");
    Serial.printf("[WiFi] Target SSID: %s\n", ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);

    int timeout_count = 0;
    while (WiFi.status() != WL_CONNECTED && timeout_count < 30) {
        delay(500);
        Serial.print(".");
        timeout_count++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] Connected successfully!");
        Serial.print("[WiFi] IP Address: http://");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n[WiFi] Connection failed or timed out. Will keep trying in background.");
    }

    // HTTPサーバー設定
    server.on("/", handleRoot);
    server.begin();
    Serial.println("[HTTP] Server started on port 80");

    // WebSocketサーバー設定
    wsServer.begin();
    wsServer.onEvent(onWebSocketEvent);
    Serial.println("[WS] Server started on port 81");
}

void web_server_loop(void) {
    server.handleClient();
    wsServer.loop();
}

void web_server_broadcast_event(const char* event_type, int val1, int val2) {
    char json_buf[128];
    snprintf(json_buf, sizeof(json_buf), "{\"type\":\"%s\",\"val\":%d,\"v2\":%d}", event_type, val1, val2);
    wsServer.broadcastTXT(json_buf);
}

} // extern "C"
