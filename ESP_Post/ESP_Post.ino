#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "connection_tool.h"

const char* ssid = "eduroam";          // 校內 WiFi 名稱
const char* username = "f74121084@eduroam.ncku.edu.tw";
const char* password = "Artw5619";  // 校內 WiFi 密碼（如有）
const char* serverIP = "10.5.3.30";
const int serverPort = 8000;
String url;

void setup() {
    Serial.begin(115200);
    
    connectWPA2(ssid, username, password);

    Serial.println("Connecting to " + String(ssid));
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting...");
    }
    Serial.println("Connected!, IP: " + WiFi.localIP().toString());

    url = "http://" + String(serverIP) + ":" + String(serverPort) + "/api/data";
}


void sendHttpData(int humi) {
    WiFiClient newClient;
    HTTPClient http;

    String payload = "{\"device\": \"esp8266_1\", \"humidity\": " + String(humi) + "}";
    String target = "http://" + String(serverIP) + ":" + String(serverPort) + "/api/data";

    yield();

    if (http.begin(newClient, target)) {
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.POST(payload);
        yield();

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
    } else {
        Serial.println("Unable to connect to server.");
    }

    yield();
}



char recv;

void loop() {
    // 可重複取得資料
    int sensorVal = analogRead(A0);
    Serial.println("humidity: " + String(sensorVal));
    if (Serial.available() > 0) {
        recv = Serial.read();
        if (recv == '5') {
            sendHttpData(sensorVal);
        }
    }
    yield();
    delay(500);
}
