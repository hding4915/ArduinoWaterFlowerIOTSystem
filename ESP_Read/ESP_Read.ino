#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "Roger";
const char* password = "artw5619";
const char* serverIP = "192.168.129.23";  // 桌機 IP http://192.168.129.23:8000
const int serverPort = 8000;
String url;


void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nConnected!");
}


void showHttpData() {
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        WiFiClient client;

        url = "http://" + String(serverIP) + ":" + String(serverPort) + "/api/last_data";
        http.begin(client, url);  // 新寫法

        int httpCode = http.GET();
        if (httpCode > 0) {
            String payload = http.getString();
            Serial.println("Received data: " + payload);
        } else {
            Serial.println("Error on HTTP request");
        }

        http.end();
    }
}


char recv;

void loop() {
    // 可重複取得資料
    int sensorVal = analogRead(A0);
    Serial.println("humidity: " + String(sensorVal));
    if (Serial.available() > 0) {
        recv = Serial.read();
        if (recv == '5') {
            showHttpData();
        }
    }
    delay(500);
}
