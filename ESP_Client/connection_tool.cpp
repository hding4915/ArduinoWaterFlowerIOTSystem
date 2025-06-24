#include "connection_tool.h"


WiFiUDP Udp;
const char *secret_key = "my_secret_key";

const char *root_CA = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDe......
-----END CERTIFICATE-----
)EOF";


void connectWPA2(const char *ssid, const char *username, const char *password) {
#if defined(ESP8266)
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    delay(1000);

    WiFi.mode(WIFI_STA);
    wifi_station_set_wpa2_enterprise_auth(1);
    wifi_station_clear_cert_key();
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
    // Get unix timestamp
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

    Serial.println("[ESP] Send confirmation messages: " + full_auth_message);
}


int getHumidityFromServer(const String& url) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[ERROR] WiFi not connected");
        return -1;
    }

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    Serial.println("[HTTP] Connecting to: " + url);
    if (!http.begin(client, url)) {
        Serial.println("[ERROR] Unable to begin HTTPS connection");
        return -1;
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.println("[ERROR] HTTP GET failed, code: " + String(httpCode));
        http.end();
        return -1;
    }

    String payload = http.getString();
    Serial.println("[HTTP] Response: " + payload);
    Serial.println("[HTTP] Payload length: " + String(payload.length()));
    http.end();

    StaticJsonDocument<96> doc;
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        Serial.print("[ERROR] JSON parse error: ");
        Serial.println(err.c_str());
        return -1;
    }

    serializeJsonPretty(doc, Serial);
    Serial.println();

    if (doc.containsKey("humidity")) {
        int humidity = doc["humidity"];
        Serial.println("[DATA] Humidity: " + String(humidity));
        return humidity;
    } else {
        Serial.println("[ERROR] Missing 'humidity' in JSON");
        return -1;
    }
}



String generateHMAC(const String &message, const String &key) {
    byte hmacResult[32];  // SHA256 output size

    // mbedtls
    mbedtls_md_context_t ctx;
    mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
    const size_t keyLength = key.length();

    mbedtls_md_init(&ctx);
    mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
    mbedtls_md_hmac_starts(&ctx, (const unsigned char *)key.c_str(), keyLength);
    mbedtls_md_hmac_update(&ctx, (const unsigned char *)message.c_str(), message.length());
    mbedtls_md_hmac_finish(&ctx, hmacResult);
    mbedtls_md_free(&ctx);

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
        secureClient.setInsecure();
        // secureClient.setCACert(root_CA);
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
        secureClient.setInsecure();
        // secureClient.setCACert(root_CA);
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
