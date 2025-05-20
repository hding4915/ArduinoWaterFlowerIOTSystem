#include "connection_tool.h"

void connectWPA2(const char *ssid, const char *username, const char *password) {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    delay(1000);

    WiFi.mode(WIFI_STA);

    wifi_station_set_wpa2_enterprise_auth(1);  // 啟用
    wifi_station_clear_cert_key();             // 不使用憑證時需呼叫
    wifi_station_set_enterprise_identity((uint8 *)username, strlen(username));
    wifi_station_set_enterprise_username((uint8 *)username, strlen(username));
    wifi_station_set_enterprise_password((uint8 *)password, strlen(password));

    WiFi.begin(ssid);
}
