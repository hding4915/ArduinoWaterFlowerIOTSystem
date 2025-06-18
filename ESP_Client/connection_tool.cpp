#include "connection_tool.h"


WiFiUDP Udp;
const char *secret_key = "my_secret_key";

const char* root_CA = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDejCCAmKgAwIBAgIQf+UwvzMTQ77dghYQST2KGzANBgkqhkiG9w0BAQsFADBX
MQswCQYDVQQGEwJCRTEZMBcGA1UEChMQR2xvYmFsU2lnbiBudi1zYTEQMA4GA1UE
CxMHUm9vdCBDQTEbMBkGA1UEAxMSR2xvYmFsU2lnbiBSb290IENBMB4XDTIzMTEx
NTAzNDMyMVoXDTI4MDEyODAwMDA0MlowRzELMAkGA1UEBhMCVVMxIjAgBgNVBAoT
GUdvb2dsZSBUcnVzdCBTZXJ2aWNlcyBMTEMxFDASBgNVBAMTC0dUUyBSb290IFI0
MHYwEAYHKoZIzj0CAQYFK4EEACIDYgAE83Rzp2iLYK5DuDXFgTB7S0md+8Fhzube
Rr1r1WEYNa5A3XP3iZEwWus87oV8okB2O6nGuEfYKueSkWpz6bFyOZ8pn6KY019e
WIZlD6GEZQbR3IvJx3PIjGov5cSr0R2Ko4H/MIH8MA4GA1UdDwEB/wQEAwIBhjAd
BgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwDwYDVR0TAQH/BAUwAwEB/zAd
BgNVHQ4EFgQUgEzW63T/STaj1dj8tT7FavCUHYwwHwYDVR0jBBgwFoAUYHtmGkUN
l8qJUC99BM00qP/8/UswNgYIKwYBBQUHAQEEKjAoMCYGCCsGAQUFBzAChhpodHRw
Oi8vaS5wa2kuZ29vZy9nc3IxLmNydDAtBgNVHR8EJjAkMCKgIKAehhxodHRwOi8v
Yy5wa2kuZ29vZy9yL2dzcjEuY3JsMBMGA1UdIAQMMAowCAYGZ4EMAQIBMA0GCSqG
SIb3DQEBCwUAA4IBAQAYQrsPBtYDh5bjP2OBDwmkoWhIDDkic574y04tfzHpn+cJ
odI2D4SseesQ6bDrarZ7C30ddLibZatoKiws3UL9xnELz4ct92vID24FfVbiI1hY
+SW6FoVHkNeWIP0GCbaM4C6uVdF5dTUsMVs/ZbzNnIdCp5Gxmx5ejvEau8otR/Cs
kGN+hr/W5GvT1tMBjgWKZ1i4//emhA1JG1BbPzoLJQvyEotc03lXjTaCzv8mEbep
8RqZ7a2CPsgRbuvTPBwcOMBBmuFeU88+FSBX6+7iP0il8b4Z0QFqIwwMHfs/L6K1
vepuoxtGzi4CZ68zJpiq1UvSqTbFJjtbD4seiMHl
-----END CERTIFICATE-----
)EOF";


void connectWPA2(const char *ssid, const char *username, const char *password) {
#if defined(ESP8266)
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

#elif defined(ESP32)
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(1000);

  WiFi.mode(WIFI_STA);
  esp_wifi_sta_enterprise_enable();
  esp_eap_client_clear_ca_cert();
  esp_eap_client_set_identity((uint8_t *)username, strlen(username));
  esp_eap_client_set_username((uint8_t *)username, strlen(username));
  esp_eap_client_set_password((uint8_t *)password, strlen(password));
  WiFi.begin(ssid);
#endif
}

void sendAuthentication(IPAddress rpi_ip, uint16_t rpi_port, String device_id) {
  // 取得 unix timestamp
  time_t now = time(nullptr);
  String timestamp = String(now);

  String auth_message = device_id + "|" + timestamp;
  String hmac = generateHMAC(auth_message, secret_key);

  String full_auth_message = "ESP_CONFIRM|" + auth_message + "|" + hmac;

  Udp.beginPacket(rpi_ip, rpi_port);

#if defined(ESP8266)
  Udp.write(full_auth_message.c_str());
#elif defined(ESP32)
  Udp.write((const uint8_t *)full_auth_message.c_str(), full_auth_message.length());
#endif

  Udp.endPacket();

  Serial.println("[ESP] 傳送認證訊息: " + full_auth_message);
}


String generateHMAC(const String &message, const String &key) {
  byte hmacResult[32];  // SHA256 output size

  // 使用 mbedtls 實作
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
  const size_t keyLength = key.length();

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
  mbedtls_md_hmac_starts(&ctx, (const unsigned char *)key.c_str(), keyLength);
  mbedtls_md_hmac_update(&ctx, (const unsigned char *)message.c_str(), message.length());
  mbedtls_md_hmac_finish(&ctx, hmacResult);
  mbedtls_md_free(&ctx);

  // 轉成十六進位字串
  String hexResult = "";
  for (int i = 0; i < 32; i++) {
    if (hmacResult[i] < 16) hexResult += "0";
    hexResult += String(hmacResult[i], HEX);
  }
  return hexResult;
}


void showHttpData(String url) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected");
    return;
  }

  HTTPClient http;

  if (url.startsWith("https://")) {
    WiFiClientSecure secureClient;
    // secureClient.setInsecure();  // 測試用，正式請使用憑證
    secureClient.setCACert(root_CA);
    if (http.begin(secureClient, url)) {
      int httpCode = http.GET();
      handleHttpGetResponse(http, httpCode);
    } else {
      Serial.println("Unable to connect to HTTPS server.");
    }
  } else {
    WiFiClient plainClient;
    if (http.begin(plainClient, url)) {
      int httpCode = http.GET();
      handleHttpGetResponse(http, httpCode);
    } else {
      Serial.println("Unable to connect to HTTP server.");
    }
  }
}


void handleHttpGetResponse(HTTPClient &http, int httpCode) {
  if (httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Received data: " + payload);
    } else {
      Serial.println("GET request failed or not 200 OK.");
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}



void sendHttpData(String payload, String targetUrl) {
  HTTPClient http;
  Serial.println("Payload: " + payload);

  if (targetUrl.startsWith("https://")) {
    WiFiClientSecure secureClient;
    // secureClient.setInsecure();  // 測試用途
    secureClient.setCACert(root_CA);
    if (http.begin(secureClient, targetUrl)) {
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.POST(payload);
      handleHttpPostResponse(http, httpCode);
    } else {
      Serial.println("Unable to connect to HTTPS server.");
    }
  } else {
    WiFiClient plainClient;
    if (http.begin(plainClient, targetUrl)) {
      http.addHeader("Content-Type", "application/json");
      int httpCode = http.POST(payload);
      handleHttpPostResponse(http, httpCode);
    } else {
      Serial.println("Unable to connect to HTTP server.");
    }
  }
}

void handleHttpPostResponse(HTTPClient &http, int httpCode) {
  if (httpCode > 0) {
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
      String response = http.getString();
      Serial.println("Response: " + response);
    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}


void sendHumiHttpData(String deviceID, String targetUrl, int humi) {
  String payload = "{\"device\": \"" + deviceID + "\", \"humidity\": " + String(humi) + "}";
  sendHttpData(payload, targetUrl);
}
